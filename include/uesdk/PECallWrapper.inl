#pragma once
#include <uesdk/PECallWrapper.hpp>

#include <iostream>
#include <mutex>

namespace SDK
{
    template <typename T>
    struct is_writable_pointer
    {
        static constexpr bool value = std::is_pointer_v<T> && !std::is_const_v<std::remove_pointer_t<T>> && !std::is_volatile_v<std::remove_pointer_t<T>>;
    };

    template <typename...>
    inline constexpr bool dependent_false = false;

    template <StringLiteral ClassName, StringLiteral FunctionName, typename ReturnType, typename... Args>
    ReturnType PECallWrapperImpl<ClassName, FunctionName, ReturnType, Args...>::Call(UObject* Obj, UFunction* Function, Args... args)
    {
        constexpr size_t NumArgs = sizeof...(Args);
        static FunctionArgInfo<NumArgs> FunctionArgs = {};

        // TODO: Add check to ensure that UFunction reflects this function signature
        if constexpr (std::is_void_v<ReturnType> && NumArgs == 0) {
            Obj->ProcessEvent(Function, nullptr);
            return;
        }

        // TODO: Add support for non-trivially copyable types like TArray in functions like GetAllActorsOfClass
        constexpr bool IsVoidRetType = std::is_void_v<ReturnType>;
        static_assert((std::is_trivially_copyable_v<std::decay_t<Args>> && ...), "All argument types must be trivially copyable");
        if constexpr (!IsVoidRetType) {
            static_assert(std::is_trivially_copyable_v<std::decay_t<ReturnType>>,
                "Return type must be trivially copyable if not void");
        }

        static std::once_flag SetupOnce;
        std::call_once(SetupOnce, [&] {
            InitializeArgInfo(Function, FunctionArgs);
        });

        bool UsedHeap = false;
        uint8_t* Parms = AllocateParams(FunctionArgs.ParmsSize, UsedHeap);

        WriteInputArgs(Parms, FunctionArgs, std::forward<Args>(args)...);
        Obj->ProcessEvent(Function, Parms);
        WriteOutputArgs(Parms, FunctionArgs, std::forward<Args>(args)...);

        if constexpr (!IsVoidRetType) {
            if (!FunctionArgs.HasReturnValue)
                throw std::logic_error("Mismatched return type: '" + Function->GetFullName() + "' expects no return value, but template specifies non-void return type");

            ReturnType Return;
            std::memcpy(&Return, Parms + FunctionArgs.ReturnValueOffset, std::min<int32_t>(FunctionArgs.ReturnValueSize, sizeof(ReturnType)));
            CleanupParams(Parms, UsedHeap);
            return Return;
        }

        CleanupParams(Parms, UsedHeap);
    }

    template <StringLiteral ClassName, StringLiteral FunctionName, typename ReturnType, typename... Args>
    ReturnType PECallWrapperImpl<ClassName, FunctionName, ReturnType, Args...>::CallAuto(UObject* Obj, Args... args)
    {
        static UFunction* Function = nullptr;

        // Thread safe
        static std::once_flag FindOnce;
        std::call_once(FindOnce, [] {
            if (!FastSearchSingle(FSUFunction(ClassName.c_str(), FunctionName.c_str(), &Function))) {
                throw std::invalid_argument("Failed to automatically find UFunction");
            }
        });

        return Call(Obj, Function, std::forward<Args>(args)...);
    }

    template <StringLiteral ClassName, StringLiteral FunctionName, typename ReturnType, typename... Args>
    uint8_t* PECallWrapperImpl<ClassName, FunctionName, ReturnType, Args...>::AllocateParams(size_t Size, bool& UsedHeap)
    {
        constexpr size_t kAlignment = alignof(std::max_align_t);
        uint8_t* Parms = nullptr;
        size_t BufferSize = Size + kAlignment;

        if (Size <= kMaxStackAllocSize) {
            UsedHeap = false;
            Parms = static_cast<uint8_t*>(_malloca(BufferSize));
        }
        else {
            UsedHeap = true;
            Parms = new (std::nothrow) uint8_t[BufferSize];
        }

        if (!Parms)
            throw std::bad_alloc();

        void* AlignedPtr = Parms;
        size_t Space = BufferSize;
        if (!std::align(kAlignment, Size, AlignedPtr, Space)) {
            CleanupParams(Parms, UsedHeap);
            throw std::bad_alloc();
        }

        Parms = static_cast<uint8_t*>(AlignedPtr);
        std::memset(Parms, 0, Size);
        return Parms;
    }

    template <StringLiteral ClassName, StringLiteral FunctionName, typename ReturnType, typename... Args>
    void PECallWrapperImpl<ClassName, FunctionName, ReturnType, Args...>::CleanupParams(uint8_t* Parms, bool UsedHeap)
    {
        if (UsedHeap)
            delete[] Parms;
        else
            _freea(Parms);
    }

    template <StringLiteral ClassName, StringLiteral FunctionName, typename ReturnType, typename... Args>
    template <size_t N>
    void PECallWrapperImpl<ClassName, FunctionName, ReturnType, Args...>::WriteInputArgs(uint8_t* Parms, FunctionArgInfo<N>& FunctionArgs, Args&&... args)
    {
        auto WriteInputArg = [Parms](auto& Arg, ArgInfo& Info) {
            using ArgType = std::decay_t<decltype(Arg)>;
            std::memcpy(Parms + Info.Offset, &Arg, std::min<int32_t>(Info.Size, sizeof(ArgType)));
        };

        size_t ArgIndex = 0;
        ((FunctionArgs.ArgOffsets[ArgIndex].IsOutParm ? void() : WriteInputArg(args, FunctionArgs.ArgOffsets[ArgIndex]), ++ArgIndex), ...);
    }

    template <StringLiteral ClassName, StringLiteral FunctionName, typename ReturnType, typename... Args>
    template <size_t N>
    void PECallWrapperImpl<ClassName, FunctionName, ReturnType, Args...>::WriteOutputArgs(uint8_t* Parms, FunctionArgInfo<N>& FunctionArgs, Args&&... args)
    {
        auto WriteOutputArg = [Parms](auto& Arg, ArgInfo& Info) {
            using ArgType = std::decay_t<decltype(Arg)>;

            // lvalue reference check takes priority to correctly parse types like void*&
            if constexpr (std::is_lvalue_reference_v<decltype(Arg)>) {
                // TODO: Somehow improve this check. Compiler won't know if an argument is output or not
                //static_assert(!std::is_const_v<std::remove_reference_t<decltype(Arg)>>, "Output reference must not be const");
                std::memcpy(std::addressof(Arg), Parms + Info.Offset, Info.Size);
            }
            else if constexpr (std::is_pointer_v<ArgType>) {
                constexpr bool IsVoidPtr = std::is_void_v<std::remove_pointer_t<ArgType>>;
                constexpr bool IsWritable = is_writable_pointer<ArgType>::value;
                static_assert(IsWritable && !IsVoidPtr, "Output argument pointer must be writable and not void*");

                // TODO: Somehow improve this check. Compiler won't know if an argument is output or not
                //static_assert(!std::is_const_v<std::remove_pointer_t<ArgType>>, "Output argument pointer must not be pointer to const");

                // TODO: Ensure this won't cause a debugging nightmare with how gracefully we handle nullptr output params
                if (Arg)
                    std::memcpy(Arg, Parms + Info.Offset, sizeof(*Arg));
            }
            else {
                static_assert(dependent_false<ArgType>, "Output argument must be a pointer or non-const lvalue reference");
            }
        };

        size_t ArgIndex = 0;
        ((FunctionArgs.ArgOffsets[ArgIndex].IsOutParm ? WriteOutputArg(args, FunctionArgs.ArgOffsets[ArgIndex]) : void(), ++ArgIndex), ...);
    }

    template <StringLiteral ClassName, StringLiteral FunctionName, typename ReturnType, typename... Args>
    template <size_t N>
    void PECallWrapperImpl<ClassName, FunctionName, ReturnType, Args...>::InitializeArgInfo(const UFunction* Function, FunctionArgInfo<N>& FunctionArgs)
    {
        static_assert(sizeof...(Args) == N, "Number of template Args must match the function parameter count");

        FunctionArgs.ParmsSize = Function->ParmsSize;
        FunctionArgs.ReturnValueOffset = Function->ReturnValueOffset;
        FunctionArgs.HasReturnValue = FunctionArgs.ReturnValueOffset != UINT16_MAX;

        int ArgIndex = 0;
        if (State::UsesFProperty) {
            for (FField* Field = Function->ChildProperties; Field; Field = Field->Next) {
                if (!Field->HasTypeFlag(CASTCLASS_FProperty))
                    continue;

                FProperty* Property = static_cast<FProperty*>(Field);
                if (Property->HasPropertyFlag(CPF_ReturnParm)) {
                    if (Property->Offset == FunctionArgs.ReturnValueOffset)
                        FunctionArgs.ReturnValueSize = Property->ElementSize;
                    continue;
                }

                FunctionArgs.ArgOffsets[ArgIndex] = { Property->Offset, Property->ElementSize, Property->HasPropertyFlag(CPF_OutParm) };
                ArgIndex++;
            }
        }
        else {
            for (UField* Child = Function->Children; Child; Child = Child->Next) {
                if (!Child->HasTypeFlag(CASTCLASS_FProperty))
                    continue;

                UProperty* Property = static_cast<UProperty*>(Child);
                if (Property->HasPropertyFlag(CPF_ReturnParm)) {
                    if (Property->Offset == FunctionArgs.ReturnValueOffset)
                        FunctionArgs.ReturnValueSize = Property->ElementSize;
                    continue;
                }

                FunctionArgs.ArgOffsets[ArgIndex] = { Property->Offset, Property->ElementSize, Property->HasPropertyFlag(CPF_OutParm) };
                ArgIndex++;
            }
        }
    }
}

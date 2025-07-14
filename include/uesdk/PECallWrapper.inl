#pragma once
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

        if constexpr (std::is_void_v<ReturnType> && NumArgs == 0) {
            Obj->ProcessEventAsNative(Function, nullptr);
            return;
        }

        static std::once_flag SetupOnce;
        std::call_once(SetupOnce, [&] {
            InitializeArgInfo(Function, FunctionArgs);
        });

        bool UsedHeap = false;
        uint8_t* Parms = AllocateParams(FunctionArgs.ParmsSize, UsedHeap);

        WriteInputArgs(Parms, FunctionArgs, std::forward<Args>(args)...);
        Obj->ProcessEventAsNative(Function, Parms);
        WriteOutputArgs(Parms, FunctionArgs, std::forward<Args>(args)...);

        if constexpr (!std::is_void_v<ReturnType>) {
            if (!FunctionArgs.HasReturnValue)
                throw std::logic_error("Missing return value in UFunction!");

            ReturnType Return;
            static_assert(std::is_trivially_copyable_v<ReturnType>, "Return type must be trivially copyable");

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
                throw std::invalid_argument("Failed to automatically find UFunction!");
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

            if constexpr (std::is_pointer_v<ArgType>) {
                constexpr bool IsVoidPtr = std::is_void_v<std::remove_pointer_t<ArgType>>;
                constexpr bool IsWritable = is_writable_pointer<ArgType>::value;

                if constexpr (!IsWritable || IsVoidPtr)
                    throw std::invalid_argument("Output argument must be writable!");

                if (Arg)
                    std::memcpy(Arg, Parms + Info.Offset, sizeof(*Arg));
            }
            else if constexpr (std::is_lvalue_reference_v<decltype(Arg)>) {
                std::memcpy(std::addressof(Arg), Parms + Info.Offset, Info.Size);
            }
            else {
                static_assert(false, "Output argument must be a pointer or non-const lvalue reference");
            }
        };

        size_t ArgIndex = 0;
        ((FunctionArgs.ArgOffsets[ArgIndex].IsOutParm ? WriteOutputArg(args, FunctionArgs.ArgOffsets[ArgIndex]) : void(), ++ArgIndex), ...);
    }

    template <StringLiteral ClassName, StringLiteral FunctionName, typename ReturnType, typename... Args>
    template <size_t N>
    void PECallWrapperImpl<ClassName, FunctionName, ReturnType, Args...>::InitializeArgInfo(const UFunction* Function, FunctionArgInfo<N>& FunctionArgs)
    {
        FunctionArgs.ParmsSize = Function->ParmsSize();
        FunctionArgs.ReturnValueOffset = Function->ReturnValueOffset();
        FunctionArgs.HasReturnValue = FunctionArgs.ReturnValueOffset != UINT16_MAX;

        int ArgIndex = 0;
        if (State::UsesFProperty) {
            for (FField* Field = Function->ChildProperties(); Field; Field = Field->Next) {
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
            for (UField* Child = Function->Children(); Child; Child = Child->Next()) {
                if (!Child->HasTypeFlag(CASTCLASS_FProperty))
                    continue;

                UProperty* Property = static_cast<UProperty*>(Child);
                if (Property->HasPropertyFlag(CPF_ReturnParm)) {
                    if (Property->Offset() == FunctionArgs.ReturnValueOffset)
                        FunctionArgs.ReturnValueSize = Property->ElementSize();
                    continue;
                }

                FunctionArgs.ArgOffsets[ArgIndex] = { Property->Offset(), Property->ElementSize(), Property->HasPropertyFlag(CPF_OutParm) };
                ArgIndex++;
            }
        }
    }
}

#pragma once
#include <uesdk/helpers/PECallWrapper.hpp>

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

        constexpr bool IsVoidRetType = std::is_void_v<ReturnType>;

        static std::once_flag SetupOnce;
        std::call_once(SetupOnce, [&] {
            InitializeArgInfo(Function, FunctionArgs);
        });

        if constexpr (std::is_void_v<ReturnType> && NumArgs == 0) {
            if (FunctionArgs.HasReturnValue || FunctionArgs.ParmsSize != 0)
                throw std::logic_error("Mismatched function signature: '" + Function->GetFullName() + "' expects non-void function, void function passed");

            Obj->ProcessEvent(Function, nullptr);
            return;
        }

        TlsArgBuffer Parms(FunctionArgs.ParmsSize);

        WriteInputArgs(Parms.GetData(), FunctionArgs, std::forward<Args>(args)...);
        Obj->ProcessEvent(Function, Parms.GetData());
        WriteOutputArgs(Parms.GetData(), FunctionArgs, std::forward<Args>(args)..., Function);

        DestroyParmsWithReturn<sizeof...(Args), Args...>(Parms.GetData(), FunctionArgs);

        if constexpr (!IsVoidRetType) {
            if (!FunctionArgs.HasReturnValue)
                throw std::logic_error("Mismatched return type: '" + Function->GetFullName() + "' expects no return value, but template specifies non-void return type");

            ReturnType Return(
                *reinterpret_cast<ReturnType*>(Parms.GetData() + FunctionArgs.ReturnValueOffset));
            return std::move(Return);
        }
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
    template <size_t N>
    void PECallWrapperImpl<ClassName, FunctionName, ReturnType, Args...>::WriteInputArgs(uint8_t* Parms, FunctionArgInfo<N>& FunctionArgs, Args&&... args)
    {
        auto WriteInputArg = [Parms](auto& Arg, ArgInfo& Info) {
            using ArgType = std::decay_t<decltype(Arg)>;
            new (Parms + Info.Offset) ArgType(std::forward<decltype(Arg)>(Arg));
        };

        size_t ArgIndex = 0;
        ((WriteInputArg(args, FunctionArgs.ArgOffsets[ArgIndex]), ++ArgIndex), ...);
    }

    template <StringLiteral ClassName, StringLiteral FunctionName, typename ReturnType, typename... Args>
    template <size_t N>
    void PECallWrapperImpl<ClassName, FunctionName, ReturnType, Args...>::WriteOutputArgs(uint8_t* Parms, FunctionArgInfo<N>& FunctionArgs, Args&&... args, UFunction* Function)
    {
        auto WriteOutputArg = [Parms, Function](auto& Arg, ArgInfo& Info) {
            using RawArgDecl = decltype(Arg);
            using ArgNoRef = std::remove_reference_t<RawArgDecl>;
            using PointeeType = std::remove_pointer_t<ArgNoRef>;

            // Pointer output parameters take priority
            if constexpr (std::is_pointer_v<ArgNoRef>) {
                if constexpr (std::is_const_v<PointeeType>) {
                    throw std::invalid_argument("Mismatched argument qualifiers: '" + Function->GetFullName() + "' Output pointer cannot be const!");
                }
                else {
                    if (!Arg)
                        return; // Nothing to write to, ignore

                    // Simple and fast memcpy for trivially copyable
                    if constexpr (std::is_trivially_copyable_v<PointeeType>) {
                        std::memcpy(reinterpret_cast<void*>(Arg), Parms + Info.Offset, static_cast<size_t>(Info.Size));
                    }
                    else {
                        // Assignment operator for non trivialy copyable
                        PointeeType* SrcPtr = reinterpret_cast<PointeeType*>(Parms + Info.Offset);
                        if constexpr (std::is_assignable_v<PointeeType&, PointeeType>) {
                            *Arg = *SrcPtr;
                        }
                        else {
                            throw std::invalid_argument("Unsupported out-parameter type for function '" + Function->GetFullName() + "': non-trivial pointee is not assignable or move-assignable");
                        }
                    }
                }
            }
            // Lvalue output parameters
            else if constexpr (std::is_lvalue_reference_v<RawArgDecl>) {
                using RefT = std::remove_reference_t<RawArgDecl>;

                if constexpr (std::is_const_v<RefT>) {
                    throw std::invalid_argument("Mismatched argument qualifiers: '" + Function->GetFullName() + "' Output lvalue reference cannot be const!");
                }
                else {
                    if constexpr (std::is_trivially_copyable_v<RefT>) {
                        std::memcpy(reinterpret_cast<void*>(std::addressof(Arg)), Parms + Info.Offset, static_cast<size_t>(Info.Size));
                    }
                    else {
                        RefT* srcPtr = reinterpret_cast<RefT*>(Parms + Info.Offset);
                        if constexpr (std::is_assignable_v<RefT&, RefT>) {
                            Arg = *srcPtr;
                        }
                        else {
                            throw std::invalid_argument("Unsupported out-parameter type for function '" + Function->GetFullName() + "': non-trivial reference is not assignable or move-assignable");
                        }
                    }
                }
            }
            // Invalid
            else {
                throw std::invalid_argument("Mismatched argument type: '" + Function->GetFullName() + "' Output argument must be lvalue reference or pointer!");
            }
        };

        size_t ArgIndex = 0;
        ((FunctionArgs.ArgOffsets[ArgIndex].IsOutParm ? WriteOutputArg(args, FunctionArgs.ArgOffsets[ArgIndex]) : void(), ++ArgIndex), ...);
    }

    template <StringLiteral ClassName, StringLiteral FunctionName, typename ReturnType, typename... Args>
    template <typename Param>
    void PECallWrapperImpl<ClassName, FunctionName, ReturnType, Args...>::DestroyParamSlot(uint8_t* ParmsBase, const ArgInfo& Info)
    {
        using RawDecl = std::remove_reference_t<Param>;

        uint8_t* Addr = ParmsBase + Info.Offset;
        if (Info.IsOutParm) {
            if constexpr (std::is_pointer_v<RawDecl>) {
                using Pointee = std::remove_pointer_t<RawDecl>;
                if constexpr (!std::is_trivially_destructible_v<Pointee> && std::is_destructible_v<Pointee>) {
                    reinterpret_cast<Pointee*>(Addr)->~Pointee();
                }
            }
            else {
                if constexpr (!std::is_trivially_destructible_v<RawDecl> && std::is_destructible_v<RawDecl>) {
                    reinterpret_cast<RawDecl*>(Addr)->~RawDecl();
                }
            }
        }
        else {
            using Decayed = std::decay_t<Param>;
            if constexpr (!std::is_trivially_destructible_v<Decayed> && std::is_destructible_v<Decayed>) {
                reinterpret_cast<Decayed*>(Addr)->~Decayed();
            }
        }
    }

    template <StringLiteral ClassName, StringLiteral FunctionName, typename ReturnType, typename... Args>
    template <size_t N, typename... ParamTypes, size_t... I>
    void PECallWrapperImpl<ClassName, FunctionName, ReturnType, Args...>::DestroyParmsImpl(uint8_t* ParmsBase, FunctionArgInfo<N>& FunctionArgs, std::index_sequence<I...>)
    {
        // Call DestroyParamSlot for each parameter in the pack
        (DestroyParamSlot<ParamTypes>(ParmsBase, FunctionArgs.ArgOffsets[I]), ...);
    }

    template <StringLiteral ClassName, StringLiteral FunctionName, typename ReturnType, typename... Args>
    template <size_t N, typename... ParamTypes>
    void PECallWrapperImpl<ClassName, FunctionName, ReturnType, Args...>::DestroyParms(uint8_t* ParmsBase, FunctionArgInfo<N>& FunctionArgs)
    {
        static_assert(sizeof...(ParamTypes) == N, "DestroyParms parameter count mismatch");

        DestroyParmsImpl<N, ParamTypes...>(
            ParmsBase,
            FunctionArgs,
            std::make_index_sequence<sizeof...(ParamTypes)> {});
    }

    template <StringLiteral ClassName, StringLiteral FunctionName, typename ReturnType, typename... Args>
    template <size_t N, typename... ParamTypes>
    void PECallWrapperImpl<ClassName, FunctionName, ReturnType, Args...>::DestroyParmsWithReturn(uint8_t* ParmsBase, FunctionArgInfo<N>& FunctionArgs)
    {
        DestroyParms<N, ParamTypes...>(ParmsBase, FunctionArgs);

        // Destroy return value if non-trivial
        if constexpr (!std::is_trivially_destructible_v<ReturnType>) {
            if (FunctionArgs.HasReturnValue) {
                reinterpret_cast<ReturnType*>(ParmsBase + FunctionArgs.ReturnValueOffset)->~ReturnType();
            }
        }
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

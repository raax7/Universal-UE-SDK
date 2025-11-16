#pragma once
#include <uesdk/Utils.hpp>
#include <uesdk/core/UnrealObjects.hpp>
#include <uesdk/helpers/TlsArgBuffer.hpp>

#include <array>
#include <atomic>
#include <exception>

namespace SDK
{
    template <StringLiteral ClassName, StringLiteral FunctionName, typename ReturnType, typename... Args>
    struct PECallWrapperImpl
    {
    public:
        ReturnType Call(UObject* Obj, UFunction* Function, Args... args);
        ReturnType CallAuto(UObject* Obj, Args... args);

    private:
        static constexpr size_t kMaxStackAllocSize = 4096;

    private:
        struct ArgInfo
        {
            int32_t Offset = 0;
            int32_t Size = 0;
            bool IsOutParm = false;
        };

        template <size_t NumArgs>
        struct FunctionArgInfo
        {
            std::array<ArgInfo, (NumArgs > 0 ? NumArgs : 1)> ArgOffsets = { 0 };
            size_t ParmsSize = 0;
            int32_t ReturnValueOffset = 0;
            int32_t ReturnValueSize = 0;
            bool HasReturnValue = false;
        };

    private:
        template <size_t N>
        void WriteInputArgs(uint8_t* Parms, FunctionArgInfo<N>& FunctionArgs, Args&&... args);

        template <size_t N>
        void WriteOutputArgs(uint8_t* Parms, FunctionArgInfo<N>& FunctionArgs, Args&&... args, UFunction* Function);

        template <typename Param>
        static void DestroyParamSlot(uint8_t* ParmsBase, const ArgInfo& Info);

        template <size_t N, typename... ParamTypes, size_t... I>
        static void DestroyParmsImpl(uint8_t* ParmsBase, FunctionArgInfo<N>& FunctionArgs, std::index_sequence<I...>);

        template <size_t N, typename... ParamTypes>
        static void DestroyParms(uint8_t* ParmsBase, FunctionArgInfo<N>& FunctionArgs);

    private:
        template <size_t N>
        void InitializeArgInfo(const UFunction* Function, FunctionArgInfo<N>& FunctionArgs);
    };

    // TODO: Add improvements for intellisense instead of this. Intellisense is so bad at handling NTTPs
    //

    template <typename T>
    struct FunctionTraits;

    template <typename R, typename... Args>
    struct FunctionTraits<R(Args...)>
    {
        using ReturnType = R;
        using ArgsTuple = std::tuple<Args...>;
        static constexpr size_t Arity = sizeof...(Args);
    };

    template <StringLiteral ClassName, StringLiteral FunctionName, typename ReturnType, typename... Args>
    struct PECallWrapperImpl;

    template <StringLiteral ClassName, StringLiteral FunctionName, typename FunctionSig>
    struct PECallWrapperSelector;

    template <StringLiteral ClassName, StringLiteral FunctionName, typename R, typename... Args>
    struct PECallWrapperSelector<ClassName, FunctionName, R(Args...)>
    {
        using type = PECallWrapperImpl<ClassName, FunctionName, R, Args...>;
    };

    /**
     * @brief Wrapper for calling a ProcessEvent function, automatically handling the parameter(s) structure.
     *
     * @tparam ClassName and FunctionName - Used to for unique template instantiation, as well as automatic function finding.
     * @tparam FunctionSig - Function signature to read return type and argument type(s) from.
     */
    template <StringLiteral ClassName, StringLiteral FunctionName, typename FunctionSig>
    struct PECallWrapper : public PECallWrapperSelector<ClassName, FunctionName, FunctionSig>::type
    {
    public:
        using Base = typename PECallWrapperSelector<ClassName, FunctionName, FunctionSig>::type;

        // PECallWrapper allows const UObject* as input for API consistency.
        // const is only promised at C++ level, UESDK does not guarentee UE will not modify the object in a non-const way.

        /**
         * @brief Calls a ProcessEvent function.
         * @brief Important points:
         * @brief - All arguments must match the order they are in the original UFunction.
         * @brief - Output parameters must end with pointer or reference.
         * @brief - Trivially copyable output parameters (e.g., FHitResult) can be any size; only the smaller of the output parameter or the actual struct is copied.
         *
         * @param[in] Function - A pointer to the UFunction to call.
         * @param[in,out] ...args - Arguments to be sent to the UFunction.
         *
         * @return The value returned from the UFunction call.
         *
         * @throws std::invalid_argument - If an output argument pointer is not writable.
         * @throws std::bad_alloc - If allocating memory for parameters on the stack failed.
         * @throws std::logic_error - If a return type was specified, but the UFunction does not have a return type.
         */
        template <typename UObjectType, typename... Args>
        auto Call(UObjectType* Obj, UFunction* Function, Args&&... args)
        {
            static_assert(std::is_base_of_v<SDK::UObject, std::remove_const_t<UObjectType>>,
                "Obj must be a UObject or const UObject");
            return Base::Call(const_cast<std::remove_const_t<UObjectType>*>(Obj), Function, args...);
        }

        /** @brief Wrapper to automatically find UFunction from template parameters. For full documentation read PECallWrapper::Call. */
        template <typename UObjectType, typename... Args>
        auto CallAuto(UObjectType* Obj, Args&&... args)
        {
            static_assert(std::is_base_of_v<SDK::UObject, std::remove_const_t<UObjectType>>,
                "Obj must be a UObject or const UObject");
            return Base::CallAuto(const_cast<std::remove_const_t<UObjectType>*>(Obj), args...);
        }
    };
}

#include <uesdk/helpers/PECallWrapper.inl>

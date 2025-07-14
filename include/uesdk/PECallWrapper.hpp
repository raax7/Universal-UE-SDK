#pragma once
#include <uesdk/UnrealObjects.hpp>
#include <uesdk/Utils.hpp>

#include <array>
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
        uint8_t* AllocateParams(size_t Size, bool& UsedHeap);
        void CleanupParams(uint8_t* Parms, bool UsedHeap);

        template <size_t N>
        void WriteInputArgs(uint8_t* Parms, FunctionArgInfo<N>& FunctionArgs, Args&&... args);

        template <size_t N>
        void WriteOutputArgs(uint8_t* Parms, FunctionArgInfo<N>& FunctionArgs, Args&&... args);

    private:
        template <size_t N>
        void InitializeArgInfo(const UFunction* Function, FunctionArgInfo<N>& FunctionArgs);
    };

    // My head hurts
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
     * @brief Wrapper for calling a ProcessEvent function, automatically handling the parameter structure.
     *
     * @tparam ClassName and FunctionName - Used to for unique template instantiation, as well as automatic function finding.
     * @tparam FunctionSig - Function signature to read return type and argument type(s) from.
     */
    template <StringLiteral ClassName, StringLiteral FunctionName, typename FunctionSig>
    struct PECallWrapper : public PECallWrapperSelector<ClassName, FunctionName, FunctionSig>::type
    {
    public:
        static inline int InstanceCounter = 0;

        PECallWrapper()
        {
            ++InstanceCounter;
            if (InstanceCounter > 1)
                throw std::runtime_error("Duplicate PECallWrapper found! Make sure ClassName and FunctionName are accurate");
        }

    public:
        using Base = typename PECallWrapperSelector<ClassName, FunctionName, FunctionSig>::type;

        /**
         * @brief Calls a ProcessEvent function.
         * @brief Requires the following:
         * @brief - All arguments must match the order they are in the original UFunction.
         * @brief - Output arguments can be larger than the actual struct; only the real size of the struct will be copied.
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
        template <typename... Args>
        auto Call(UObject* Obj, UFunction* Function, Args... args)
        {
            return Base::Call(Obj, Function, args...);
        }

        /** @brief Wrapper to automatically find UFunction from template parameters. For full documentation read PECallWrapper::Call. */
        template <typename... Args>
        auto CallAuto(UObject* Obj, Args... args)
        {
            return Base::CallAuto(Obj, args...);
        }
    };
}

#include <uesdk/PECallWrapper.inl>

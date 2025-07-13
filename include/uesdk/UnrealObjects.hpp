#pragma once
#include <uesdk/Macros.hpp>
#include <uesdk/UnrealContainers.hpp>
#include <uesdk/UnrealEnums.hpp>
#include <uesdk/UnrealTypes.hpp>
#include <uesdk/Utils.hpp>

#include <memory>
#include <stdexcept>
#include <string>

namespace SDK
{
    struct PropertyInfo
    {
        bool Found = false;
        int32_t Flags;
        int32_t Offset;
        uint8_t ByteMask;
    };
}

namespace SDK
{
    class UObject
    {
    private:
        UObject() = delete;
        ~UObject() = delete;

    public:
        void** VFT;
        DECLARE_GETTER_SETTER(int32_t, Flags);
        DECLARE_GETTER_SETTER(int32_t, Index);
        DECLARE_GETTER_SETTER(class UClass*, Class);
        DECLARE_GETTER_SETTER(class FName, Name);
        DECLARE_GETTER_SETTER(class UObject*, Outer);

    public:
        bool HasTypeFlag(EClassCastFlags TypeFlag) const;
        bool IsA(class UClass* Target) const;
        bool IsDefaultObject() const;

        std::string GetName() const;
        std::string GetFullName() const;

        void ProcessEventAsNative(class UFunction* Function, void* Parms);
        void ProcessEvent(class UFunction* Function, void* Parms);

        /**
         * @brief FastSearchSingle wrapper to retrieve the value of a member in a class.
         * @brief This does not search inhereted classes, so make sure you use the exact class name that contains the member you want.
         *
         * @tparam ClassName - The name of the UClass to search for the member in.
         * @tparam MemberName - The name of the member.
         * @tparam ObjectType - The type of the member.
         *
         * @return The value of the class member.
         *
         * @throws std::runtime_error - If FastSearchSingle was unable to find the member offset.
         */
        template <StringLiteral ClassName, StringLiteral MemberName, typename MemberType>
        MemberType GetMember();

        /** @brief Alternative version of UObject::GetMember that returns a pointer to the member and doesn't dereference it. For full documentation read the original UObject::GetMember function. */
        template <StringLiteral ClassName, StringLiteral MemberName, typename MemberType>
        MemberType* GetMemberPtr();

        /**
         * @brief FastSearchSingle wrapper to set the value of a member in a class.
         * @brief This does not search inhereted classes, so make sure you use the exact class name that contains the member you want.
         *
         * @tparam ClassName - The name of the UClass to search for the member in.
         * @tparam MemberName - The name of the member.
         * @tparam ObjectType - The type of the member.
         *
         * @param Value - The value to set the member to.
         *
         * @throws std::runtime_error - If FastSearchSingle was unable to find the member offset.
         */
        template <StringLiteral ClassName, StringLiteral MemberName, typename MemberType>
        void SetMember(MemberType Value);
    };

    class UField : public UObject
    {
    private:
        UField() = delete;
        ~UField() = delete;

    public:
        DECLARE_GETTER_SETTER(class UField*, Next);
    };

    class UStruct : public UField
    {
    private:
        UStruct() = delete;
        ~UStruct() = delete;

    public:
        DECLARE_GETTER_SETTER(class UStruct*, SuperStruct);
        DECLARE_GETTER_SETTER(class UField*, Children);
        DECLARE_GETTER_SETTER(FField*, ChildProperties); // May return nullptr depending if FProperties are used.
        DECLARE_GETTER_SETTER(int32_t, PropertiesSize);
        DECLARE_GETTER_SETTER(int32_t, MinAlignment);

    public:
        /**
         * @brief Finds a child matching the specified name and EClassCastFlags.
         *
         * @param[in] Name - Target child name.
         * @param[in] (optional) TypeFlag - Target EClassCastFlags for the child.
         *
         * @return A pointer to the child if found, else a nullptr.
         */
        UField* FindMember(const FName& Name, EClassCastFlags TypeFlag = CASTCLASS_None) const;

        /**
         * @brief Finds a target property, supporting UProperties and FProperties.
         *
         * @param[in] Name - Target property name.
         * @param[in] (optional) PropertyFlag - Target property EPropertyFlags.
         *
         * @return An instance of the PropertyInfo struct.
         */
        PropertyInfo FindProperty(const FName& Name, EPropertyFlags PropertyFlag = CPF_None) const;

        /** @brief Wrapper for FindMember to find a UFunction. */
        class UFunction* FindFunction(const FName& Name) const;
    };

    class UScriptStruct : public UStruct
    {
    public:
        UScriptStruct() = delete;
        ~UScriptStruct() = delete;
    };

    class UClass : public UStruct
    {
    private:
        UClass() = delete;
        ~UClass() = delete;

    public:
        DECLARE_GETTER_SETTER(EClassCastFlags, ClassCastFlags);
        DECLARE_GETTER_SETTER(UObject*, ClassDefaultObject);
    };

    class UProperty : public UField
    {
    private:
        UProperty() = delete;
        ~UProperty() = delete;

    public:
        bool HasPropertyFlag(EPropertyFlags PropertyFlag) const;

    public:
        DECLARE_GETTER_SETTER(int32_t, Offset);
        DECLARE_GETTER_SETTER(int32_t, ElementSize);
        DECLARE_GETTER_SETTER(EPropertyFlags, PropertyFlags);
    };

    class UBoolProperty : public UProperty
    {
    private:
        UBoolProperty() = delete;
        ~UBoolProperty() = delete;

    public:
        /** @return If the UBoolProperty is for a bitfield, or a native bool. */
        bool IsNativeBool() const;
        uint8_t GetFieldMask() const;
        uint8_t GetBitIndex() const;
    };

    class UEnum : public UField
    {
    private:
        UEnum() = delete;
        ~UEnum() = delete;

    public:
        DECLARE_GETTER_SETTER(TYPE_WRAPPER(TArray<TPair<FName, int64_t>>), Names);

    public:
        /**
         * @brief Finds an enumerator value based off of
         * @param Name - Target enumerator name.
         * @return The enumerator value if found, else OFFSET_NOT_FOUND.
         */
        int64_t FindEnumerator(const FName& Name) const;
    };

    class UFunction : public UStruct
    {
    private:
        UFunction() = delete;
        ~UFunction() = delete;

    public:
        using FNativeFuncPtr = void (*)(void* Context, void* TheStack, void* Result);

    public:
        DECLARE_GETTER_SETTER(EFunctionFlags, FunctionFlags);
        DECLARE_GETTER_SETTER(uint8_t, NumParms);
        DECLARE_GETTER_SETTER(uint16_t, ParmsSize);
        DECLARE_GETTER_SETTER(uint16_t, ReturnValueOffset);
        DECLARE_GETTER_SETTER(FNativeFuncPtr, Func);
    };

    class UDataTable : public UObject
    {
    public:
        UDataTable() = delete;
        ~UDataTable() = delete;

    public:
        DECLARE_GETTER_SETTER(UScriptStruct*, RowStruct);
        DECLARE_GETTER_SETTER(TYPE_WRAPPER(TMap<FName, uint8_t*>), RowMap);
    };
}

#include <uesdk/UnrealObjects.inl>

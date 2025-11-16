#pragma once
#include <uesdk/Offsets.hpp>
#include <uesdk/Utils.hpp>
#include <uesdk/core/UnrealContainers.hpp>
#include <uesdk/core/UnrealEnums.hpp>
#include <uesdk/core/UnrealTypes.hpp>
#include <uesdk/helpers/ReflectionMacros.hpp>

#include <memory>
#include <stdexcept>
#include <string>

namespace SDK
{
    class UObject
    {
    private:
        // TODO: Decide whether to delete or keep constructor and destructor.
        // UObject() = delete;
        //~UObject() = delete;

    public:
        // clang-format off

        void** VFT;
        UESDK_UPROPERTY_OFFSET(int32_t,         Flags,  Offsets::UObject::Flags);
        UESDK_UPROPERTY_OFFSET(int32_t,         Index,  Offsets::UObject::Index);
        UESDK_UPROPERTY_OFFSET(class UClass*,   Class,  Offsets::UObject::Class);
        UESDK_UPROPERTY_OFFSET(class FName,     Name,   Offsets::UObject::Name);
        UESDK_UPROPERTY_OFFSET(class UObject*,  Outer,  Offsets::UObject::Outer);

        // clang-format on

    public:
        bool HasTypeFlag(EClassCastFlags TypeFlag) const;
        bool IsA(class UClass* Target) const;
        bool IsDefaultObject() const;

        std::string GetName() const;
        std::string GetFullName() const;

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
        UESDK_UPROPERTY_OFFSET(class UField*, Next, SDK::Offsets::UField::Next);
    };

    class UStruct : public UField
    {
    private:
        UStruct() = delete;
        ~UStruct() = delete;

    public:
        // clang-format off

        UESDK_UPROPERTY_OFFSET(class UStruct*,  SuperStruct,        SDK::Offsets::UStruct::SuperStruct);
        UESDK_UPROPERTY_OFFSET(class UField*,   Children,           SDK::Offsets::UStruct::Children);
        UESDK_UPROPERTY_OFFSET(FField*,         ChildProperties,    SDK::Offsets::UStruct::ChildProperties); // May return nullptr depending if FProperties are used.
        UESDK_UPROPERTY_OFFSET(int32_t,         PropertiesSize,     SDK::Offsets::UStruct::PropertiesSize);
        UESDK_UPROPERTY_OFFSET(int32_t,         MinAlignment,       SDK::Offsets::UStruct::MinAlignment);

        // clang-format on

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
        // clang-format off

        UESDK_UPROPERTY_OFFSET(EClassCastFlags, ClassCastFlags,     SDK::Offsets::UClass::ClassCastFlags);
        UESDK_UPROPERTY_OFFSET(UObject*,        ClassDefaultObject, SDK::Offsets::UClass::ClassDefaultObject);

        // clang-format on
    };

    class UProperty : public UField
    {
    private:
        UProperty() = delete;
        ~UProperty() = delete;

    public:
        bool HasPropertyFlag(EPropertyFlags PropertyFlag) const;

    public:
        // clang-format off

        UESDK_UPROPERTY_OFFSET(int32_t,         Offset,         SDK::Offsets::UProperty::Offset);
        UESDK_UPROPERTY_OFFSET(int32_t,         ElementSize,    SDK::Offsets::UProperty::ElementSize);
        UESDK_UPROPERTY_OFFSET(EPropertyFlags,  PropertyFlags,  SDK::Offsets::UProperty::PropertyFlags);

        // clang-format on
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
        UESDK_UPROPERTY_OFFSET(UESDK_TYPE(TArray<TPair<FName, int64_t>>), Names, SDK::Offsets::UEnum::Names);

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
        // clang-format off

        UESDK_UPROPERTY_OFFSET(EFunctionFlags,  FunctionFlags,      SDK::Offsets::UFunction::FunctionFlags);
        UESDK_UPROPERTY_OFFSET(uint8_t,         NumParms,           SDK::Offsets::UFunction::NumParms);
        UESDK_UPROPERTY_OFFSET(uint16_t,        ParmsSize,          SDK::Offsets::UFunction::ParmsSize);
        UESDK_UPROPERTY_OFFSET(uint16_t,        ReturnValueOffset,  SDK::Offsets::UFunction::ReturnValueOffset);
        UESDK_UPROPERTY_OFFSET(FNativeFuncPtr,  Func,               SDK::Offsets::UFunction::Func);

        // clang-format on
    };

    class UDataTable : public UObject
    {
    public:
        UDataTable() = delete;
        ~UDataTable() = delete;

    public:
        // clang-format off

        UESDK_UPROPERTY_OFFSET(UScriptStruct*,                      RowStruct,  SDK::Offsets::UDataTable::RowStruct);
        UESDK_UPROPERTY_OFFSET(UESDK_TYPE(TMap<FName, uint8_t*>),   RowMap,     SDK::Offsets::UDataTable::RowMap);

        // clang-format on
    };
}

#include <uesdk/core/UnrealObjects.inl>

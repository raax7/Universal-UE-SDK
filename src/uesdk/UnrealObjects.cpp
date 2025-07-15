#pragma once
#include <private/Offsets.hpp>
#include <uesdk/State.hpp>
#include <uesdk/UnrealContainers.hpp>
#include <uesdk/UnrealObjects.hpp>

namespace SDK
{
    DEFINE_GETTER_SETTER(UObject, int32_t, Flags, Offsets::UObject::Flags)
    DEFINE_GETTER_SETTER(UObject, int32_t, Index, Offsets::UObject::Index)
    DEFINE_GETTER_SETTER(UObject, UClass*, Class, Offsets::UObject::Class)
    DEFINE_GETTER_SETTER(UObject, FName, Name, Offsets::UObject::Name)
    DEFINE_GETTER_SETTER(UObject, UObject*, Outer, Offsets::UObject::Outer)

    DEFINE_GETTER_SETTER(UField, UField*, Next, Offsets::UField::Next);

    DEFINE_GETTER_SETTER(UStruct, UStruct*, SuperStruct, Offsets::UStruct::SuperStruct);
    DEFINE_GETTER_SETTER(UStruct, UField*, Children, Offsets::UStruct::Children);
    DEFINE_GETTER_SETTER(UStruct, FField*, ChildProperties, Offsets::UStruct::ChildProperties);
    DEFINE_GETTER_SETTER(UStruct, int32_t, PropertiesSize, Offsets::UStruct::PropertiesSize);
    DEFINE_GETTER_SETTER(UStruct, int32_t, MinAlignment, Offsets::UStruct::MinAlignment);

    DEFINE_GETTER_SETTER(UClass, EClassCastFlags, ClassCastFlags, Offsets::UClass::ClassCastFlags);
    DEFINE_GETTER_SETTER(UClass, UObject*, ClassDefaultObject, Offsets::UClass::ClassDefaultObject);

    DEFINE_GETTER_SETTER(UProperty, int32_t, Offset, Offsets::UProperty::Offset);
    DEFINE_GETTER_SETTER(UProperty, int32_t, ElementSize, Offsets::UProperty::ElementSize);
    DEFINE_GETTER_SETTER(UProperty, EPropertyFlags, PropertyFlags, Offsets::UProperty::PropertyFlags);

    DEFINE_GETTER_SETTER(UEnum, TYPE_WRAPPER(TArray<TPair<FName, int64_t>>), Names, Offsets::UEnum::Names);

    DEFINE_GETTER_SETTER(UFunction, EFunctionFlags, FunctionFlags, Offsets::UFunction::FunctionFlags);
    DEFINE_GETTER_SETTER(UFunction, uint8_t, NumParms, Offsets::UFunction::NumParms);
    DEFINE_GETTER_SETTER(UFunction, uint16_t, ParmsSize, Offsets::UFunction::ParmsSize);
    DEFINE_GETTER_SETTER(UFunction, uint16_t, ReturnValueOffset, Offsets::UFunction::ReturnValueOffset);
    DEFINE_GETTER_SETTER(UFunction, UFunction::FNativeFuncPtr, Func, Offsets::UFunction::FuncOffset);

    DEFINE_GETTER_SETTER(UDataTable, UScriptStruct*, RowStruct, Offsets::UDataTable::RowStruct);
    DEFINE_GETTER_SETTER(UDataTable, TYPE_WRAPPER(TMap<FName, uint8_t*>), RowMap, Offsets::UDataTable::RowMap);

    bool UObject::HasTypeFlag(EClassCastFlags TypeFlag) const
    {
        return TypeFlag != CASTCLASS_None ? Class()->ClassCastFlags() & TypeFlag : true;
    }
    bool UObject::IsA(UClass* Target) const
    {
        for (UStruct* Super = Class(); Super; Super = Super->SuperStruct()) {
            if (Super == Target) {
                return true;
            }
        }

        return false;
    }
    bool UObject::IsDefaultObject() const
    {
        return ((Flags() & RF_ClassDefaultObject) == RF_ClassDefaultObject);
    }
    std::string UObject::GetName() const
    {
        return Name().ToString();
    }
    std::string UObject::GetFullName() const
    {
        if (Class()) {
            std::string Temp;

            for (UObject* NextOuter = Outer(); NextOuter; NextOuter = NextOuter->Outer()) {
                Temp = NextOuter->GetName() + "." + Temp;
            }

            std::string Name = Class()->GetName();
            Name += " ";
            Name += Temp;
            Name += GetName();

            return Name;
        }

        return "None";
    }
    void UObject::ProcessEvent(UFunction* Function, void* Parms)
    {
        using ProcessEvent_t = void (*)(UObject*, UFunction*, void*);
        ProcessEvent_t PE = reinterpret_cast<ProcessEvent_t>(VFT[Offsets::UObject::ProcessEventIdx]);
        PE(this, Function, Parms);
    }

    UField* UStruct::FindMember(const FName& Name, EClassCastFlags TypeFlag) const
    {
        for (UField* Child = Children(); Child; Child = Child->Next()) {
            if (Child->HasTypeFlag(TypeFlag) && Child->Name() == Name)
                return Child;
        }

        return nullptr;
    }
    PropertyInfo UStruct::FindProperty(const FName& Name, EPropertyFlags PropertyFlag) const
    {
        PropertyInfo Result = { .Found = false };

        if (State::UsesFProperty) {
            for (FField* Field = ChildProperties(); Field; Field = Field->Next) {
                if (!Field->HasTypeFlag(CASTCLASS_FProperty))
                    continue;

                FProperty* Property = reinterpret_cast<FProperty*>(Field);
                if (!Property->HasPropertyFlag(PropertyFlag))
                    continue;

                if (Field->Name == Name) {
                    Result.Found = true;
                    Result.Flags = Property->PropertyFlags;
                    Result.Offset = Property->Offset;

                    if (Property->HasTypeFlag(CASTCLASS_FBoolProperty)) {
                        FBoolProperty* BoolProperty = reinterpret_cast<FBoolProperty*>(Property);
                        if (!BoolProperty->IsNativeBool()) {
                            Result.ByteMask = BoolProperty->GetFieldMask();
                        }
                    }

                    return Result;
                }
            }
        }
        else {
            for (UField* Child = Children(); Child; Child = Child->Next()) {
                if (!Child->HasTypeFlag(CASTCLASS_FProperty))
                    continue;

                UProperty* Property = reinterpret_cast<UProperty*>(Child);
                if (!Property->HasPropertyFlag(PropertyFlag))
                    continue;

                if (Child->Name() == Name) {
                    Result.Found = true;
                    Result.Flags = Child->Flags();
                    Result.Offset = Property->Offset();

                    if (Property->HasTypeFlag(CASTCLASS_FBoolProperty)) {
                        UBoolProperty* BoolProperty = reinterpret_cast<UBoolProperty*>(Property);
                        if (!BoolProperty->IsNativeBool()) {
                            Result.ByteMask = BoolProperty->GetFieldMask();
                        }
                    }

                    return Result;
                }
            }
        }

        return Result;
    }
    UFunction* UStruct::FindFunction(const FName& Name) const
    {
        return reinterpret_cast<UFunction*>(FindMember(Name, CASTCLASS_UFunction));
    }

    bool UProperty::HasPropertyFlag(EPropertyFlags PropertyFlag) const
    {
        return PropertyFlag != CPF_None ? PropertyFlags() & PropertyFlag : true;
    }

    bool UBoolProperty::IsNativeBool() const
    {
        return GetFieldMask() == 0xFF;
    }
    uint8_t UBoolProperty::GetFieldMask() const
    {
        return reinterpret_cast<Offsets::UBoolProperty::UBoolPropertyBase*>((uintptr_t)this + Offsets::UBoolProperty::Base)->FieldMask;
    }
    uint8_t UBoolProperty::GetBitIndex() const
    {
        uint8_t FieldMask = GetFieldMask();

        if (FieldMask != 0xFF) {
            if (FieldMask == 0x01)
                return 0;
            if (FieldMask == 0x02)
                return 1;
            if (FieldMask == 0x04)
                return 2;
            if (FieldMask == 0x08)
                return 3;
            if (FieldMask == 0x10)
                return 4;
            if (FieldMask == 0x20)
                return 5;
            if (FieldMask == 0x40)
                return 6;
            if (FieldMask == 0x80)
                return 7;
        }

        return 0xFF;
    }

    int64_t UEnum::FindEnumerator(const FName& Name) const
    {
        auto NamesArray = Names();
        for (auto& It : NamesArray) {
            if (It.Key() == Name) {
                return It.Value();
            }
        }

        return OFFSET_NOT_FOUND;
    }
}

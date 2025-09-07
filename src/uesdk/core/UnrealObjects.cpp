#pragma once
#include <uesdk/Offsets.hpp>
#include <uesdk/State.hpp>
#include <uesdk/core/UnrealContainers.hpp>
#include <uesdk/core/UnrealObjects.hpp>

namespace SDK
{
    bool UObject::HasTypeFlag(EClassCastFlags TypeFlag) const
    {
        return TypeFlag != CASTCLASS_None ? Class->ClassCastFlags & TypeFlag : true;
    }
    bool UObject::IsA(UClass* Target) const
    {
        for (UStruct* Super = Class; Super; Super = Super->SuperStruct) {
            if (Super == Target) {
                return true;
            }
        }

        return false;
    }
    bool UObject::IsDefaultObject() const
    {
        return ((Flags & RF_ClassDefaultObject) == RF_ClassDefaultObject);
    }
    std::string UObject::GetName() const
    {
        return Name.ToString();
    }
    std::string UObject::GetFullName() const
    {
        if (Class) {
            std::string Temp;

            for (UObject* NextOuter = Outer; NextOuter; NextOuter = NextOuter->Outer) {
                Temp = NextOuter->GetName() + "." + Temp;
            }

            std::string Name = Class->GetName();
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
        for (UField* Child = Children; Child; Child = Child->Next) {
            if (Child->HasTypeFlag(TypeFlag) && Child->Name == Name)
                return Child;
        }

        return nullptr;
    }
    PropertyInfo UStruct::FindProperty(const FName& Name, EPropertyFlags PropertyFlag) const
    {
        PropertyInfo Result = { .Found = false };

        if (State::UsesFProperty) {
            for (FField* Field = ChildProperties; Field; Field = Field->Next) {
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
            for (UField* Child = Children; Child; Child = Child->Next) {
                if (!Child->HasTypeFlag(CASTCLASS_FProperty))
                    continue;

                UProperty* Property = reinterpret_cast<UProperty*>(Child);
                if (!Property->HasPropertyFlag(PropertyFlag))
                    continue;

                if (Child->Name == Name) {
                    Result.Found = true;
                    Result.Flags = Child->Flags;
                    Result.Offset = Property->Offset;

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
        return PropertyFlag != CPF_None ? PropertyFlags & PropertyFlag : true;
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
            switch (FieldMask) {
            case 0x01:
                return 0;
            case 0x02:
                return 1;
            case 0x04:
                return 2;
            case 0x08:
                return 3;
            case 0x10:
                return 4;
            case 0x20:
                return 5;
            case 0x40:
                return 6;
            case 0x80:
                return 7;
            }
        }

        return 0xFF;
    }

    int64_t UEnum::FindEnumerator(const FName& Name) const
    {
        auto NamesArray = Names;
        for (auto& It : NamesArray) {
            if (It.Key() == Name) {
                return It.Value();
            }
        }

        return OFFSET_NOT_FOUND;
    }
}

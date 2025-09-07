#include <uesdk/Offsets.hpp>
#include <uesdk/core/ObjectArray.hpp>
#include <uesdk/core/UnrealTypes.hpp>

namespace SDK
{
    FName::FName(const char* Str)
        : ComparisonIndex(0)
        , Number(0)
    {
        static void (*ConstructorNarrow)(const FName*, const char*, bool) = nullptr;

        if (!ConstructorNarrow)
            ConstructorNarrow = reinterpret_cast<void (*)(const FName*, const char*, bool)>(Offsets::FName::ConstructorNarrow);

        ConstructorNarrow(const_cast<FName*>(this), Str, true);
    }
    FName::FName(const wchar_t* Str)
        : ComparisonIndex(0)
        , Number(0)
    {
        static void (*ConstructorWide)(const FName*, const wchar_t*, bool) = nullptr;

        if (!ConstructorWide)
            ConstructorWide = reinterpret_cast<void (*)(const FName*, const wchar_t*, bool)>(Offsets::FName::ConstructorWide);

        ConstructorWide(const_cast<FName*>(this), Str, true);
    }

    std::string FName::GetRawString() const
    {
        static void (*AppendString)(const FName*, FString*) = nullptr;

        if (!AppendString)
            AppendString = reinterpret_cast<void (*)(const FName*, FString*)>(Offsets::FName::AppendString);

        FString TempString;
        AppendString(const_cast<FName*>(this), &TempString);

        return TempString.ToString();
    }
    std::string FName::ToString() const
    {
        std::string OutputString = GetRawString();
        size_t pos = OutputString.rfind('/');

        if (pos == std::string::npos)
            return OutputString;

        return OutputString.substr(pos + 1);
    }

    bool FField::HasTypeFlag(EClassCastFlags TypeFlag) const
    {
        if (ClassPrivate == nullptr)
            return false;

        return TypeFlag != CASTCLASS_None ? ClassPrivate->CastFlags & TypeFlag : true;
    }

    bool FProperty::HasPropertyFlag(EPropertyFlags PropertyFlag) const
    {
        return PropertyFlag != CPF_None ? PropertyFlags & PropertyFlag : true;
    }

    bool FBoolProperty::IsNativeBool()
    {
        return GetFieldMask() == 0xFF;
    }
    uint8_t FBoolProperty::GetFieldMask()
    {
        return FieldMask;
    }
    uint8_t FBoolProperty::GetBitIndex()
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

    class UObject* FWeakObjectPtr::Get() const
    {
        return GObjects->GetByIndex(ObjectIndex);
    }
    class UObject* FWeakObjectPtr::operator->() const
    {
        return GObjects->GetByIndex(ObjectIndex);
    }
    bool FWeakObjectPtr::operator==(const FWeakObjectPtr& Other) const
    {
        return ObjectIndex == Other.ObjectIndex;
    }
    bool FWeakObjectPtr::operator!=(const FWeakObjectPtr& Other) const
    {
        return ObjectIndex != Other.ObjectIndex;
    }
    bool FWeakObjectPtr::operator==(const class UObject* Other) const
    {
        return ObjectIndex == Other->Index;
    }
    bool FWeakObjectPtr::operator!=(const class UObject* Other) const
    {
        return ObjectIndex != Other->Index;
    }
}

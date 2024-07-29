#pragma once

#include <ugsdk/UnrealContainers.hpp>
#include <ugsdk/UnrealEnums.hpp>

namespace SDK
{
    class FFieldVariant
    {
    public:
        using ContainerType = union
        {
            class FField* Field;
            class UObject* Object;
        };

        ContainerType Container;
        bool bIsUObject;
    };

    class FFieldClass
    {
    public:
        FName Name;
        uint64_t Id;
        uint64_t CastFlags;
        EClassFlags ClassFlags;
        uint8_t Pad_3[0x4];
        class FFieldClass* SuperClass;
    };

    class FField
    {
    public:
        void** VFT;
        FFieldClass* ClassPrivate;
        FFieldVariant Owner;
        FField* Next;
        FName Name;
        int32_t ObjFlags;

    public:
        bool HasTypeFlag(EClassCastFlags TypeFlag) const;
    };

    class FProperty : public FField
    {
    public:
        int32_t ArrayDim;
        int32_t ElementSize;
        uint64_t PropertyFlags;
        uint8_t Pad_4[0x4];
        int32_t Offset;
        uint8_t Pad_5[0x28];

    public:
        bool HasPropertyFlag(EPropertyFlags PropertyFlag) const;
    };

    class FBoolProperty final : public FProperty
    {
    public:
        uint8_t FieldSize;
        uint8_t ByteOffset;
        uint8_t ByteMask;
        uint8_t FieldMask;

    public:
        bool IsNativeBool() const;
        uint8_t GetFieldMask() const;
        uint8_t GetBitIndex() const;
    };

    struct FWeakObjectPtr
    {
    public:
        int32_t ObjectIndex;
        int32_t ObjectSerialNumber;

    public:
        FWeakObjectPtr() { Reset(); }
        FWeakObjectPtr(const class UObject* Object) { (*this) = Object; }

        void operator=(const class UObject* Object);
        FWeakObjectPtr& operator=(const FWeakObjectPtr& Other) = default;
        bool operator==(const FWeakObjectPtr& Other) const
        {
            return (ObjectIndex == Other.ObjectIndex && ObjectSerialNumber == Other.ObjectSerialNumber) /*|| (!IsValid() && !Other.IsValid())*/;
        }
        bool operator!=(const FWeakObjectPtr& Other) const
        {
            return (ObjectIndex != Other.ObjectIndex || ObjectSerialNumber != Other.ObjectSerialNumber) /*&& (IsValid() || Other.IsValid())*/;
        }

    public:
        bool HasSameIndexAndSerialNumber(const FWeakObjectPtr& Other) const
        {
            return ObjectIndex == Other.ObjectIndex && ObjectSerialNumber == Other.ObjectSerialNumber;
        }

        void Reset()
        {
            ObjectIndex = -1;
            ObjectSerialNumber = 0;
        }
    };
}
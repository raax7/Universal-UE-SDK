#pragma once
#include <uesdk/UnrealContainers.hpp>
#include <uesdk/UnrealEnums.hpp>

namespace SDK
{
    class FName
    {
    public:
        FName(const std::string& Str)
            : FName(Str.c_str())
        { }
        FName(const std::wstring& Str)
            : FName(Str.c_str())
        { }
        FName(const char* Str);
        FName(const wchar_t* Str);
        FName() = default;
        ~FName() = default;

    public:
        inline bool operator==(const FName& Other) const { return ComparisonIndex == Other.ComparisonIndex; }
        inline bool operator!=(const FName& Other) const { return ComparisonIndex != Other.ComparisonIndex; }
        inline bool IsNone() const { return !ComparisonIndex; }

    public:
        uint32_t ComparisonIndex;
        uint32_t Number;

    public:
        std::string GetRawString() const;
        std::string ToString() const;
    };

    class FTextData
    {
    public:
        uint8_t Pad_0[0x28];
        class FString TextSource;
    };
    class FText
    {
    public:
        class FTextData* TextData;
        uint8_t Pad_8[0x10];

    public:
        const class FString& GetStringRef() const
        {
            return TextData->TextSource;
        }
        std::string ToString() const
        {
            return TextData->TextSource.ToString();
        }
    };

    struct FWeakObjectPtr
    {
    public:
        int32_t ObjectIndex;
        int32_t ObjectSerialNumber;

    public:
        class UObject* Get() const;
        class UObject* operator->() const;
        bool operator==(const FWeakObjectPtr& Other) const;
        bool operator!=(const FWeakObjectPtr& Other) const;
        bool operator==(const class UObject* Other) const;
        bool operator!=(const class UObject* Other) const;
    };
    template <typename UEType>
    class TWeakObjectPtr : public FWeakObjectPtr
    {
    public:
        UEType* Get() const
        {
            return static_cast<UEType*>(FWeakObjectPtr::Get());
        }

        UEType* operator->() const
        {
            return static_cast<UEType*>(FWeakObjectPtr::Get());
        }
    };

    struct FSoftObjectPath
    {
    public:
        FName AssetPathName;
        FString SubPathString;
    };
    template <typename TObjectID>
    class TPersistentObjectPtr
    {
    public:
        FWeakObjectPtr WeakPtr;
        int32_t TagAtLastTest;
        TObjectID ObjectID;

    public:
        class UObject* Get() const
        {
            return WeakPtr.Get();
        }
        class UObject* operator->() const
        {
            return WeakPtr.Get();
        }
    };

    class FSoftObjectPtr : public TPersistentObjectPtr<FSoftObjectPath>
    {
    };
    template <typename UEType>
    class TSoftObjectPtr : public FSoftObjectPtr
    {
    public:
        UEType* Get() const
        {
            return static_cast<UEType*>(TPersistentObjectPtr::Get());
        }
        UEType* operator->() const
        {
            return static_cast<UEType*>(TPersistentObjectPtr::Get());
        }
    };

    template <typename UEType>
    class TSoftClassPtr : public FSoftObjectPtr
    {
    public:
        UEType* Get() const
        {
            return static_cast<UEType*>(TPersistentObjectPtr::Get());
        }
        UEType* operator->() const
        {
            return static_cast<UEType*>(TPersistentObjectPtr::Get());
        }
    };

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
        bool IsNativeBool();
        uint8_t GetFieldMask();
        uint8_t GetBitIndex();
    };
}

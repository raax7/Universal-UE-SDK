#pragma once
#include <uesdk/Macros.hpp>
#include <cstdint>

namespace SDK::Offsets
{
    using Offset_t = int32_t;

    namespace UObject
    {
        constexpr Offset_t VFT = 0; // This will always be 0.
        inline Offset_t Flags = 8;
        inline Offset_t Index = 0xC;
        inline Offset_t Class = 0x10;
        inline Offset_t Name = 0x18;
        inline Offset_t Outer = 0x20;

        inline Offset_t ProcessEventIdx = OFFSET_NOT_FOUND;
    }
    namespace UField
    {
        inline Offset_t Next = OFFSET_NOT_FOUND;
    }
    namespace UStruct
    {
        inline Offset_t SuperStruct = OFFSET_NOT_FOUND;
        inline Offset_t Children = OFFSET_NOT_FOUND;
        inline Offset_t ChildProperties = OFFSET_NOT_FOUND;
        inline Offset_t PropertiesSize = OFFSET_NOT_FOUND;
        inline Offset_t MinAlignment = OFFSET_NOT_FOUND;
    }
    namespace UClass
    {
        inline Offset_t ClassCastFlags = OFFSET_NOT_FOUND;
        inline Offset_t ClassDefaultObject = OFFSET_NOT_FOUND;
    }
    namespace UProperty
    {
        inline Offset_t Offset = OFFSET_NOT_FOUND;
        inline Offset_t ElementSize = OFFSET_NOT_FOUND;
        inline Offset_t PropertyFlags = OFFSET_NOT_FOUND;
    }
    namespace UBoolProperty
    {
        struct UBoolPropertyBase
        {
            uint8_t FieldSize;
            uint8_t ByteOffset;
            uint8_t ByteMask;
            uint8_t FieldMask;
        };

        inline Offset_t Base = OFFSET_NOT_FOUND;
    }
    namespace UEnum
    {
        inline Offset_t Names = OFFSET_NOT_FOUND;
    }
    namespace UFunction
    {
        inline Offset_t FunctionFlags = OFFSET_NOT_FOUND;
        inline Offset_t NumParms = OFFSET_NOT_FOUND;
        inline Offset_t ParmsSize = OFFSET_NOT_FOUND;
        inline Offset_t ReturnValueOffset = OFFSET_NOT_FOUND;
        inline Offset_t Func = OFFSET_NOT_FOUND;
    }
    namespace UDataTable
    {
        inline Offset_t RowStruct = OFFSET_NOT_FOUND;
        inline Offset_t RowMap = OFFSET_NOT_FOUND;
    }

    namespace FMemory
    {
        inline uintptr_t Realloc = 0;
    }
    namespace FName
    {
        inline uintptr_t ConstructorNarrow = 0;
        inline uintptr_t ConstructorWide = 0;
        inline uintptr_t AppendString = 0;
    }
}

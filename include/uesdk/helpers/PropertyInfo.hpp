#pragma once
#include <cstdint>

namespace SDK
{
    constexpr int32_t OFFSET_NOT_FOUND = INT32_MAX;

    /* @brief Struct used for finding properties. */
    struct PropertyInfo
    {
        bool Found = false;
        int32_t Offset = 0;
        uint8_t ByteMask = 0;
        uint64_t Flags = 0;
        union
        {
            class UProperty* Prop;
            class FProperty* FProp;
        };
    };
}

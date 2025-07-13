#pragma once
#include <algorithm>
#include <cstdint>

namespace SDK
{
    template <size_t N>
    struct StringLiteral
    {
        char Value[N] {};

        constexpr StringLiteral(const char (&Str)[N])
        {
            for (size_t i = 0; i < N; ++i)
                Value[i] = Str[i];
        }

        constexpr const char* c_str() const { return Value; }

        constexpr bool operator==(const StringLiteral<N>& other) const
        {
            for (size_t i = 0; i < N; ++i)
                if (Value[i] != other.Value[i])
                    return false;
            return true;
        }
    };
}

#pragma once
#include <type_traits>

#define ENUM_OPERATORS(EEnumClass)                                                                                                                                      \
    inline constexpr EEnumClass operator|(EEnumClass Left, EEnumClass Right)                                                                                            \
    {                                                                                                                                                                   \
        return (EEnumClass)((std::underlying_type<EEnumClass>::type)(Left) | (std::underlying_type<EEnumClass>::type)(Right));                                          \
    }                                                                                                                                                                   \
                                                                                                                                                                        \
    inline constexpr EEnumClass& operator|=(EEnumClass& Left, EEnumClass Right)                                                                                         \
    {                                                                                                                                                                   \
        return (EEnumClass&)((std::underlying_type<EEnumClass>::type&)(Left) |= (std::underlying_type<EEnumClass>::type)(Right));                                       \
    }                                                                                                                                                                   \
                                                                                                                                                                        \
    inline bool operator&(EEnumClass Left, EEnumClass Right)                                                                                                            \
    {                                                                                                                                                                   \
        return (((std::underlying_type<EEnumClass>::type)(Left) & (std::underlying_type<EEnumClass>::type)(Right)) == (std::underlying_type<EEnumClass>::type)(Right)); \
    }

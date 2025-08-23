#pragma once
#include <cstdint>
#include <type_traits>

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

/* @brief Small fix for templated types */
#define UESDK_TYPE(...) __VA_ARGS__

// TODO: Add better support for blueprint classes
/* @brief Sets up StaticClass() and GetDefaultObj() functions. */
#define UESDK_DEFINEOBJECT(ClassNameStr, Type)                                          \
    static UClass* StaticClass()                                                        \
    {                                                                                   \
        static UClass* Clss = nullptr;                                                  \
        if (!Clss)                                                                      \
            Clss = GObjects->FindObjectFast<UClass>(ClassNameStr, CASTCLASS_UClass);    \
        return Clss;                                                                    \
    }                                                                                   \
    static Type* GetDefaultObj()                                                        \
    {                                                                                   \
        static Type* Default = nullptr;                                                 \
        if (!Default)                                                                   \
            Default = static_cast<Type*>(StaticClass()->ClassDefaultObject);            \
        return Default;                                                                 \
    }                                                                                   \
    static constexpr const char* g_ClassName = ClassNameStr

/* @brief Internal implementation details of UESDK_UPROPERTY */
#define UESDK_UPROPERTY_IMPL(Type, Name)                                                                                    \
    static const PropertyInfo& getpropinfo_##Name(bool SuppressFailure = false)                                             \
    {                                                                                                                       \
        static PropertyInfo Prop = GObjects->FindObjectFast<UStruct>(g_ClassName, CASTCLASS_UStruct)->FindProperty(#Name);  \
        return Prop;                                                                                                        \
    }                                                                                                                       \
    static const bool ispropvalid_##Name()                                                                                  \
    {                                                                                                                       \
        const PropertyInfo& Prop = getpropinfo_##Name();                                                                    \
        return Prop.Found;                                                                                                  \
    }

/* @brief Standard macro for defining a UPROPERTY. Does not support bit-fields */
#define UESDK_UPROPERTY(Type, Name)                                                    \
    UESDK_UPROPERTY_IMPL(Type, Name)                                                   \
    inline void putprop_##Name(const Type& v)                                          \
    {                                                                                  \
        const PropertyInfo& Prop = getpropinfo_##Name();                               \
        *reinterpret_cast<Type*>((uint8_t*)this + Prop.Offset) = const_cast<Type&>(v); \
    }                                                                                  \
    inline void putprop_##Name(Type& v)                                                \
    {                                                                                  \
        const PropertyInfo& Prop = getpropinfo_##Name();                               \
        *reinterpret_cast<Type*>((uint8_t*)this + Prop.Offset) = v;                    \
    }                                                                                  \
    inline Type& getprop_##Name() const                                                \
    {                                                                                  \
        const PropertyInfo& Prop = getpropinfo_##Name();                               \
        return *reinterpret_cast<Type*>((uint8_t*)this + Prop.Offset);                 \
    }                                                                                  \
    __declspec(property(get = getprop_##Name, put = putprop_##Name)) Type Name

/* @brief Macro for UPROPERTIES that are bit-fields with fallback to handling as a bool */
#define UESDK_UPROPERTY_BITFIELD(Name)                                                   \
    UESDK_UPROPERTY_IMPL(Type, Name)                                                     \
    inline void putprop_##Name(const bool& v)                                            \
    {                                                                                    \
        const PropertyInfo& Prop = getpropinfo_##Name();                                 \
        if (Prop.ByteMask) {                                                             \
            auto& ByteValue = *reinterpret_cast<uint8_t*>((uint8_t*)this + Prop.Offset); \
            ByteValue &= ~Prop.ByteMask;                                                 \
            if (v) {                                                                     \
                ByteValue |= Prop.ByteMask;                                              \
            }                                                                            \
            return;                                                                      \
        }                                                                                \
        *reinterpret_cast<bool*>((uint8_t*)this + Prop.Offset) = const_cast<bool&>(v);   \
    }                                                                                    \
    inline bool getprop_##Name() const                                                   \
    {                                                                                    \
        const PropertyInfo& Prop = getpropinfo_##Name();                                 \
        auto Value = *reinterpret_cast<uint8_t*>((uint8_t*)this + Prop.Offset);          \
        if (Prop.ByteMask)                                                               \
            return Value & Prop.ByteMask;                                                \
        return Value;                                                                    \
    }                                                                                    \
    __declspec(property(get = getprop_##Name, put = putprop_##Name)) bool Name

/* @brief UESDK_UPROPERTY macro for user-defined offsets */
#define UESDK_UPROPERTY_OFFSET(Type, Name, Offset)                                \
    inline void putprop_##Name(const Type& v)                                     \
    {                                                                             \
        *reinterpret_cast<Type*>((uint8_t*)this + Offset) = const_cast<Type&>(v); \
    }                                                                             \
    inline Type& getprop_##Name() const                                           \
    {                                                                             \
        return *reinterpret_cast<Type*>((uint8_t*)this + Offset);                 \
    }                                                                             \
    __declspec(property(get = getprop_##Name, put = putprop_##Name)) Type Name

//
//
//

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

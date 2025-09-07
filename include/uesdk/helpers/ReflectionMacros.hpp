#pragma once
#include <uesdk/core/UnrealObjects.hpp>
#include <uesdk/helpers/PropertyInfo.hpp>

#include <cstdint>

/* @brief Small fix for templated types */
#define UESDK_TYPE(...) __VA_ARGS__

// TODO: Add better support for blueprint classes
/* @brief Sets up StaticClass() and GetDefaultObj() functions. */
#define UESDK_UOBJECT(ClassNameStr, Type)                                                                                                     \
public:                                                                                                                                       \
    static SDK::UClass* StaticClass()                                                                                                         \
    {                                                                                                                                         \
        static SDK::UClass* Clss = nullptr;                                                                                                   \
        if (!Clss)                                                                                                                            \
            Clss = SDK::GObjects->FindObjectFast<SDK::UClass>(ClassNameStr, SDK::CASTCLASS_UClass);                                           \
        return Clss;                                                                                                                          \
    }                                                                                                                                         \
    static Type* GetDefaultObj()                                                                                                              \
    {                                                                                                                                         \
        static Type* Default = nullptr;                                                                                                       \
        if (!Default)                                                                                                                         \
            Default = static_cast<Type*>(StaticClass()->ClassDefaultObject);                                                                  \
        return Default;                                                                                                                       \
    }                                                                                                                                         \
                                                                                                                                              \
private:                                                                                                                                      \
    static void _CheckInheritance()                                                                                                           \
    {                                                                                                                                         \
        static_assert(std::is_base_of_v<SDK::UObject, Type>, "UESDK_UOBJECT can only be used on UObjects. Class must inherit from UObject."); \
    }                                                                                                                                         \
    static constexpr const char* g_ClassName = ClassNameStr

#define UESDK_STRUCT(StructNameStr, Type)                                                                                                                 \
private:                                                                                                                                                  \
    static void _CheckInheritance()                                                                                                                       \
    {                                                                                                                                                     \
        static_assert(!std::is_base_of_v<SDK::UObject, Type>, "UESDK_STRUCT can only be used on non-UObjects. Remove inheritance or use UESDK_UOBJECT."); \
    }                                                                                                                                                     \
    static_assert(std::is_class_v<Type>, "UESDK_STRUCT can only be used on classes and structs.");                                                        \
    static constexpr const char* g_ClassName = StructNameStr

/* @brief Internal implementation details of UESDK_UPROPERTY */
#define UESDK_UPROPERTY_IMPL(Type, Name)                                                                        \
public:                                                                                                         \
    static_assert(g_ClassName, "Use either UESDK_UOBJECT or UESDK_STRUCT at the start of class/struct");        \
                                                                                                                \
    static SDK::PropertyInfo& _GetPropInfo_##Name(bool SuppressFailure = true)                                  \
    {                                                                                                           \
        static SDK::PropertyInfo Prop = SDK::GObjects                                                           \
                                            ->FindObjectFast<SDK::UStruct>(g_ClassName, SDK::CASTCLASS_UStruct) \
                                            ->FindProperty(#Name);                                              \
        if (!SuppressFailure && !Prop.Found)                                                                    \
            throw std::runtime_error("Failed to get property!");                                                \
        return Prop;                                                                                            \
    }                                                                                                           \
    static const bool _IsPropValid_##Name()                                                                     \
    {                                                                                                           \
        const SDK::PropertyInfo& Prop = _GetPropInfo_##Name();                                                  \
        return Prop.Found;                                                                                      \
    }

/* @brief Standard macro for defining a UPROPERTY. Does not support bit-fields */
#define UESDK_UPROPERTY(Type, Name)                                            \
    UESDK_UPROPERTY_IMPL(Type, Name)                                           \
                                                                               \
    inline int& _PutProp_##Name(Type v)                                        \
    {                                                                          \
        const SDK::PropertyInfo& Prop = _GetPropInfo_##Name(false);            \
        *reinterpret_cast<Type*>((uint8_t*)this + Prop.Offset) = std::move(v); \
        return *reinterpret_cast<int*>((uint8_t*)this + Prop.Offset);          \
    }                                                                          \
                                                                               \
    inline Type& _GetProp_##Name() const                                       \
    {                                                                          \
        const SDK::PropertyInfo& Prop = _GetPropInfo_##Name(false);            \
        return *reinterpret_cast<Type*>((uint8_t*)this + Prop.Offset);         \
    }                                                                          \
                                                                               \
    __declspec(property(get = _GetProp_##Name, put = _PutProp_##Name)) Type Name

/* @brief Macro for UPROPERTIES that are bit-fields with fallback to handling as a bool */
#define UESDK_UPROPERTY_BITFIELD(Name)                                                   \
    UESDK_UPROPERTY_IMPL(Type, Name)                                                     \
                                                                                         \
    inline void _PutProp_##Name(bool v)                                                  \
    {                                                                                    \
        const SDK::PropertyInfo& Prop = _GetPropInfo_##Name(false);                      \
        if (Prop.ByteMask) {                                                             \
            auto& ByteValue = *reinterpret_cast<uint8_t*>((uint8_t*)this + Prop.Offset); \
            ByteValue &= ~Prop.ByteMask;                                                 \
            if (v)                                                                       \
                ByteValue |= Prop.ByteMask;                                              \
            return;                                                                      \
        }                                                                                \
        *reinterpret_cast<bool*>((uint8_t*)this + Prop.Offset) = std::move(v);           \
    }                                                                                    \
                                                                                         \
    inline bool _GetProp_##Name() const                                                  \
    {                                                                                    \
        const SDK::PropertyInfo& Prop = _GetPropInfo_##Name(false);                      \
        auto Value = *reinterpret_cast<uint8_t*>((uint8_t*)this + Prop.Offset);          \
        if (Prop.ByteMask)                                                               \
            return Value & Prop.ByteMask;                                                \
        return Value;                                                                    \
    }                                                                                    \
                                                                                         \
    __declspec(property(get = _GetProp_##Name, put = _PutProp_##Name)) bool Name

/* @brief UESDK_UPROPERTY macro for user-defined offsets */
#define UESDK_UPROPERTY_OFFSET(Type, Name, Offset)                        \
public:                                                                   \
    inline void _PutProp_##Name(Type v)                                   \
    {                                                                     \
        *reinterpret_cast<Type*>((uint8_t*)this + Offset) = std::move(v); \
    }                                                                     \
                                                                          \
    inline Type& _GetProp_##Name() const                                  \
    {                                                                     \
        return *reinterpret_cast<Type*>((uint8_t*)this + Offset);         \
    }                                                                     \
                                                                          \
    __declspec(property(get = _GetProp_##Name, put = _PutProp_##Name)) Type Name

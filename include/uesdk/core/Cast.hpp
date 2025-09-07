#pragma once
#include <uesdk/core/UnrealObjects.hpp>

#include <type_traits>

namespace SDK
{
    template <typename T, bool ForceCast = false>
        requires(std::is_base_of_v<UObject, T>)
    const T* Cast(const UObject* Obj)
    {
        if (!Obj)
            return nullptr;

        if (!ForceCast && !Obj->IsA(T::StaticClass()))
            return nullptr;

        return static_cast<const T*>(Obj);
    }

    template <typename T, bool ForceCast = false>
        requires(std::is_base_of_v<UObject, T>)
    T* Cast(UObject* Obj)
    {
        return const_cast<T*>(Cast<T, ForceCast>(static_cast<const UObject*>(Obj)));
    }
}

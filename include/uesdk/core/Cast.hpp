#pragma once
#include <uesdk/core/UnrealObjects.hpp>

#include <type_traits>

namespace SDK
{
    template <typename T, bool ForceCast = false, typename U>
        requires(std::is_base_of_v<UObject, T>&& std::is_base_of_v<UObject, std::remove_const_t<U>>)
    T* Cast(U* Obj)
    {
        if (!Obj)
            return nullptr;

        if (!ForceCast && !Obj->IsA(T::StaticClass()))
            return nullptr;

        return static_cast<T*>(Obj);
    }
}

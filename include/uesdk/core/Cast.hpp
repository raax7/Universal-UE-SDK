#pragma once
#include <uesdk/core/UnrealObjects.hpp>

#include <type_traits>

namespace SDK
{
    /**
     * @brief Safely casts a UObject pointer to another UObject type.
     * @tparam T - Target UObject type.
     * @tparam U - Source UObject type.
     * @tparam ForceCast - If true, skips type checking and performs an unconditional cast.
     * @param Obj - Pointer to the UObject to cast.
     * @return Pointer to T if cast is valid; nullptr otherwise.
     */
    template <typename T, bool ForceCast = false, typename U>
        requires(std::is_base_of_v<UObject, T> && std::is_base_of_v<UObject, std::remove_const_t<U>>)
    std::conditional_t<std::is_const_v<U>, const T, T>* Cast(U* Obj)
    {
        if (!Obj)
            return nullptr;

        if (!ForceCast && !Obj->IsA(T::StaticClass()))
            return nullptr;

        return static_cast<std::conditional_t<std::is_const_v<U>, const T, T>*>(Obj);
    }
}

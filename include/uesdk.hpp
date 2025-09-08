#pragma once
#include <uesdk/State.hpp>
#include <uesdk/Status.hpp>
#include <uesdk/core/Cast.hpp>
#include <uesdk/core/FMemory.hpp>
#include <uesdk/core/ObjectArray.hpp>
#include <uesdk/core/UnrealContainers.hpp>
#include <uesdk/core/UnrealEnums.hpp>
#include <uesdk/core/UnrealObjects.hpp>
#include <uesdk/helpers/FastSearch.hpp>
#include <uesdk/helpers/PECallWrapper.hpp>
#include <uesdk/helpers/ReflectionMacros.hpp>

namespace SDK
{
    typedef int8_t int8;
    typedef int16_t int16;
    typedef int32_t int32;
    typedef int64_t int64;

    typedef uint8_t uint8;
    typedef uint16_t uint16;
    typedef uint32_t uint32;
    typedef uint64_t uint64;

    /**
     * @brief Initiates the core SDK. Should be called before any other interaction with the library.
     * @return SDK::Status result. Should be compared with SDK::Status::Success.
     */
    ESDKStatus Init();
}

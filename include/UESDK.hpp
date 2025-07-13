#pragma once
#include <uesdk/FMemory.hpp>
#include <uesdk/FastSearch.hpp>
#include <uesdk/ObjectArray.hpp>
#include <uesdk/State.hpp>
#include <uesdk/UESDKStatus.hpp>
#include <uesdk/PECallWrapper.hpp>
#include <uesdk/UnrealContainers.hpp>
#include <uesdk/UnrealEnums.hpp>
#include <uesdk/UnrealObjects.hpp>

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
    Status Init();
}

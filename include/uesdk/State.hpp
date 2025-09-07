#pragma once

/** @brief This namespace is managed by the library. Do not write to any values. */
namespace SDK::State
{
    inline bool Setup = false;

    inline bool UsesChunkedGObjects = true;
    inline bool UsesFProperty = false;

    inline bool SetupFMemory = false;
    inline bool SetupGObjects = false;
    inline bool SetupAppendString = false;
    inline bool SetupFNameConstructorNarrow = false;
    inline bool SetupFNameConstructorWide = false;
    inline bool SetupMemberOffsets = false;
}

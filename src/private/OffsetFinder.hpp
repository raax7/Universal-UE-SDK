#pragma once
#include <uesdk/Status.hpp>

namespace SDK::OffsetFinder
{
    bool FindFMemoryRealloc();
    bool FindGObjects();
    bool FindFNameConstructorNarrow();
    bool FindFNameConstructorWide();
    bool FindAppendString();
    bool FindProcessEventIdx();

    ESDKStatus SetupMemberOffsets();
}

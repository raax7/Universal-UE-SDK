#include <UESDK.hpp>
#include <private/OffsetFinder.hpp>

namespace SDK
{
    ESDKStatus Init()
    {
        if (State::Setup)
            return ESDKStatus::Failed_AlreadySetup;

        if (!OffsetFinder::FindFMemoryRealloc())
            return ESDKStatus::Failed_FMemoryRealloc;

        if (!OffsetFinder::FindGObjects())
            return ESDKStatus::Failed_GObjects;

        if (!OffsetFinder::FindFNameConstructorNarrow())
            return ESDKStatus::Failed_NarrowFNameConstructor;

        if (!OffsetFinder::FindFNameConstructorWide())
            return ESDKStatus::Failed_WideFNameConstructor;

        if (!OffsetFinder::FindAppendString())
            return ESDKStatus::Failed_AppendString;

        if (const auto Status = OffsetFinder::SetupMemberOffsets(); Status != ESDKStatus::Success)
            return Status;

        if (!OffsetFinder::FindProcessEventIdx())
            return ESDKStatus::Failed_ProcessEvent;

        State::Setup = true;
        return ESDKStatus::Success;
    }
}

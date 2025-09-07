#include <uesdk/Offsets.hpp>
#include <uesdk/core/FMemory.hpp>

namespace SDK::FMemory
{
    using ReallocParams = void* (*)(void* Original, uint32_t Size, uint32_t Alignment);

    void Free(void* Original)
    {
        ReallocParams ReallocFunction = reinterpret_cast<ReallocParams>(Offsets::FMemory::Realloc);
        ReallocFunction(Original, 0, 0);
    }
    [[nodiscard]] void* Malloc(uint32_t Size, uint32_t Alignment)
    {
        ReallocParams ReallocFunction = reinterpret_cast<ReallocParams>(Offsets::FMemory::Realloc);
        return ReallocFunction(nullptr, Size, Alignment);
    }
    [[nodiscard]] void* Realloc(void* Original, uint32_t Size, uint32_t Alignment)
    {
        ReallocParams ReallocFunction = reinterpret_cast<ReallocParams>(Offsets::FMemory::Realloc);
        return ReallocFunction(Original, Size, Alignment);
    }
}

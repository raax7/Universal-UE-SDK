#pragma once
#include <cstdint>

namespace SDK::FMemory
{
    void Free(void* Original);
    [[nodiscard]] void* Malloc(uint32_t Size, uint32_t Alignment);
    [[nodiscard]] void* Realloc(void* Original, uint32_t Size, uint32_t Alignment);
}

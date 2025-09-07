#pragma once
#include <uesdk/helpers/PropertyInfo.hpp>

#include <libhat.hpp>

#include <Windows.h>
#include <functional>
#include <vector>

// Credit to https://github.com/Encryqed/Dumper-7 for the FindOffset and GetValidPointerOffset function.

namespace SDK::Memory
{
    uintptr_t CalculateRVA(uintptr_t Addr, uint32_t Offset);
    bool Is32BitRelativeAddress(uint8_t ModRM);

    bool IsInProcessRange(uintptr_t Addr);

    std::byte* IterateAll(hat::signature_view Signature, const std::string& Section, const std::function<bool(std::byte*)>& It);
    std::byte* FindPatternInRange(const std::byte* Start, const std::byte* End, hat::signature_view Signature);

    std::pair<const void*, int32_t> IterateVFT(void** VTable, const std::function<bool(std::byte* Addr)>& CallBackForEachFunc, int32_t NumFunctions = 0x150, int32_t OffsetFromStart = 0x0);

    template <typename T = const char*>
    inline std::byte* FindStringRef(T String)
    {
        // The encoding of our target LEA is:
        //
        // 48 - REX prefix (indicating 64-bit operand size)
        // 8D - LEA
        // ?? - ModR/M byte
        // ?? ?? ?? ?? - Address
        //
        // We will use Is32BitRelativeAddress to check if the ModR/M byte is correct.

        auto Validate = [String](std::byte* Address) -> bool {
            if (!Is32BitRelativeAddress(*(uint8_t*)(Address + 2)))
                return false;

            uintptr_t RVA = CalculateRVA(reinterpret_cast<uintptr_t>(Address), 3);
            if (!IsInProcessRange(RVA))
                return false;

            if constexpr (std::is_same_v<T, const char*>) {
                if (!strcmp((const char*)String, (const char*)RVA))
                    return true;
            }
            else if constexpr (std::is_same_v<T, const wchar_t*>) {
                if (!wcscmp((const wchar_t*)String, (const wchar_t*)RVA))
                    return true;
            }
            else {
                static_assert(std::is_same_v<T, const char*> || std::is_same_v<T, const wchar_t*>, "Unsupported string type");
            }

            return false;
        };

        std::byte* Result = IterateAll(hat::compile_signature<"4C 8D ? ? ? ? ?">(), ".text", Validate);
        if (!Result)
            Result = IterateAll(hat::compile_signature<"48 8D ? ? ? ? ?">(), ".text", Validate);

        return Result;
    }

    template <int Alignement = 4, typename T>
    inline int32_t FindOffset(std::vector<std::pair<void*, T>>& ObjectValuePair, int MinOffset = 0x28, int MaxOffset = 0x1A0)
    {
        int32_t HighestFoundOffset = MinOffset;

        for (int i = 0; i < ObjectValuePair.size(); i++) {
            uint8_t* BytePtr = (uint8_t*)(ObjectValuePair[i].first);

            for (int j = HighestFoundOffset; j < MaxOffset; j += Alignement) {
                if (*reinterpret_cast<T*>(BytePtr + j) == ObjectValuePair[i].second && j >= HighestFoundOffset) {
                    if (j > HighestFoundOffset) {
                        HighestFoundOffset = j;
                        i = 0;
                    }
                    j = MaxOffset;
                }
            }
        }

        return HighestFoundOffset != MinOffset ? HighestFoundOffset : OFFSET_NOT_FOUND;
    }

    template <bool bCheckForVft = true>
    inline int32_t GetValidPointerOffset(uint8_t* ObjA, uint8_t* ObjB, int32_t StartingOffset, int32_t MaxOffset)
    {
        auto IsBadReadPtr = [](void* p) -> bool {
            MEMORY_BASIC_INFORMATION mbi;

            if (VirtualQuery(p, &mbi, sizeof(mbi))) {
                constexpr DWORD mask = (PAGE_READONLY | PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY);
                bool b = !(mbi.Protect & mask);
                if (mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS))
                    b = true;

                return b;
            }

            return true;
        };

        if (IsBadReadPtr(ObjA) || IsBadReadPtr(ObjB))
            return OFFSET_NOT_FOUND;

        for (int j = StartingOffset; j <= MaxOffset; j += 0x8) {
            const bool bIsAValid = !IsBadReadPtr(*reinterpret_cast<void**>(ObjA + j)) && (bCheckForVft ? !IsBadReadPtr(**reinterpret_cast<void***>(ObjA + j)) : true);
            const bool bIsBValid = !IsBadReadPtr(*reinterpret_cast<void**>(ObjB + j)) && (bCheckForVft ? !IsBadReadPtr(**reinterpret_cast<void***>(ObjB + j)) : true);

            if (bIsAValid && bIsBValid)
                return j;
        }

        return OFFSET_NOT_FOUND;
    };
}

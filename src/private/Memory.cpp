#include <private/Memory.hpp>

namespace SDK::Memory
{
    uintptr_t CalculateRVA(uintptr_t Addr, uint32_t Offset)
    {
        return ((Addr + Offset + 4) + *(int32_t*)(Addr + Offset));
    }
    bool Is32BitRelativeAddress(uint8_t ModRM)
    {
        // MOD - Bit 6-7
        // REG - Bit 3-5
        // R/M - Bit 0-2
        //
        // MOD should be 0b00 and R/M should be 0b101.
        // REG varies depending on the output register
        // so we can't check it.

        return ((ModRM & 0b11000111) == 0b00000101);
    }

    bool IsInProcessRange(uintptr_t Addr)
    {
        // NtCurrentPeb() + 0x10 = ImageBaseAddress.

        PIMAGE_DOS_HEADER DosHeader = *(PIMAGE_DOS_HEADER*)(__readgsqword(0x60) + 0x10);
        PIMAGE_NT_HEADERS NtHeaders = (PIMAGE_NT_HEADERS)((uintptr_t)DosHeader + DosHeader->e_lfanew);

        const auto ImageBase = (uintptr_t)DosHeader;
        const auto ImageSize = NtHeaders->OptionalHeader.SizeOfImage;

        return Addr > ImageBase && Addr < (ImageBase + ImageSize);
    }

    std::byte* IterateAll(hat::signature_view Signature, const std::string& Section, const std::function<bool(std::byte*)>& It)
    {
        auto Module = hat::process::get_process_module();
        std::span<std::byte> Data = Module.get_section_data(Section);
        if (Data.empty())
            return nullptr;

        const std::byte* DataBegin = Data.data();
        const std::byte* DataEnd = DataBegin + Data.size();

        while (DataBegin < DataEnd) {
            auto Result = hat::find_pattern(DataBegin, DataEnd, Signature);
            if (!Result.has_result())
                break;

            std::byte* ResPtr = (std::byte*)&(*Result.get());
            if (It(ResPtr))
                return ResPtr;

            DataBegin = ResPtr + 1;
        }

        return nullptr;
    }

    std::byte* FindPatternInRange(const std::byte* Start, const std::byte* End, hat::signature_view Signature)
    {
        auto Result = hat::find_pattern(Start, End, Signature);
        return const_cast<std::byte*>(Result.get());
    }

    std::pair<const void*, int32_t> IterateVFT(void** VTable, const std::function<bool(std::byte* Addr)>& CallBackForEachFunc, int32_t NumFunctions, int32_t OffsetFromStart)
    {
        if (!CallBackForEachFunc)
            return { nullptr, OFFSET_NOT_FOUND };

        for (int i = 0; i < 0x250; i++) {
            std::byte* CurrentFuncAddress = reinterpret_cast<std::byte*>(VTable[i]);

            if (CurrentFuncAddress == nullptr || !IsInProcessRange(reinterpret_cast<uintptr_t>(CurrentFuncAddress)))
                break;

            if (CallBackForEachFunc(CurrentFuncAddress))
                return { CurrentFuncAddress, i };
        }

        return { nullptr, OFFSET_NOT_FOUND };
    }
}

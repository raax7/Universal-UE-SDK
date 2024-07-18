#pragma once
#include <vector>
#include <libhat.hpp>

#define RVA(addr, size)			((uint8_t*)(addr + *(uint32_t*)(addr + ((size) - 4)) + size))

namespace SDK::Memory
{
    std::byte* Find(hat::signature_view Signature) {
        if (const auto result = hat::find_pattern(Signature, ".rdata"); result.has_result())
            return (std::byte*)result.get();

        return nullptr;
    }
    std::vector<std::byte*> FindAll(hat::signature_view Signature) {
        std::vector<std::byte*> Results = {};

        uintptr_t Start = (uintptr_t)hat::process::get_process_module();
        std::span<std::byte> Data = hat::process::get_module_data(hat::process::module_t(Start));
        if (Data.empty())
            return Results;

        const std::byte* DataBegin = Data.data();
        const std::byte* DataEnd = DataBegin + Data.size();

        while (DataBegin < DataEnd)
        {
            auto Result = hat::find_pattern(DataBegin, DataEnd, Signature);
            if (!Result.has_result())
                break;

            std::byte* ResPtr = (std::byte*)&(*Result.get());
            Results.emplace_back(ResPtr);
            DataBegin = ResPtr + 1;
        }

        return Results;
    }

    template<typename T>
    std::byte* FindString(const T* String) {
        const auto StringSig = hat::string_to_signature(std::basic_string_view<T>(String));
        return Find(StringSig);
    }

    template<typename T>
    std::byte* FindStringRef(const T* String) {
        std::byte* StringAddr = FindString(String);

        auto ProcessPattern = [&StringAddr](const std::vector<std::byte*>& StringRefs) -> std::byte* {
            for (const auto& Found : StringRefs)
            {
                if ((std::byte*)RVA(Found, 7) == StringAddr)
                    return Found;
            }

            return nullptr;
            };

        const std::vector<std::vector<std::byte*>> LEAs = {
            FindAll(hat::compile_signature<"48 8D 05 ?? ?? ?? ??">()),
            FindAll(hat::compile_signature<"4C 8D 05 ?? ?? ?? ??">())
        };

        for (const auto& Search : LEAs)
        {
            if (const auto Result = ProcessPattern(Search); Result)
                return Result;
        }

        return nullptr;
    }
}
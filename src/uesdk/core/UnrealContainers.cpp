#include <uesdk/core/UnrealContainers.hpp>

#include <private/Offsets.hpp>

#include <string>

namespace SDK
{
    FName::FName(const char* Str)
        : ComparisonIndex(0)
        , Number(0)
    {
        static void (*ConstructorNarrow)(const FName*, const char*, bool) = nullptr;

        if (!ConstructorNarrow)
            ConstructorNarrow = reinterpret_cast<void (*)(const FName*, const char*, bool)>(Offsets::FName::ConstructorNarrow);

        ConstructorNarrow(const_cast<FName*>(this), Str, true);
    }
    FName::FName(const wchar_t* Str)
        : ComparisonIndex(0)
        , Number(0)
    {
        static void (*ConstructorWide)(const FName*, const wchar_t*, bool) = nullptr;

        if (!ConstructorWide)
            ConstructorWide = reinterpret_cast<void (*)(const FName*, const wchar_t*, bool)>(Offsets::FName::ConstructorWide);

        ConstructorWide(const_cast<FName*>(this), Str, true);
    }

    std::string FName::GetRawString() const
    {
        static void (*AppendString)(const FName*, FString*) = nullptr;

        if (!AppendString)
            AppendString = reinterpret_cast<void (*)(const FName*, FString*)>(Offsets::FName::AppendString);

        FString TempString;
        AppendString(const_cast<FName*>(this), &TempString);

        return TempString.ToString();
    }
    std::string FName::ToString() const
    {
        std::string OutputString = GetRawString();
        size_t pos = OutputString.rfind('/');

        if (pos == std::string::npos)
            return OutputString;

        return OutputString.substr(pos + 1);
    }
}

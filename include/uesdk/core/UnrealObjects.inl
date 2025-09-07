#pragma once
#include <uesdk/State.hpp>
#include <uesdk/core/UnrealObjects.hpp>
#include <uesdk/helpers/FastSearch.hpp>

#include <algorithm>
#include <array>
#include <format>

namespace SDK
{
    template <StringLiteral ClassName, StringLiteral MemberName, typename MemberType>
    MemberType UObject::GetMember()
    {
        static int32_t Offset = OFFSET_NOT_FOUND;
        if (Offset == OFFSET_NOT_FOUND && !FastSearchSingle(FSProperty(ClassName.c_str(), MemberName.c_str(), &Offset))) {
            throw std::runtime_error(std::format("Failed to find member offset! ({}, {})", ClassName.c_str(), MemberName.c_str()));
        }
        return *reinterpret_cast<MemberType*>((uint8_t*)this + Offset);
    }

    template <StringLiteral ClassName, StringLiteral MemberName, typename MemberType>
    MemberType* UObject::GetMemberPtr()
    {
        static int32_t Offset = OFFSET_NOT_FOUND;
        if (Offset == OFFSET_NOT_FOUND && !FastSearchSingle(FSProperty(ClassName.c_str(), MemberName.c_str(), &Offset))) {
            throw std::runtime_error(std::format("Failed to find member offset! ({}, {})", ClassName.c_str(), MemberName.c_str()));
        }
        return reinterpret_cast<MemberType*>((uint8_t*)this + Offset);
    }

    template <StringLiteral ClassName, StringLiteral MemberName, typename MemberType>
    void UObject::SetMember(MemberType Value)
    {
        static int32_t Offset = OFFSET_NOT_FOUND;
        if (Offset == OFFSET_NOT_FOUND && !FastSearchSingle(FSProperty(ClassName.c_str(), MemberName.c_str(), &Offset))) {
            throw std::runtime_error(std::format("Failed to find member offset! ({}, {})", ClassName.c_str(), MemberName.c_str()));
        }
        *reinterpret_cast<MemberType*>((uint8_t*)this + Offset) = Value;
    }
}

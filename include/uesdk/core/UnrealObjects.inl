#pragma once
#include <uesdk/FastSearch.hpp>
#include <uesdk/State.hpp>
#include <uesdk/UnrealObjects.hpp>

#include <algorithm>
#include <array>
#include <format>

namespace SDK
{
    template <StringLiteral ClassName, StringLiteral MemberName, typename MemberType>
    MemberType UObject::GetMember()
    {
        static int32_t Offset = OFFSET_NOT_FOUND;
        if (Offset == OFFSET_NOT_FOUND && !FastSearchSingle(FSProperty(ClassName.c_str(), MemberName.c_str(), &Offset, nullptr))) {
            throw std::runtime_error("Failed to find member offset!");
        }
        return *(MemberType*)((uintptr_t)this + Offset);
    }

    template <StringLiteral ClassName, StringLiteral MemberName, typename MemberType>
    MemberType* UObject::GetMemberPtr()
    {
        static int32_t Offset = OFFSET_NOT_FOUND;
        if (Offset == OFFSET_NOT_FOUND && !FastSearchSingle(FSProperty(ClassName.c_str(), MemberName.c_str(), &Offset, nullptr))) {
            throw std::runtime_error("Failed to find member offset!");
        }
        return (MemberType*)((uintptr_t)this + Offset);
    }

    template <StringLiteral ClassName, StringLiteral MemberName, typename MemberType>
    void UObject::SetMember(MemberType Value)
    {
        static int32_t Offset = OFFSET_NOT_FOUND;
        if (Offset == OFFSET_NOT_FOUND && !FastSearchSingle(FSProperty(ClassName.c_str(), MemberName.c_str(), &Offset, nullptr))) {
            throw std::runtime_error("Failed to find member offset!");
        }
        *(MemberType*)((uintptr_t)this + Offset) = Value;
    }
}

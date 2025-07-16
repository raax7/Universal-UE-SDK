#include <uesdk/FMemory.hpp>
#include <uesdk/FastSearch.hpp>
#include <uesdk/ObjectArray.hpp>
#include <uesdk/Offsets.hpp>
#include <uesdk/State.hpp>
#include <uesdk/UESDKStatus.hpp>

#include <private/Memory.hpp>

#include <libhat.hpp>

#include <format>
#include <memory>
#include <sstream>

// Most of the member offset finding is based off of https://github.com/Encryqed/Dumper-7, so credits to them.

#define GET_OFFSET(Function, Variable, ErrorCode) \
    Variable = Function();                        \
    if (Variable == OFFSET_NOT_FOUND)             \
        return ErrorCode;

// Variables.
//
namespace SDK::OffsetFinder
{
    UObject* PlayerController = nullptr;
    UObject* Controller = nullptr;

    UObject* Vector = nullptr;
    UObject* Vector4 = nullptr;
    UObject* Vector2D = nullptr;
    UStruct* Guid = nullptr;
    UObject* Transform = nullptr;

    UObject* KismetSystemLibrary = nullptr;
    UObject* KismetStringLibrary = nullptr;

    UObject* Struct = nullptr;
    UObject* Field = nullptr;
    UObject* Class = nullptr;

    UObject* Actor = nullptr;
    UObject* Object = nullptr;

    UObject* Default__Object = nullptr;
    UObject* Default__Field = nullptr;

    UStruct* Color = nullptr;

    UClass* Engine = nullptr;

    UObject* ENetRole = nullptr;
    UObject* ETraceTypeQuery = nullptr;

    UFunction* WasInputKeyJustPressed = nullptr;
    UFunction* ToggleSpeaking = nullptr;
    UFunction* SwitchLevel = nullptr;

    UFunction* SetViewTargetWithBlend = nullptr;
    UFunction* SetHapticsByValue = nullptr;
    UFunction* SphereTraceSingleForObjects = nullptr;

    UFunction* WasRecentlyRendered = nullptr;
    UFunction* CrossProduct2D = nullptr;
    UFunction* GetSpectatorPawn = nullptr;
}

// Helper functions.
//
namespace SDK::OffsetFinder
{
    std::string FormatUint32(uint32_t Value)
    {
        std::ostringstream oss;

        for (int i = 0; i < 4; ++i) {
            uint8_t Byte = (Value >> (i * 8)) & 0xFF;
            oss << std::format("{:02X}", Byte);

            if (i < 3) {
                oss << " ";
            }
        }

        return oss.str();
    }
}

// Class member offset finders.
//
namespace SDK::OffsetFinder
{
    int32_t Find_UField_Next()
    {
        uint8_t* KismetSystemLibraryChild = reinterpret_cast<uint8_t*>(((UStruct*)(KismetSystemLibrary))->Children);
        uint8_t* KismetStringLibraryChild = reinterpret_cast<uint8_t*>(((UStruct*)(KismetStringLibrary))->Children);

        return Memory::GetValidPointerOffset(KismetSystemLibraryChild, KismetStringLibraryChild, Offsets::UObject::Outer + 0x8, 0x48);
    }

    int32_t Find_UStruct_SuperStruct()
    {
        std::vector<std::pair<void*, void*>> ValuePair = {
            { Struct, Field },
            { Class, Struct }
        };

        return Memory::FindOffset(ValuePair);
    }
    int32_t Find_UStruct_Children()
    {
        std::vector<std::pair<void*, void*>> ValuePair = {
            { PlayerController, GObjects->FindObjectFastInOuter("WasInputKeyJustReleased", "PlayerController") },
            { Controller, GObjects->FindObjectFastInOuter("UnPossess", "Controller") }
        };

        if (Memory::FindOffset(ValuePair) == OFFSET_NOT_FOUND) {
            ValuePair = {
                { Vector, GObjects->FindObjectFastInOuter("X", "Vector") },
                { Vector4, GObjects->FindObjectFastInOuter("X", "Vector4") },
                { Vector2D, GObjects->FindObjectFastInOuter("X", "Vector2D") },
                { Guid, GObjects->FindObjectFastInOuter("A", "Guid") }
            };

            return Memory::FindOffset(ValuePair);
        }

        State::UsesFProperty = true;
        return Memory::FindOffset(ValuePair);
    }
    int32_t Find_UStruct_ChildProperties()
    {
        return Memory::GetValidPointerOffset<true>(reinterpret_cast<uint8_t*>(Color), reinterpret_cast<uint8_t*>(Guid), Offsets::UStruct::Children + 0x8, Offsets::UStruct::Children + 0x60);
    }
    int32_t Find_UStruct_PropertiesSize()
    {
        std::vector<std::pair<void*, int32_t>> ValuePair = {
            { Color, 0x4 },
            { Guid, 0x10 }
        };

        return Memory::FindOffset(ValuePair);
    }
    int32_t Find_UStruct_MinAlignment()
    {
        std::vector<std::pair<void*, int32_t>> ValuePair = {
            { Transform, 0x10 },
            { PlayerController, 0x8 }
        };

        return Memory::FindOffset(ValuePair);
    }

    int32_t Find_UClass_ClassCastFlags()
    {
        std::vector<std::pair<void*, EClassCastFlags>> ValuePair = {
            { Actor, CASTCLASS_AActor },
            { Class, CASTCLASS_UField | CASTCLASS_UStruct | CASTCLASS_UClass }
        };

        return Memory::FindOffset(ValuePair);
    }
    int32_t Find_UClass_ClassDefaultObject()
    {
        std::vector<std::pair<void*, void*>> ValuePair = {
            { Object, Default__Object },
            { Field, Default__Field }
        };

        return Memory::FindOffset(ValuePair);
    }

    int32_t Find_UProperty_Offset()
    {
        std::vector<std::pair<void*, int32_t>> ValuePair = {
            { Color->FindMember(FName("B")), 0x00 },
            { Color->FindMember(FName("G")), 0x01 },
            { Color->FindMember(FName("R")), 0x02 }
        };

        return Memory::FindOffset(ValuePair);
    }
    int32_t Find_UProperty_ElementSize()
    {
        std::vector<std::pair<void*, int32_t>> ValuePair = {
            { Guid->FindMember(FName("A")), 0x04 },
            { Guid->FindMember(FName("B")), 0x04 },
            { Guid->FindMember(FName("C")), 0x04 }
        };

        return Memory::FindOffset(ValuePair);
    }
    int32_t Find_UProperty_PropertyFlags()
    {
        constexpr EPropertyFlags GuidMemberFlags = CPF_Edit | CPF_ZeroConstructor | CPF_SaveGame | CPF_IsPlainOldData | CPF_NoDestructor | CPF_HasGetValueTypeHash;
        constexpr EPropertyFlags ColorMemberFlags = CPF_Edit | CPF_BlueprintVisible | CPF_ZeroConstructor | CPF_SaveGame | CPF_IsPlainOldData | CPF_NoDestructor | CPF_HasGetValueTypeHash;

        std::vector<std::pair<void*, EPropertyFlags>> ValuePair = {
            { Guid->FindMember(FName("A")), GuidMemberFlags },
            { Color->FindMember(FName("R")), ColorMemberFlags }
        };

        int FlagsOffset = Memory::FindOffset(ValuePair);

        // Same flags without AccessSpecifier.
        if (FlagsOffset == OFFSET_NOT_FOUND) {
            ValuePair[0].second |= CPF_NativeAccessSpecifierPublic;
            ValuePair[1].second |= CPF_NativeAccessSpecifierPublic;

            FlagsOffset = Memory::FindOffset(ValuePair);
        }

        return FlagsOffset;
    }

    int32_t Find_UBoolProperty_Base()
    {
        std::vector<std::pair<void*, uint8_t>> ValuePair = {
            { Engine->FindMember(FName("bIsOverridingSelectedColor")), 0xFF },
            { Engine->FindMember(FName("bEnableOnScreenDebugMessagesDisplay")), 0b00000010 },
            { GObjects->FindClassFast("PlayerController")->FindMember(FName("bAutoManageActiveCameraTarget")), 0xFF }
        };

        return Memory::FindOffset<1>(ValuePair, Offsets::UProperty::Offset) - 0x3;
    }

    int32_t Find_UEnum_Names()
    {
        std::vector<std::pair<void*, int32_t>> ValuePair {
            { ENetRole, 0x5 },
            { ETraceTypeQuery, 0x22 }
        };

        return Memory::FindOffset(ValuePair) - 0x8;
    }

    int32_t Find_UFunction_FunctionFlags()
    {
        std::vector<std::pair<void*, EFunctionFlags>> ValuePair = {
            { WasInputKeyJustPressed, FUNC_Final | FUNC_Native | FUNC_Public | FUNC_BlueprintCallable | FUNC_BlueprintPure | FUNC_Const },
            { ToggleSpeaking, FUNC_Exec | FUNC_Native | FUNC_Public },
            { SwitchLevel, FUNC_Exec | FUNC_Native | FUNC_Public }
        };

        int32_t Ret = Memory::FindOffset(ValuePair);

        if (Ret == OFFSET_NOT_FOUND)
            for (auto& [_, Flags] : ValuePair)
                Flags = Flags | FUNC_RequiredAPI;

        return Memory::FindOffset(ValuePair);
    }
    int32_t Find_UFunction_NumParms()
    {
        std::vector<std::pair<void*, uint8_t>> ValuePair = {
            { SwitchLevel, 0x1 },
            { SetViewTargetWithBlend, 0x5 },
            { SetHapticsByValue, 0x3 },
            { SphereTraceSingleForObjects, 0xE }
        };

        return Memory::FindOffset<1>(ValuePair);
    }
    int32_t Find_UFunction_ParmsSize()
    {
        std::vector<std::pair<void*, uint16_t>> ValuePair = {
            { SwitchLevel, 0x10 },
            { SetViewTargetWithBlend, 0x15 },
            { SetHapticsByValue, 0x9 },
            { SphereTraceSingleForObjects, 0x109 }
        };

        return Memory::FindOffset<1>(ValuePair);
    }
    int32_t Find_UFunction_ReturnValueOffset()
    {
        std::vector<std::pair<void*, uint16_t>> ValuePair = {
            { WasRecentlyRendered, 0x4 },
            { CrossProduct2D, 0x10 },
            { GetSpectatorPawn, 0x0 },
            { SphereTraceSingleForObjects, 0x108 }
        };

        return Memory::FindOffset<1>(ValuePair);
    }
    int32_t Find_UFunction_Func()
    {
        for (int i = 0x40; i < 0x140; i += 8) {
            if (Memory::IsInProcessRange(*reinterpret_cast<uintptr_t*>((uintptr_t)WasInputKeyJustPressed + i)) && Memory::IsInProcessRange(*reinterpret_cast<uintptr_t*>((uintptr_t)ToggleSpeaking + i)) && Memory::IsInProcessRange(*reinterpret_cast<uintptr_t*>((uintptr_t)SwitchLevel + i))) {
                return i;
            }
        }

        return OFFSET_NOT_FOUND;
    }
}

// General offset finder functions.
//
namespace SDK::OffsetFinder
{
    bool FindFMemoryRealloc()
    {
        // This signature has been very reliable from what I have tested.
        // Since Realloc can be used as Alloc and Free aswell, it is easier to only get Realloc.
        /*
        48 89 5C 24 08	mov     [rsp+arg_0], rbx
        48 89 74 24 10	mov     [rsp+arg_8], rsi
        57				push    rdi
        48 83 EC 20		sub     rsp, 20h
        48 8B F1		mov     rsi, rcx
        41 8B D8		mov     ebx, r8d
        48 8B 0D 64		mov     rcx, cs:qword_6024FE0
        5A 10 04
        48 8B FA		mov     rdi, rdx
        48 85 C9		test    rcx, rcx
        */
        constexpr hat::fixed_signature FMemorySignature = hat::compile_signature<"48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 48 8B F1 41 8B D8 48 8B 0D ? ? ? ? 48 8B FA 48 85 C9">();

        const auto& Result = hat::find_pattern(FMemorySignature, ".text");
        Offsets::FMemory::Realloc = reinterpret_cast<uintptr_t>(Result.get());

        if (Result.has_result())
            State::SetupFMemory = true;

        return Result.has_result();
    }
    bool FindGObjects()
    {
        State::UsesChunkedGObjects = true;

        constexpr hat::fixed_signature ChunkedGObjectsSignature = hat::compile_signature<"48 8B 05 ? ? ? ? 48 8B 0C C8 48 8D 04 D1">();
        constexpr hat::fixed_signature FixedGObjectsSignature = hat::compile_signature<"48 8B 05 ? ? ? ? 48 8D 14 C8 EB 02">();

        const auto& ChunkedGObjects = hat::find_pattern(ChunkedGObjectsSignature, ".text");
        if (ChunkedGObjects.has_result()) {
            State::UsesChunkedGObjects = true;
            GObjects = std::make_unique<TUObjectArray>(State::UsesChunkedGObjects, (void*)Memory::CalculateRVA((uintptr_t)ChunkedGObjects.get(), 3));
            State::SetupGObjects = true;
            return true;
        }

        const auto& FixedGObjects = hat::find_pattern(FixedGObjectsSignature, ".text");
        if (FixedGObjects.has_result()) {
            State::UsesChunkedGObjects = false;
            GObjects = std::make_unique<TUObjectArray>(State::UsesChunkedGObjects, (void*)Memory::CalculateRVA((uintptr_t)FixedGObjects.get(), 3));
            State::SetupGObjects = true;
            return true;
        }

        return false;
    }
    bool FindFNameConstructorNarrow()
    {
        /*
        41 B8 01 00 00 00       mov     r8d, 1
        48 8D 15 0B BC CE 04    lea     rdx, aGetoutofbounds_0 ; "GetOutOfBoundsWarning"
        48 8D 0D 14 FE F4 05    lea     rcx, qword_6215D18
        E9 27 70 D3 01          jmp     FNameConstructorNarrow
        */

        std::byte* Start = Memory::FindStringRef("GetOutOfBoundsWarning");
        std::byte* End = Start + 0x60;

        if (!Start)
            return false;

        const auto& PatternResult = hat::find_pattern(Start, End, hat::compile_signature<"48 8D ? ? ? ? ? E9">());
        if (!PatternResult.has_result())
            return false;

        Offsets::FName::ConstructorNarrow = Memory::CalculateRVA((uintptr_t)PatternResult.get(), 8);
        if (!Memory::IsInProcessRange(Offsets::FName::ConstructorNarrow))
            return false;

        State::SetupFNameConstructorNarrow = true;
        return true;
    }
    bool FindFNameConstructorWide()
    {
        /*
        41 B8 01 00 00 00       mov     r8d, 1
        48 8D 15 29 CB 04 02    lea     rdx, aCanvasobject ; "CanvasObject"
        48 8D 0D 0A 28 2C 03    lea     rcx, qword_6210D40
        E8 75 EA 0A FF          call    FNameConstructor
        */

        std::byte* Start = Memory::FindStringRef(L"CanvasObject");
        std::byte* End = Start + 0x60;

        if (!Start)
            return false;

        const auto& PatternResult = hat::find_pattern(Start, End, hat::compile_signature<"48 8D ? ? ? ? ? E8">());
        if (!PatternResult.has_result())
            return false;

        Offsets::FName::ConstructorWide = Memory::CalculateRVA((uintptr_t)PatternResult.get(), 8);
        if (!Memory::IsInProcessRange(Offsets::FName::ConstructorWide))
            return false;

        State::SetupFNameConstructorWide = true;
        return true;
    }
    bool FindAppendString()
    {
        /*
        4C 8D 05 65 17 79 02	lea     r8, aForwardshading ; "ForwardShadingQuality_"
        41 B9 17 00 00 00		mov     r9d, 17h
        B8 3F 00 00 00			mov     eax, 3Fh ; '?'
        41 8B D1				mov     edx, r9d
        66 89 44 24 20			mov     word ptr [rsp+0A0h+var_80], ax
        E8 AD AF EE FD			call    sub_36C900
        48 8D 55 17				lea     rdx, [rbp+57h+var_40]
        48 8D 4D 6F				lea     rcx, [rbp+57h+arg_8]
        E8 40 DE B7 FF			call    AppendString
        */

        std::byte* Start = Memory::FindStringRef("ForwardShadingQuality_");
        std::byte* End = Start + 0x60;

        if (!Start)
            return false;

        const std::vector<std::pair<hat::fixed_signature<10>, int>> Signatures = {
            { hat::compile_signature<"48 8D ? ? 48 8D ? ? E8 ?">(), 9 },
            { hat::compile_signature<"48 8D ? ? ? 48 8D ? ? E8">(), 14 },
            { hat::compile_signature<"48 8D ? ? 49 8B ? E8 ? ?">(), 12 },
            { hat::compile_signature<"48 8D ? ? ? 49 8B ? E8 ?">(), 13 }
        };

        for (const auto& Sig : Signatures) {
            std::byte* Result = Memory::FindPatternInRange(Start, End, Sig.first);
            if (Result) {
                Offsets::FName::AppendString = Memory::CalculateRVA((uintptr_t)Result, Sig.second);
                if (!Memory::IsInProcessRange(Offsets::FName::AppendString))
                    return false;

                State::SetupAppendString = true;
                return true;
            }
        }

        return false;
    }
    bool FindProcessEventIdx()
    {
        auto Sig1 = hat::parse_signature(std::format("F7 ? {:02X} 00 00 00 00 04 00 00", Offsets::UFunction::FunctionFlags));
        auto Sig2 = hat::parse_signature(std::format("F7 ? {:02X} 00 00 00 00 00 40 00", Offsets::UFunction::FunctionFlags));

        auto IsProcessEvent = [Sig1, Sig2](std::byte* FuncAddress) -> bool {
            return Memory::FindPatternInRange(FuncAddress, FuncAddress + 0x400, Sig1.value())
                && Memory::FindPatternInRange(FuncAddress, FuncAddress + 0xF00, Sig2.value());
        };

        void** VFT = GObjects->GetByIndex(0)->VFT;
        auto [FuncPtr, FuncIdx] = Memory::IterateVFT(VFT, IsProcessEvent);

        Offsets::UObject::ProcessEventIdx = FuncIdx;
        return FuncIdx != OFFSET_NOT_FOUND;
    }

    Status SetupMemberOffsets()
    {
        std::vector<FSEntry> Search = {
            { FSUObject("PlayerController", &PlayerController) },
            { FSUObject("Controller", &Controller) },

            { FSUObject("Vector", &Vector) },
            { FSUObject("Vector4", &Vector4) },
            { FSUObject("Vector2D", &Vector2D) },
            { FSUObject("Guid", &Guid) },
            { FSUObject("Transform", &Transform) },

            { FSUObject("KismetSystemLibrary", &KismetSystemLibrary) },
            { FSUObject("KismetStringLibrary", &KismetStringLibrary) },

            { FSUObject("Struct", &Struct) },
            { FSUObject("Field", &Field) },
            { FSUObject("Class", &Class) },

            { FSUObject("Actor", &Actor) },
            { FSUObject("Object", &Object) },

            { FSUObject("Default__Object", &Default__Object) },
            { FSUObject("Default__Field", &Default__Field) },

            { FSUObject("Color", &Color) },
            { FSUObject("Engine", &Engine) },

            { FSUObject("ENetRole", &ENetRole) },
            { FSUObject("ETraceTypeQuery", &ETraceTypeQuery) }
        };
        if (!FastSearch(Search))
            return Status::Failed_FastSearchPass1;

        // The order of the functions below is very important as there is a dependency chain.
        // Do not change the order.

        GET_OFFSET(Find_UStruct_Children, Offsets::UStruct::Children, Status::Failed_UStruct_Children);
        GET_OFFSET(Find_UField_Next, Offsets::UField::Next, Status::Failed_UField_Next);
        GET_OFFSET(Find_UStruct_SuperStruct, Offsets::UStruct::SuperStruct, Status::Failed_UStruct_SuperStruct);
        GET_OFFSET(Find_UStruct_PropertiesSize, Offsets::UStruct::PropertiesSize, Status::Failed_UStruct_PropertiesSize);
        GET_OFFSET(Find_UStruct_MinAlignment, Offsets::UStruct::MinAlignment, Status::Failed_UStrct_MinAlignment);
        GET_OFFSET(Find_UClass_ClassCastFlags, Offsets::UClass::ClassCastFlags, Status::Failed_UClass_ClassCastFlags);

        // In order to do the second pass, we required UClass::CastFlags.

        Search = {
            { FSUFunction("PlayerController", "WasInputKeyJustPressed", &WasInputKeyJustPressed) },
            { FSUFunction("PlayerController", "ToggleSpeaking", &ToggleSpeaking) },
            { FSUFunction("PlayerController", "SwitchLevel", &SwitchLevel) },
            { FSUFunction("PlayerController", "SetViewTargetWithBlend", &SetViewTargetWithBlend) },
            { FSUFunction("PlayerController", "SetHapticsByValue", &SetHapticsByValue) },
            { FSUFunction("PlayerController", "GetSpectatorPawn", &GetSpectatorPawn) },
            { FSUFunction("KismetSystemLibrary", "SphereTraceSingleForObjects", &SphereTraceSingleForObjects) },
            { FSUFunction("KismetMathLibrary", "CrossProduct2D", &CrossProduct2D) },
            { FSUFunction("Actor", "WasRecentlyRendered", &WasRecentlyRendered) },
        };
        if (!FastSearch(Search))
            return Status::Failed_FastSearchPass2;

        GET_OFFSET(Find_UEnum_Names, Offsets::UEnum::Names, Status::Failed_UEnum_Names);

        GET_OFFSET(Find_UFunction_FunctionFlags, Offsets::UFunction::FunctionFlags, Status::Failed_UFunction_FunctionFlags);
        GET_OFFSET(Find_UFunction_NumParms, Offsets::UFunction::NumParms, Status::Failed_UFunction_NumParms);
        GET_OFFSET(Find_UFunction_ParmsSize, Offsets::UFunction::ParmsSize, Status::Failed_UFunction_ParmsSize);
        GET_OFFSET(Find_UFunction_ReturnValueOffset, Offsets::UFunction::ReturnValueOffset, Status::Failed_UFunction_ReturnValueOffset);
        GET_OFFSET(Find_UFunction_Func, Offsets::UFunction::Func, Status::Failed_UFunction_FuncOffset);

        if (State::UsesFProperty) {
            GET_OFFSET(Find_UStruct_ChildProperties, Offsets::UStruct::ChildProperties, Status::Failed_UStruct_ChildProperties);
        }
        else {
            GET_OFFSET(Find_UProperty_Offset, Offsets::UProperty::Offset, Status::Failed_UProperty_Offset);
            GET_OFFSET(Find_UProperty_ElementSize, Offsets::UProperty::ElementSize, Status::Failed_UProperty_ElementSize);
            GET_OFFSET(Find_UProperty_PropertyFlags, Offsets::UProperty::PropertyFlags, Status::Failed_UProperty_PropertyFlags);
            GET_OFFSET(Find_UBoolProperty_Base, Offsets::UBoolProperty::Base, Status::Failed_UBoolProperty_Base);
        }

        GET_OFFSET(Find_UClass_ClassDefaultObject, Offsets::UClass::ClassDefaultObject, Status::Failed_UClass_ClassDefaultObject);

        Search = {
            { FSProperty("DataTable", "RowStruct", &Offsets::UDataTable::RowStruct, nullptr) }
        };
        if (!FastSearch(Search))
            return Status::Failed_FastSearchPass3;

        // In every version I've tested, this is correct.
        Offsets::UDataTable::RowMap = Offsets::UDataTable::RowStruct + 0x8;

        State::SetupMemberOffsets = true;
        return Status::Success;
    }
}

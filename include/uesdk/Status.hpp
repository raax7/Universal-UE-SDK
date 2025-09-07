#pragma once

namespace SDK
{
    enum class Status
    {
        Success = 0,

        // Function offsets and VFT indexes.
        //
        Failed_FMemoryRealloc,
        Failed_GObjects,
        Failed_NarrowFNameConstructor,
        Failed_WideFNameConstructor,
        Failed_AppendString,
        Failed_ProcessEvent,

        // Class member offsets.
        //
        Failed_UField_Next,
        Failed_UStruct_SuperStruct,
        Failed_UStruct_Children,
        Failed_UStruct_ChildProperties,
        Failed_UStruct_PropertiesSize,
        Failed_UStrct_MinAlignment,
        Failed_UClass_ClassCastFlags,
        Failed_UClass_ClassDefaultObject,
        Failed_UEnum_Names,
        Failed_UFunction_FunctionFlags,
        Failed_UFunction_NumParms,
        Failed_UFunction_ParmsSize,
        Failed_UFunction_ReturnValueOffset,
        Failed_UFunction_FuncOffset,
        Failed_UProperty_Offset,
        Failed_UProperty_ElementSize,
        Failed_UProperty_PropertyFlags,
        Failed_UBoolProperty_Base,

        // FastSearch passes.
        //
        Failed_FastSearchPass1,
        Failed_FastSearchPass2,
        Failed_FastSearchPass3,
    };
}

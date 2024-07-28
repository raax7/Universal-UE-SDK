#pragma once

namespace SDK
{
    enum SDKStatus
    {
        SDK_SUCCESS = 0,

        SDK_FAILED_FMEMORY_REALLOC,
        SDK_FAILED_GOBJECTS,
        SDK_FAILED_FNAMECONSTRUCTOR,
        SDK_FAILED_APPENDSTRING,
        SDK_FAILED_PROCESSEVENTIDX,

        SDK_FAILED_UFIELD_NEXT,

        SDK_FAILED_USRTUCT_SUPER,

        SDK_FAILED_UCLASS_CHILDREN,
        SDK_FAILED_UCLASS_CASTFLAGS,
        SDK_FAILED_UCLASS_DEFAULTOBJECT,

        SDK_FAILED_UFUNCTION_FUNCTIONFLAGS,
        SDK_FAILED_UFUNCTION_NUMPARMS,
        SDK_FAILED_UFUNCTION_PARMSSIZE,
        SDK_FAILED_UFUNCTION_RETURNVALUEOFFSET,
        SDK_FAILED_UFUNCTION_FUNCOFFSET,

        SDK_FAILED_UPROPERTY_OFFSET,
        SDK_FAILED_UPROPERTY_ELEMENTSIZE,
        SDK_FAILED_UPROPERTY_PROPERTYFLAGS,

        SDK_FAILED_UBOOLPROPERTY_BASE,

        SDK_FAILED_FASTSEARCH_PASS1,
        SDK_FAILED_FASTSEARCH_PASS2,
    };
}
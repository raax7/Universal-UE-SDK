#pragma once
#include <uesdk/UnrealEnums.hpp>
#include <uesdk/UnrealTypes.hpp>
#include <vector>

namespace SDK
{
    /**
     * @brief Used to find a UObject matching the specified EClassCastFlags.
     *
     * @param[in] ObjectName - Name of the UObject to find.
     * @param[in] (optional) RequiredType - Required EClassCastFlags for the target UObject.
     * @param[in,out] OutObject -  Pointer to the output T*, no value is written if unfound.
     */
    struct FSUObject
    {
        FName ObjectName;
        EClassCastFlags RequiredType;
        class UObject** OutObject;

        template <typename T>
        explicit FSUObject(const std::string& ObjectName, uint64_t RequiredType, T** OutObject)
            : ObjectName(FName(ObjectName))
            , RequiredType((EClassCastFlags)(RequiredType))
            , OutObject(reinterpret_cast<class UObject**>(OutObject))
        {
        }
        template <typename T>
        explicit FSUObject(const std::string& ObjectName, T** OutObject)
            : ObjectName(FName(ObjectName))
            , RequiredType(CASTCLASS_None)
            , OutObject(reinterpret_cast<class UObject**>(OutObject))
        {
        }
    };

    /**
     * @brief Used to find a UFunction member of a UClass.
     * @brief This does not search inhereted classes, so make sure you use the exact class name that contains the member you want.
     *
     * @param[in] ClassName - Name of the target UClass.
     * @param[in] FunctionName - Name of the target UFunction.
     * @param[in,out] (optional) OutFunction - Pointer to the output UFunction*, no value is written if unfound.
     */
    struct FSUFunction
    {
        FName ClassName;
        FName FunctionName;
        class UFunction** OutFunction;

        explicit FSUFunction(const std::string& ClassName, const std::string& FunctionName, class UFunction** OutFunction)
            : ClassName(FName(ClassName))
            , FunctionName(FName(FunctionName))
            , OutFunction(OutFunction)
        {
        }
    };

    /**
     * @brief Used to find the enumerator value for a UEnum.
     *
     * @param[in] EnumName - Name of the target UEnum.
     * @param[in] EnumeratorName - Name of the target enumerator.
     * @param[in,out] (optional) OutEnumeratorValue - Pointer to the output enumerator value, no value is written if unfound.
     * @param[in,out] (optional) OutEnum - Pointer to the output UEnum*, no value is written if unfound.
     */
    struct FSUEnum
    {
        FName EnumName;
        FName EnumeratorName;
        int64_t* OutEnumeratorValue;
        class UEnum** OutEnum;

        explicit FSUEnum(const std::string& EnumName, const std::string& EnumeratorName, int64_t* OutEnumeratorValue, class UEnum** OutEnum)
            : EnumName(FName(EnumName))
            , EnumeratorName(FName(EnumeratorName))
            , OutEnumeratorValue(OutEnumeratorValue)
            , OutEnum(OutEnum)
        {
        }
        explicit FSUEnum(const std::string& EnumName, const std::string& EnumeratorName, int64_t* OutEnumeratorValue)
            : EnumName(FName(EnumName))
            , EnumeratorName(FName(EnumeratorName))
            , OutEnumeratorValue(OutEnumeratorValue)
            , OutEnum(nullptr)
        {
        }
    };

    /**
     * @brief Used to find a property member of a UClass.
     * @brief This does not search inhereted classes, so make sure you use the exact class name that contains the member you want.
     *
     * @param[in] ClassName - Name of the target UClass.
     * @param[in] PropertyName - Name of the target property.
     * @param[in,out] (optional) OutOffset - Pointer to the output offset, no value is written if unfound.
     * @param[in,out] (optional) OutMask - Pointer to the output bitmask, no value is written if unfound or unsupported on property type.
     */
    struct FSProperty
    {
        FName ClassName;
        FName PropertyName;
        int32_t* OutOffset;
        uint8_t* OutMask;

        explicit FSProperty(const std::string& ClassName, const std::string& PropertyName, int32_t* OutOffset, uint8_t* OutMask)
            : ClassName(FName(ClassName))
            , PropertyName(FName(PropertyName))
            , OutOffset(OutOffset)
            , OutMask(OutMask)
        {
        }
    };

    /** @brief Internal use only. Refer to other structs prefixed with FS. */
    enum FSType
    {
        FS_UOBJECT,
        FS_UENUM,
        FS_UFUNCTION,
        FS_PROPERTY,
    };

    /** @brief Internal use only. Refer to other structs prefixed with FS. */
    struct FSEntry
    {
        FSType Type;
        union
        {
            struct FSUObject Object;
            struct FSProperty Property;
            struct FSUEnum Enum;
            struct FSUFunction Function;
        };

        FSEntry(const FSUObject& Object)
            : Type(FS_UOBJECT)
            , Object(Object)
        {
        }
        FSEntry(const FSUEnum& Enum)
            : Type(FS_UENUM)
            , Enum(Enum)
        {
        }
        FSEntry(const FSUFunction& Function)
            : Type(FS_UFUNCTION)
            , Function(Function)
        {
        }
        FSEntry(const FSProperty& Property)
            : Type(FS_PROPERTY)
            , Property(Property)
        {
        }
    };

    /** @brief UClass wrapper for FSUObject. */
    inline FSUObject FSUClass(const std::string& ObjectName, class UClass** OutClass) { return FSUObject(ObjectName, CASTCLASS_UClass, OutClass); }
}

namespace SDK
{
    /**
     * @brief Batch searching of UObjects, UClasses, UFunctions, class member offsets and enumerator values.
     * @brief Found entries are removed from the input SearchList.
     * @brief For information on the behaviour of different search entries, view the documentation of all FSEntry types.
     *
     * @param[in,out] SearchList - Reference to a list of entries to search for. Entry types are prefixed with FS for FastSearch, i.e FSUObject.
     *
     * @return If all fast search entries were found.
     */
    bool FastSearch(std::vector<FSEntry>& SearchList);

    /** @brief Wrapper for searching for a single FSEntry. Batch searching is optimal. */
    template <typename T>
    inline bool FastSearchSingle(const T Entry)
    {
        std::vector<FSEntry> Search = { Entry };
        return FastSearch(Search);
    }
}

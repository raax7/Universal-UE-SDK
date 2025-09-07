#pragma once
#include <uesdk/core/UnrealEnums.hpp>
#include <uesdk/core/UnrealTypes.hpp>

#include <vector>

namespace SDK
{
    /**
     * @brief Used to find a UObject matching the specified EClassCastFlags.
     */
    struct FSUObject
    {
        FName ObjectName;
        EClassCastFlags RequiredType;
        class UObject** OutObject;

        /**
         * @brief Construct an FSUObject search entry without a required type.
         *
         * @tparam T - A type derived from UObject.
         * @param[in] ObjectName - Name of the UObject to find.
         * @param[in,out] OutObject - Pointer to the output T*, no value is written if unfound.
         */
        template <typename T>
        explicit FSUObject(std::string_view ObjectName, T** OutObject)
            : ObjectName(FName(ObjectName))
            , RequiredType(CASTCLASS_None)
            , OutObject(reinterpret_cast<class UObject**>(OutObject))
        {
            static_assert(std::is_base_of_v<UObject, T>, "T must derive from UObject.");
        }

        /**
         * @brief Construct an FSUObject search entry with a required type.
         *
         * @tparam T - A type derived from UObject.
         * @param[in] ObjectName - Name of the UObject to find.
         * @param[in] RequiredType - Required EClassCastFlags for the target UObject.
         * @param[in,out] OutObject - Pointer to the output T*, no value is written if unfound.
         */
        template <typename T>
        explicit FSUObject(std::string_view ObjectName, uint64_t RequiredType, T** OutObject)
            : ObjectName(FName(ObjectName))
            , RequiredType(static_cast<EClassCastFlags>(RequiredType))
            , OutObject(reinterpret_cast<class UObject**>(OutObject))
        {
            static_assert(std::is_base_of_v<UObject, T>, "T must derive from UObject.");
        }
    };

    /**
     * @brief Used to find a UFunction member of a UClass.
     * @brief This does not search inherited classes, so make sure you use the exact class name that contains the member you want.
     */
    struct FSUFunction
    {
        FName ClassName;
        FName FunctionName;
        class UFunction** OutFunction;

        /**
         * @brief Construct an FSUFunction search entry.
         *
         * @param[in] ClassName - Name of the target UClass.
         * @param[in] FunctionName - Name of the target UFunction.
         * @param[in,out] OutFunction - Pointer to the output UFunction*, no value is written if unfound.
         */
        explicit FSUFunction(std::string_view ClassName, std::string_view FunctionName, class UFunction** OutFunction)
            : ClassName(FName(ClassName))
            , FunctionName(FName(FunctionName))
            , OutFunction(OutFunction)
        {
        }
    };

    /**
     * @brief Used to find the enumerator value for a UEnum.
     */
    struct FSUEnum
    {
        FName EnumName;
        FName EnumeratorName;
        int64_t* OutEnumeratorValue;
        class UEnum** OutEnum;

        /**
         * @brief Construct an FSUEnum search entry with both output value and UEnum.
         *
         * @param[in] EnumName - Name of the target UEnum.
         * @param[in] EnumeratorName - Name of the target enumerator.
         * @param[in,out] OutEnumeratorValue - Pointer to the output enumerator value, no value is written if unfound.
         * @param[in,out] OutEnum - Pointer to the output UEnum*, no value is written if unfound.
         */
        explicit FSUEnum(std::string_view EnumName, std::string_view EnumeratorName, int64_t* OutEnumeratorValue, class UEnum** OutEnum)
            : EnumName(FName(EnumName))
            , EnumeratorName(FName(EnumeratorName))
            , OutEnumeratorValue(OutEnumeratorValue)
            , OutEnum(OutEnum)
        {
        }

        /**
         * @brief Construct an FSUEnum search entry with only output enumerator value.
         *
         * @param[in] EnumName - Name of the target UEnum.
         * @param[in] EnumeratorName - Name of the target enumerator.
         * @param[in,out] OutEnumeratorValue - Pointer to the output enumerator value, no value is written if unfound.
         */
        explicit FSUEnum(std::string_view EnumName, std::string_view EnumeratorName, int64_t* OutEnumeratorValue)
            : EnumName(FName(EnumName))
            , EnumeratorName(FName(EnumeratorName))
            , OutEnumeratorValue(OutEnumeratorValue)
            , OutEnum(nullptr)
        {
        }
    };

    /**
     * @brief Used to find a property member of a UClass.
     * @brief This does not search inherited classes, so make sure you use the exact class name that contains the member you want.
     */
    struct FSProperty
    {
        FName ClassName;
        FName PropertyName;
        PropertyInfo* OutPropInfo;

        /**
         * @brief Construct a FSProperty search entry.
         *
         * @param[in] ClassName - Name of the target UClass.
         * @param[in] PropertyName - Name of the target property.
         * @param[in,out] OutPropInfo - Pointer to the output property info, no value is written if unfound.
         */
        explicit FSProperty(const std::string& ClassName, const std::string& PropertyName, PropertyInfo* OutPropInfo)
            : ClassName(FName(ClassName))
            , PropertyName(FName(PropertyName))
            , OutPropInfo(OutPropInfo)
        {
        }
    };

    /** @brief Internal use only. Refer to other structs prefixed with FS. */
    enum class FSType
    {
        UObject,
        UEnum,
        UFunction,
        Property,
    };

    /** @brief Internal use only. Refer to other structs prefixed with FS. */
    struct FSEntry
    {
        FSType Type;
        union
        {
            struct FSUObject Object;
            struct FSUFunction Function;
            struct FSUEnum Enum;
            struct FSProperty Property;
        };

        FSEntry(const FSUObject& Object)
            : Type(FSType::UObject)
            , Object(Object)
        {
        }
        FSEntry(const FSUFunction& Function)
            : Type(FSType::UFunction)
            , Function(Function)
        {
        }
        FSEntry(const FSUEnum& Enum)
            : Type(FSType::UEnum)
            , Enum(Enum)
        {
        }
        FSEntry(const FSProperty& Property)
            : Type(FSType::Property)
            , Property(Property)
        {
        }
    };

    /** @brief UClass wrapper for FSUObject. */
    inline FSUObject FSUClass(std::string_view ObjectName, class UClass** OutClass) { return FSUObject(ObjectName, OutClass); }
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

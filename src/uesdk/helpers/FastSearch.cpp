#include <uesdk/State.hpp>
#include <uesdk/core/ObjectArray.hpp>
#include <uesdk/core/UnrealContainers.hpp>
#include <uesdk/core/UnrealObjects.hpp>
#include <uesdk/helpers/FastSearch.hpp>

namespace SDK
{
    bool ProcessSearchEntry(const UObject* Obj, const FSEntry& Entry)
    {
        switch (Entry.Type) {
        case FSType::UObject: {
            if (!Obj->HasTypeFlag(Entry.Object.RequiredType) || Obj->Name != Entry.Object.ObjectName)
                break;

            *Entry.Object.OutObject = const_cast<UObject*>(Obj);
            return true;
        }
        case FSType::UEnum: {
            if (!Obj->HasTypeFlag(CASTCLASS_UEnum) || Obj->Name != Entry.Enum.EnumName)
                break;

            const UEnum* ObjEnum = reinterpret_cast<const UEnum*>(Obj);
            int64_t Value = ObjEnum->FindEnumerator(Entry.Enum.EnumeratorName);
            if (Value == OFFSET_NOT_FOUND)
                break;

            if (Entry.Enum.OutEnumeratorValue)
                *Entry.Enum.OutEnumeratorValue = Value;
            if (Entry.Enum.OutEnum)
                *Entry.Enum.OutEnum = const_cast<UEnum*>(ObjEnum);

            return true;
        }
        case FSType::UFunction: {
            if (!Obj->HasTypeFlag(CASTCLASS_UStruct) || Obj->Name != Entry.Function.ClassName)
                break;

            const UStruct* ObjStruct = reinterpret_cast<const UStruct*>(Obj);
            UFunction* Function = ObjStruct->FindFunction(Entry.Function.FunctionName);
            if (!Function)
                break;

            *Entry.Function.OutFunction = Function;

            return true;
        }
        case FSType::Property: {
            if (!Obj->HasTypeFlag(CASTCLASS_UStruct) || Obj->Name != Entry.Property.ClassName)
                break;

            const UStruct* ObjStruct = reinterpret_cast<const UStruct*>(Obj);
            PropertyInfo Info = ObjStruct->FindProperty(Entry.Property.PropertyName);
            if (!Info.Found)
                break;

            *Entry.Property.OutPropInfo = Info;

            return true;
        }

        default:
            throw std::logic_error("Invalid FSType!");
        }

        return false;
    }

    bool FastSearch(std::vector<FSEntry>& SearchList)
    {
        // We require all of these functionalities.
        if (!State::SetupFMemory || !State::SetupGObjects || !State::SetupAppendString)
            return false;

        for (int32_t i = 0; i < GObjects->Num(); i++) {
            if (SearchList.empty())
                return true;

            UObject* Obj = GObjects->GetByIndex(i);
            if (!Obj)
                continue;

            auto It = SearchList.begin();
            while (It != SearchList.end()) {
                if (ProcessSearchEntry(Obj, *It))
                    It = SearchList.erase(It);
                else
                    ++It;
            }
        }

        return SearchList.empty();
    }
}

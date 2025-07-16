#include <uesdk/FastSearch.hpp>
#include <uesdk/ObjectArray.hpp>
#include <uesdk/State.hpp>
#include <uesdk/UnrealContainers.hpp>
#include <uesdk/UnrealObjects.hpp>

namespace SDK
{
    bool FastSearch(std::vector<FSEntry>& SearchList)
    {
        // We require all of these functionalities.
        if (!State::SetupFMemory || !State::SetupGObjects || !State::SetupAppendString)
            return false;

        for (int i = 0; (i < GObjects->Num()) && SearchList.size(); i++) {
            UObject* Obj = GObjects->GetByIndex(i);
            if (!Obj)
                continue;

            auto It = SearchList.begin();
            while (It != SearchList.end()) {
                bool Found = false;

                switch (It->Type) {
                case FS_UOBJECT: {
                    if (Obj->HasTypeFlag(It->Object.RequiredType) && Obj->Name == It->Object.ObjectName) {
                        *It->Object.OutObject = Obj;
                        Found = true;
                    }
                    break;
                }
                case FS_UENUM: {
                    if (!Obj->HasTypeFlag(CASTCLASS_UEnum) || Obj->Name != It->Enum.EnumName)
                        break;

                    UEnum* ObjEnum = reinterpret_cast<UEnum*>(Obj);
                    int64_t Value = ObjEnum->FindEnumerator(It->Enum.EnumeratorName);
                    if (Value == OFFSET_NOT_FOUND)
                        break;

                    if (It->Enum.OutEnumeratorValue)
                        *It->Enum.OutEnumeratorValue = Value;
                    if (It->Enum.OutEnum)
                        *It->Enum.OutEnum = ObjEnum;

                    Found = true;
                    break;
                }
                case FS_UFUNCTION: {
                    if (!Obj->HasTypeFlag(CASTCLASS_UStruct) || Obj->Name != It->Function.ClassName)
                        break;

                    UStruct* ObjStruct = reinterpret_cast<UStruct*>(Obj);
                    UFunction* Function = ObjStruct->FindFunction(It->Function.FunctionName);
                    if (!Function)
                        break;

                    *It->Function.OutFunction = Function;

                    Found = true;
                    break;
                }
                case FS_PROPERTY: {
                    if (!Obj->HasTypeFlag(CASTCLASS_UStruct) || Obj->Name != It->Property.ClassName)
                        break;

                    UStruct* ObjStruct = reinterpret_cast<UStruct*>(Obj);
                    PropertyInfo Info = ObjStruct->FindProperty(It->Property.PropertyName);
                    if (!Info.Found)
                        break;

                    if (It->Property.OutOffset)
                        *It->Property.OutOffset = Info.Offset;
                    if (It->Property.OutMask)
                        *It->Property.OutMask = Info.ByteMask;

                    Found = true;
                    break;
                }

                default:
                    ++It;
                    break;
                }

                if (Found)
                    It = SearchList.erase(It);
                else
                    ++It;
            }
        }

        return SearchList.empty();
    }
}

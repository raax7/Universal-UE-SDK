#pragma once
#include <uesdk/ObjectArray.hpp>
#include <uesdk/UnrealContainers.hpp>
#include <uesdk/UnrealObjects.hpp>

namespace SDK
{
    template <typename UEType>
    UEType* TUObjectArray::FindObject(const std::string& FullName, EClassCastFlags RequiredType)
    {
        for (int i = 0; i < GObjects->Num(); i++) {
            UObject* Object = GObjects->GetByIndex(i);
            if (!Object)
                continue;

            if (Object->HasTypeFlag(RequiredType) && Object->GetFullName() == FullName)
                return static_cast<UEType*>(Object);
        }

        return nullptr;
    }

    template <typename UEType>
    UEType* TUObjectArray::FindObjectFast(const std::string& Name, EClassCastFlags RequiredType)
    {
        FName fName = FName(Name);

        for (int i = 0; i < GObjects->Num(); i++) {
            UObject* Object = GObjects->GetByIndex(i);
            if (!Object)
                continue;

            if (Object->HasTypeFlag(RequiredType) && Object->Name == fName)
                return static_cast<UEType*>(Object);
        }

        return nullptr;
    }

    template <typename UEType>
    UEType* TUObjectArray::FindObjectFastInOuter(const std::string& Name, const std::string& Outer)
    {
        FName fName = FName(Name);
        FName fOuter = FName(Outer);

        for (int i = 0; i < GObjects->Num(); i++) {
            UObject* Object = GObjects->GetByIndex(i);
            if (!Object)
                continue;

            if (Object->Name == fName && Object->Outer->Name == fOuter)
                return static_cast<UEType*>(Object);
        }

        return nullptr;
    }
}

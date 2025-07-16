#pragma once
#include <exampleutils.h>

/**
 * Lists all AActors along with their locations.
 */

namespace ListActors
{
    void Example()
    {
        // Create output stringstream
        std::stringstream ss;

        // Get a pointer to the UClass for AActor.
        SDK::UClass* ActorClass = nullptr;
        if (!FastSearchSingle(FSUClass("Actor", &ActorClass)))
            return;

        // Loop through every UObject.
        for (int i = 0; i < SDK::GObjects->Num(); i++) {
            SDK::UObject* Obj = SDK::GObjects->GetByIndex(i);

            // Check if the object is valid, not a default object and is or inherits from AActor.
            if (!Obj || Obj->IsDefaultObject() || !Obj->IsA(ActorClass))
                continue;

            // Use PECallWrapper to call the UFunction, this way the library will automatically setup the parameters struct for you.
            static SDK::PECallWrapper<"Actor", "K2_GetActorLocation", FVector()> K2_GetActorLocation;

            FVector ActorPos = K2_GetActorLocation.CallAuto(Obj);
            std::string ActorName = Obj->Name.ToString();

            // Output the actor name and position.
            ss << ActorName << '\n';
            ss << "X: " << ActorPos.X << " Y: " << ActorPos.Y << " Z: " << ActorPos.Z << "\n";
        }

        WriteSSToFile("ListActors", ss);
    }
}

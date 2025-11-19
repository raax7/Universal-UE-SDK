<h1 align="center">Universal-UE-SDK</h1>

<p align="center">A simple UE4 & UE5 wrapper for fundamental Unreal-Engine structs, classes and functionality.
</p>
<p align="center">
  If you found the project useful, feel free to join the Discord and star the repo!<br>
  UESDK is still under development, so any and all help or bug reporting is appreciated!<br>
  A lot of the code from this repo is taken from Dumper-7, so please go over and star them too.
</p>

<p align="center">
    <a href="https://github.com/ilyida/Universal-UE-SDK/issues">Report an Issue</a> |
    <a href="https://github.com/Encryqed/Dumper-7">Dumper-7</a>
</p>
<p align="center">
    <img alt="Stars" src="https://img.shields.io/github/stars/ilyida/Universal-UE-SDK?color=blue&style=for-the-badge">
</p>

## Table of Contents
<ol>
    <li><a href="#credits">Credits</a></li>
    <li><a href="#project-info">Project Info</a></li>
    <li><a href="#how-to-help">How to Help</a></li>
    <li><a href="#examples">Examples</a></li>
    <li><a href="#how-to-use">How to Use</a></li>
</ol>


## Credits
- [Dumper-7](https://github.com/Encryqed/Dumper-7) - Most fundamental structures and offset-finding methods are taken from Dumper-7, so huge credit to them. This project would not be possible without their open-source contributions. If you need a static SDK dumper for any Unreal Engine game, I highly recommend checking out Dumper-7.
- [libhat](https://github.com/BasedInc/libhat) - A fast and easy-to-use pattern matcher, offering additional abstractions as well.


## Project Info
Universal-UE-SDK (UESDK) is a universal SDK framework for UE4 & UE5 offering fundemental Unreal-Engine structures as well as some convenience features such as PECallWrapper.

UESDK is designed to simplify making an Unreal-Engine mod support multiple Unreal-Engine versions, offering runtime solutions for an SDK as opposed to compile-time solutions such as Dumper-7. UESDK does not come pre-loaded with classes like AActor, UWorld, UEngine, etc and instead gives the user full control.

### Compiler Support
- **MSVC** (recommended, full support)
- **Clang** (partial support, may work with MSVC compatibility extensions)
- **GCC** (not officially supported)  
All compiler compatibility issues come from the use of MSVC's "virtual member" feature.  
Removing this from ReflectionMacros.hpp will result in full compatibility.

### Unreal-Engine support
Both **UE4** & **UE5** are officially supported. There are currently no plans to add support for UE3.

### Differences between Dumper-7
Although this project takes a lot of inspiration from Dumper-7, it serves a different purpose.  
Dumper-7 generates a static SDK that is tied to a specific game build, whereas this project focuses on providing wrappers for more dynamic, runtime SDK usage.
| Feature                   | Dumper-7 | Universal-UE-SDK     |
| ------------------------- | -------- | -------------------- |
| Core struct dumps         | ✅        | ✅ (wrapped)       |
| Static SDK generation     | ✅        | ❌ (not the goal)  |
| Runtime SDK generation    | ❌        | ✅                 |
| Wrappers and abstraction  | ❌        | ✅                 |


## How to Help
UESDK is still early in development and may contain bugs, incorrect or incomplete structures, design issues, etc. If you come across an issue whilst using UESDK, feel free to open an issue on GitHub!


## Examples
### Listing information for every AActor:
This example covers:
- Making custom classes extended from UObject.
- Using reflection macros to handle reflected members.
- Using PECallWrapper to call process-event functions easily.
- Iterating GObjects.
- Casting UObjects.

```C++
#include <uesdk.hpp>
#include <print>

// UE4 uses float for matrices, UE5 uses double.
// Uncomment for UE5:
// #define UE5

#ifdef UE5
using MatrixType = double;
#else
using MatrixType = float;
#endif

struct FVector {
    MatrixType X, Y, Z;
};


// Dummy declaration for compilation.
class USceneComponent;


// Trivially copyable POD struct for PECallWrapper.
struct FHitResult {
public:
    // Allows same features as UESDK_UOBJECT (see below)
    // without needing to inherit from UObject.
    UESDK_STRUCT("HitResullt", FHitResult);

public:
    // Only the smaller of the struct and engine's actual size is copied when using PECallWrapper.
    // Must be >= actual engine size to ensure full data is preserved across versions.
    std::byte m_Pad[0x100];
};

// Verify struct is trivially-copyable and supports dynamic struct sizes in PECallWrapper.
static_assert(std::is_trivially_copyable_v<FHitResult>);


// Reflected AActor class.
class AActor : public SDK::UObject {
public:
    // Sets up class as a reflected UObject class.
    // UESDK_STRUCT should be used for structs like FActorTickFunction.
    UESDK_UOBJECT("Actor", AActor);

public:
    // Setup custom offset property.
    constexpr static auto CustomOffset = 0; // Offset of VFT
    UESDK_UPROPERTY_OFFSET(void*, Custom, CustomOffset);

    // Setup bit-field property (with fallback to bool).
    UESDK_UPROPERTY_BITFIELD(bHidden);

    // Setup standard property.
    UESDK_UPROPERTY(USceneComponent*, RootComponent);

public:
    /*
    * PECallWrapper wraps UObject::ProcessEvent calls into a simple function signature.
    *
    * Typically, calling a PE function requires a parameter struct and manual output parameter handling.
    * PECallWrapper automatically manages a parameter buffer and handles output parameters.
    */

    // Example const-function with return value.
    FVector K2_GetActorLocation() const {
        static SDK::PECallWrapper<"Actor", "K2_GetActorLocation", FVector()> Function;
        return Function.CallAuto(this);
    }

    // Example function with input parameters.
    void K2_SetActorLocation(
        const FVector& NewLocation,
        bool bSweep,
        FHitResult& SweepHitResult,
        bool bTeleport
    )
    {
        static SDK::PECallWrapper<"Actor", "K2_SetActorLocation", void(const FVector&, bool, FHitResult&, bool)> Function;
        Function.CallAuto(this, NewLocation, bSweep, SweepHitResult, bTeleport);
    }
};


bool ListActorInfo()
{
    // Initialize SDK.
    if (SDK::Init() != SDK::ESDKStatus::Success)
        return false;

    // Loop through every UObject.
    for (int32_t i = 0; i < SDK::GObjects->Num(); i++) {
        static SDK::UObject* Obj = SDK::GObjects->GetByIndex(i);

        // Check if the object is valid, not a default object and is or inherits from AActor.
        if (!Obj || Obj->IsDefaultObject())
            continue;

        // Cast<T> is a type-safe UE cast, returning nullptr if the cast is invalid.
        static AActor* Actor = SDK::Cast<AActor>(Obj);
        if (!Actor)
            continue;

        // Get the actor's location using our custom function.
        FVector ActorPos = Actor->K2_GetActorLocation();

        // We can access members, including bit-field members, as if they were standard members.
        // This is thanks to MSVCs "virtual member" functionality, sorry GCC & Clang users.
        void*               Custom          = Actor->Custom;        // custom
        bool                bHidden         = Actor->bHidden;       // bit-field
        USceneComponent*    RootComponent   = Actor->RootComponent; // standard

        // We can also interface with standard UE types like UObject::Name and convert to a std::string.
        std::string ActorName = Obj->Name.ToString();

        // Output gathered information.
        std::println("ActorName:        {}", ActorName);
        std::println("ActorPos:         X,Y,Z -> {:.2f}, {:.2f}, {:.2f}", ActorPos.X, ActorPos.Y, ActorPos.Z);
        std::println("Custom:           {}", Custom);
        std::println("bHidden:          {}", bHidden);
        std::println("RootComponent:    {}", reinterpret_cast<void*>(RootComponent));
        std::println();
    }

    return true;
}
```

---

### Getting basic UE information using FastSearch.
This example covers:
- Using FastSearch & FastSearchSingle.
- Finding a UClass.
- Finding a UFunction.
- Finding an enumerator value.
- Getting property info.

```C++
#include <uesdk.hpp>
#include <print>

bool FindObjects()
{
    // Initialize SDK.
    if (SDK::Init() != SDK::ESDKStatus::Success)
        return false;

    // Prepare output variables for searching.
    SDK::UClass*        ActorClass = nullptr;
    SDK::UFunction*     K2_GetActorLocation = nullptr;
    int64_t             ROLE_Authority = 0;
    SDK::PropertyInfo   RootComponent{};

    // Create vector to hold SDK::FSEntry.
    std::vector<SDK::FSEntry> Entries = {
        SDK::FSUClass(      "Actor",                            &ActorClass),
        SDK::FSUFunction(   "Actor",    "K2_GetActorLocation",  &K2_GetActorLocation),
        SDK::FSUEnum(       "ENetRole", "ROLE_Authority",       &ROLE_Authority),
        SDK::FSProperty(    "Actor",    "RootComponent",        &RootComponent),
    };

    // Attempt to find all entries, returning false if any weren't found.
    // Entries that are unfound are left in Entries, any found are removed.
    if (!SDK::FastSearch(Entries))
        return false;


    // Prepare output variable for single searching.
    SDK::UClass* SceneComponent = nullptr;

    // Call small wrapper for FastSearch to search for a single SDK::FSEntry.
    if (!SDK::FastSearchSingle(SDK::FSUClass("SceneComponent", &SceneComponent)))
        return false;

    // Output gathered information from SDK::FastSearch.
    std::println("FastSearch:");
    std::println("  ActorClass:           {}", reinterpret_cast<void*>(ActorClass));
    std::println("  K2_GetActorLocation:  {}", reinterpret_cast<void*>(K2_GetActorLocation));
    std::println("  ROLE_Authority:       {}", ROLE_Authority);
    std::println("  RootComponent:        {}", RootComponent.Offset);
    std::println();

    // Output gathered information from SDK::FastSearchSingle.
    std::println("FastSearchSingle:");
    std::println("  SceneComponent:       {}", reinterpret_cast<void*>(SceneComponent));
    std::println();

    return true;
}
```


## How to Use
### Setting up library
UESDK uses CMake as it's build system meaning you can simply build in the command line.  
This will generate the required library files, allowing you to link with any compiler you wish.
```
cmake -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
```

Alternatively, if you are using CMake for your own project, you can add UESDK as a sub directory with something like:
```cmake
add_subdirectory(extern/uesdk)
target_link_libraries(ProjectName PRIVATE uesdk)
```

### Using the library
#### Prerequisites
All functions and classes should have clear documentation, so if you are ever stuck remember to consult the documentation first.

It's important to note that UESDK should only ever be used from inside Unreal-Engine's "game thread", as nearly every piece of functionality relies on objects only safely accessible from within said thread.

#### Basic Usage
To use the library include ``uesdk.hpp``.
```C++
#include <uesdk.hpp>
```

After including the main header file, you should initialize the SDK before attempting to use it. It is important to check that initialization succeeded since failure is possible.
```C++
const auto Status = SDK::Init();
if (Status != SDK::Status::Success)
{
    // Handle error...
}
```

After this, you can safely use the SDK.

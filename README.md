<h1 align="center">Universal-UE-SDK</h1>

<p align="center">A simple UE4 and UE5 wrapper for fundamental internal Unreal-Engine structs, classes and functionality.
</p>
<p align="center">
  If you found this useful, please join the Discord server and star the project!<br>
  This project is still under development. Any and all help and bug reporting is appreciated!<br>
  A lot of the code from this repo is taken from Dumper-7, so please go over and star them too.
</p>

<p align="center">
	<a href="https://2ly.link/26uAo">Discord Server</a> |
	<a href="https://github.com/raax7/Universal-UE-SDK/issues">Report an Issue</a> |
	<a href="https://github.com/Encryqed/Dumper-7">Dumper-7</a>
</p>
<p align="center">
    <img alt="Stars" src="https://img.shields.io/github/stars/raax7/Universal-UE-SDK?color=blue&style=for-the-badge">
</p>

## Table of Contents
<ol>
    <li><a href="#credits">Credits</a></li>
    <li><a href="#project-info">Project Info</a></li>
    <li><a href="#how-to-help">How to Help</a></li>
    <li><a href="#how-to-use">How to Use</a></li>
</ol>


## Credits
- [Dumper-7](https://github.com/Encryqed/Dumper-7) - Most of the fundamental structures and offset-finding methods are taken from Dumper-7, so huge credit to them. This project would not be possible without their open-source contributions. If you need a static SDK dumper for any Unreal Engine game, I highly recommend checking out Dumper-7.
- [libhat](https://github.com/BasedInc/libhat) - A fast and easy-to-use pattern matcher, offering additional abstractions as well.


## Project Info
### Unreal-Engine version support
I only plan to support UE4 and UE5 with this project, UE3 is not planned.

### What this project is
This project is designed to help simplify the process of making an Unreal-Engine mod support multiple Unreal-Engine versions. This project offers simple and useful wrappers for fundamental features of Unreal-Engine such as UObject, UClass, FName, FString, TArray, etc.
### What this project is NOT
This project is NOT designed to be a complete SDK with other fundamental classes like AActor, APlayerController, UWorld, UEngine, etc. This is simply a framework for making creating those classes and structures significantly easier. This project will not be able to support modified versions of Unreal-Engine, I only plan to support base UE4 and UE5 versions.

### Differences from Dumper-7
Although this project takes a lot of inspiration from Dumper-7, it serves a different purpose.  
Dumper-7 generates a static SDK that is tied to a specific game build, whereas this project focuses on providing wrappers for more dynamic, runtime SDK usage.
| Feature                   | Dumper-7 | Universal-UE-SDK     |
| ------------------------- | -------- | -------------------- |
| Core struct dumps         | ✅        | ✅ (wrapped)       |
| Static SDK generation     | ✅        | ❌ (not the goal)  |
| Runtime SDK generation    | ❌        | ✅                 |
| Wrappers and abstraction  | ❌        | ✅                 |


## How to Help
This project is fairly early into development and will likely have lots of bugs, incorrect/incomplete structures and classes, design flaws, etc. I am open to any and all suggestions. If you have any problems using this library, please open an issue and I will help you fix it, or fix the library if needed.


## How to Use
### Setting up library
This project uses CMake, meaning we highly recommend that you make your project with CMake. It is possible to use this library without, but using CMake just makes the whole process easier. If you want to use it without, you should include the header files in your project and build the .lib file and manually include it in your project.
<br/><br/>
If you wish to use this library with CMake and don't know how, there are plenty of tutorials online. Simply Google something like "How to make a DLL in CMake" and then after you've done that Google something like "How to include a library in CMake". This library has a dependency so make sure to recursively clone the git project, or add the project as a git submodule.

### Using the library
Every function that you can call in the library should have thorough documentation, so if you are wondering what a function does simply read its documentation.

To use the library, first you need to include ``UESDK.hpp``.
```C++
#include <UESDK.hpp>
```

After including the main header file, you need to init the SDK before using any other SDK related function. You should check the status return value to make sure it initiated successfully.
```C++
const auto Status = SDK::Init();
if (Status != SDK::Status::Success)
{
    // Handle error...
}
```

After you've done this you can safely use the SDK. Here is an example of listing every AActor's name and location.
```C++
// UE4 uses float for matrices, UE5 uses double.
using MatrixType = float;

struct FVector {
    MatrixType X, Y, Z;
};

bool ListActorNamesAndLocations()
{
    // Get a pointer to the UClass for AActor.
    SDK::UClass* ActorClass = nullptr;
    if (!FastSearchSingle(FSUClass("Actor", &ActorClass)))
        return false;

    // Loop through every UObject.
    for (int i = 0; i < SDK::GObjects->Num(); i++) {
        SDK::UObject* Obj = SDK::GObjects->GetByIndex(i);

        // Check if the object is valid, not a default object and is or inherits from AActor.
        if (!Obj || Obj->IsDefaultObject() || !Obj->IsA(ActorClass))
            continue;

        // Use PECallWrapper to call the UFunction, this way the library will automatically setup the parameters struct for you.
        static SDK::PECallWrapper<"Actor", "K2_GetActorLocation", FVector()> K2_GetActorLocation;

        FVector ActorPos = K2_GetActorLocation.CallAuto(Obj);
        std::string ActorName = Obj->Name().ToString();

        // Output the actor name and position.
        std::cout << ActorName << std::endl;
        std::cout << "X: " << ActorPos.X << "\nY: " << ActorPos.Y << "\nZ: " << ActorPos.Z << "\n\n";
    }

    return true;
}
```

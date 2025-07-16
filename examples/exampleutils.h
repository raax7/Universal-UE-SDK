#pragma once
#include <UESDK.hpp>

#include <sstream>
#include <fstream>
#include <stdexcept>

// File-Utils
//
void WriteSSToFile(const std::string& FilePath, const std::stringstream& ss) {
    std::ofstream OutFile(FilePath);
    if (!OutFile.is_open())
        throw std::ios_base::failure("Failed to open file: " + FilePath);

    OutFile << ss.rdbuf();
}

// Unreal-Engine
//
#ifndef UE4
#ifndef UE5
static_assert(false, "Either UE4 or UE5 macro must be defined!");
#endif
#endif

#ifdef UE4
using MatrixType = float;
#else
using MatrixType = double;
#endif

struct FVector
{
    MatrixType X, Y, Z;
};

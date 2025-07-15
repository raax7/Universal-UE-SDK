#pragma once
#include <UESDK.hpp>

// Not trivially copyable struct
struct NotTrivial
{
    NotTrivial() { }
    NotTrivial(const NotTrivial&) { }
};

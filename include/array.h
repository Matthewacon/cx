#pragma once

#include "templates.h"

//TODO This one's not so easy...
//Cosntexpr array like std::array<typename T, std::size_t size>, except, the size need not be explicitly stated in
//constexpr construction. Can also accept a CX::Tuple with any parameters, with a predefined constexpr conversion
//function to unify all types to the array's target type, to create a homogeneous array of polymorphic values. - WIP
namespace CX {

}
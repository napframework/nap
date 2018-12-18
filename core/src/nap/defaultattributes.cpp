// Local Includes
#include "numeric.h"

// External Includes
#include <string>
#include <rtti/typeinfo.h>

// Numeric Defaults
RTTI_DEFINE_STRUCT(std::string)
RTTI_DEFINE_STRUCT(int)
RTTI_DEFINE_STRUCT(float)
RTTI_DEFINE_STRUCT(double)
RTTI_DEFINE_STRUCT(nap::uint)
RTTI_DEFINE_STRUCT(nap::int8)
RTTI_DEFINE_STRUCT(nap::uint8)

// Arrays of numeric defaults
RTTI_DEFINE_STRUCT(std::vector<double>)
RTTI_DEFINE_STRUCT(std::vector<std::string>)
RTTI_DEFINE_STRUCT(std::vector<int>)
RTTI_DEFINE_STRUCT(std::vector<float>)
RTTI_DEFINE_STRUCT(std::vector<nap::uint>)
RTTI_DEFINE_STRUCT(std::vector<nap::int8>)
RTTI_DEFINE_STRUCT(std::vector<nap::uint8>)
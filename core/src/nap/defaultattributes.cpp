// Local Includes
#include "numeric.h"

// External Includes
#include <string>
#include <rtti/typeinfo.h>

RTTI_DEFINE_STRUCT(std::string)
RTTI_DEFINE_STRUCT(int)
RTTI_DEFINE_STRUCT(float)
RTTI_DEFINE_STRUCT(double)
RTTI_DEFINE_STRUCT(std::vector<double>)
RTTI_DEFINE_STRUCT(nap::int8)
RTTI_DEFINE_STRUCT(nap::uint8)
RTTI_DEFINE_STRUCT(nap::int16)
RTTI_DEFINE_STRUCT(nap::uint16)
RTTI_DEFINE_STRUCT(nap::int32)
RTTI_DEFINE_STRUCT(nap::uint32)
RTTI_DEFINE_STRUCT(nap::uint64)
RTTI_DEFINE_STRUCT(nap::uint)

RTTI_DEFINE_STRUCT(std::vector<std::string>)
RTTI_DEFINE_STRUCT(std::vector<int>)
RTTI_DEFINE_STRUCT(std::vector<float>)
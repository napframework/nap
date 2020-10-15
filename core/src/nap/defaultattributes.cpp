/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "numeric.h"

// External Includes
#include <string>
#include <rtti/typeinfo.h>

// Numeric Defaults
RTTI_DEFINE_STRUCT(std::string)
RTTI_DEFINE_STRUCT(short)
RTTI_DEFINE_STRUCT(int)
RTTI_DEFINE_STRUCT(float)
RTTI_DEFINE_STRUCT(double)
RTTI_DEFINE_STRUCT(nap::uint)
RTTI_DEFINE_STRUCT(nap::int8)
RTTI_DEFINE_STRUCT(nap::uint8)

// Arrays of numeric defaults
RTTI_DEFINE_STRUCT(std::vector<double>)
RTTI_DEFINE_STRUCT(std::vector<std::string>)
RTTI_DEFINE_STRUCT(std::vector<short>)
RTTI_DEFINE_STRUCT(std::vector<int>)
RTTI_DEFINE_STRUCT(std::vector<float>)
RTTI_DEFINE_STRUCT(std::vector<nap::uint>)
RTTI_DEFINE_STRUCT(std::vector<nap::int8>)
RTTI_DEFINE_STRUCT(std::vector<nap::uint8>)
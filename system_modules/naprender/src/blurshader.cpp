/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local includes
#include "blurshader.h"
#include <nap/core.h>

RTTI_BEGIN_ENUM(nap::EBlurSamples)
	RTTI_ENUM_VALUE(nap::EBlurSamples::X5, "5x5"),
	RTTI_ENUM_VALUE(nap::EBlurSamples::X9, "9x9"),
	RTTI_ENUM_VALUE(nap::EBlurSamples::X13, "13x13")
RTTI_END_ENUM

// nap::Blur5x5Shader run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Blur5x5Shader, "Gaussian blur shader with a 5 texel kernel")
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

// nap::Blur9x9Shader run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Blur9x9Shader, "Gaussian blur shader with a 9 texel kernel")
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

// nap::Blur13x13Shader run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Blur13x13Shader, "Gaussian blur shader with a 13 texel kernel")
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

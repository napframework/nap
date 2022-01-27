/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local includes
#include "blurshader.h"
#include "renderservice.h"

// External includes
#include <nap/core.h>

RTTI_BEGIN_ENUM(nap::EBlurSamples)
	RTTI_ENUM_VALUE(nap::EBlurSamples::X5, "5x5"),
	RTTI_ENUM_VALUE(nap::EBlurSamples::X9, "9x9")
RTTI_END_ENUM

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Blur5x5Shader)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Blur9x9Shader)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

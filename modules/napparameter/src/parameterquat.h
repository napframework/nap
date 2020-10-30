/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "parametersimple.h"

// External Includes
#include <glm/gtc/quaternion.hpp>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// Quaternion Parameter Type Definitions
	//////////////////////////////////////////////////////////////////////////

	using ParameterQuat = ParameterSimple<glm::quat>;
}

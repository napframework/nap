/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <utility/dllexport.h>

namespace nap
{
	// Forward declares
	class UniformStruct;

	/**
	 * Returns the total size of the shader variable
	 */
	size_t NAPAPI getUniformStructSizeRecursive(const UniformStruct& uniforStruct);

	/**
	 * Returns the maximum depth of the shader variable
	 */
	int NAPAPI getUniformStructDepth(const UniformStruct& uniform);
}

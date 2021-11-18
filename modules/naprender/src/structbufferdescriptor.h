/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */
#pragma once

// External includes
#include <nap/resourceptr.h>
#include <nap/numeric.h>

namespace nap
{
	// Forward declares
	class UniformStruct;

	/**
	* StorageBufferDescriptor
	*/
	struct NAPAPI StructBufferDescriptor
	{
		RTTI_ENABLE()
	public:
		ResourcePtr<UniformStruct> mElement = nullptr;					///<
		uint mCount = 1;												///<
	};
}

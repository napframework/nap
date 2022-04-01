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
	 * StructBufferDescriptor describes the layout of a struct buffer. Combines the layout of a single element with a total
	 * element count.
	 * 
	 * This resource is used by StructGPUBuffer to denote the buffer layout, allocate the right amount of memory, and
	 * possibly store information on how fill the buffer accordingly.
	 */
	struct NAPAPI StructBufferDescriptor
	{
	public:
		ResourcePtr<UniformStruct> mElement;		///< Property 'Element': The layout of a single struct buffer element
		uint mCount = 1;							///< Property 'Count': The number of elements the buffer consists of
	};
}

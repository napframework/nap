/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "vertexbuffer.h"

// External Includes
#include <vector>
#include <nap/numeric.h>


namespace nap
{
	/**
	 * Index buffer, controls how triangles are formed when rendered.
	 * For more information on GPU buffers in general see: nap::GPUBuffer
	 */
	class NAPAPI IndexBuffer : public IntVertexBuffer
	{
		RTTI_ENABLE(IntVertexBuffer)
	public:
		/**
		 * Every index buffer needs to have access to the render engine.
		 * The given 'usage' controls if a mesh can be updated more than once, 
		 * and in which memory space it is placed.
		 * @param core the nap core
		 */
		IndexBuffer(Core& core) :
			IntVertexBuffer(core)
		{ }

		/**
		 * Every index buffer needs to have access to the render engine.
		 * The given 'usage' controls if a mesh can be updated more than once,
		 * and in which memory space it is placed.
		 * @param core the nap core
		 * @param usage how the buffer is used at runtime.
		 */
		IndexBuffer(Core& core, EMeshDataUsage usage) :
			IntVertexBuffer(core, usage, false)
		{ }

		/**
		 * Initialize this IndexBuffer
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Uploads index data to the GPU buffer.
		 * @param indices: Index data to upload to the GPU.
		 * @param error: contains the error message when the operation fails.
		 * @return if index data has been set correctly.
		 */
		bool setData(const std::vector<uint32>& indices, utility::ErrorState& error);
	};
}

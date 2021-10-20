/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "gpubuffer.h"
#include "uniformdeclarations.h"
#include "vulkan/vulkan_core.h"

// External Includes
#include <stdint.h>

namespace nap
{
	/**
	 * A list of vertices on the GPU that represent a specific attribute of the geometry, for example:
	 * position, uv0, uv1, color0, color1, normals etc.
	 * For more information on buffers on the GPU, refer to: nap::GPUBuffer
	 */
	class NAPAPI StorageBuffer : public GPUBuffer
	{
	public:
		/**
		 * Every vertex buffer needs to have access to the render engine.
		 * The given 'usage' controls if a buffer can be updated more than once, and in which memory space it is placed. 
		 * The format defines the vertex element size in bytes.
		 * @param renderService the render engine
		 * @param format buffer format, defines element size in bytes
		 * @param usage how the buffer is used at runtime.
		 */
		StorageBuffer(RenderService& renderService, EBufferObjectType type, EMeshDataUsage usage);

		/**
		 * @return vertex buffer format
		 */
		VkFormat getFormat() const { return mFormat; }

		/**
		 * Uploads data to the GPU based on the settings provided.
		 * This function automatically allocates GPU memory if required. 
		 * @param data pointer to the block of data that needs to be uploaded.
		 * @param the size of the data in bytes
		 * @param error contains the error if upload operation failed
		 * @return if upload succeeded
		 */
		bool setData(void* data, size_t size, utility::ErrorState& error);

	private:
		VkFormat			mFormat;
		EBufferObjectType	mType;
		int					mElementSize = -1;
	};
}

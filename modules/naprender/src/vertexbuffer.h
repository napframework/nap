/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "gpubuffer.h"
#include "vulkan/vulkan_core.h"

// External Includes
#include <stdint.h>

namespace nap
{
	/**
	 * Returns the size in bytes, for a single element, of the given format.
	 * @param format requested format
	 * @return size in bytes, for a single element, of the given format. -1 if unsupported.
	 */
	int getVertexSize(VkFormat format);

	/**
	 * A list of vertices on the GPU that represent a specific attribute of the geometry, for example:
	 * position, uv0, uv1, color0, color1, normals etc.
	 * For more information on buffers on the GPU, refer to: nap::GPUBuffer
	 */
	class NAPAPI VertexBuffer : public GPUBuffer
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
		VertexBuffer(RenderService& renderService, VkFormat format, EMeshDataUsage usage);

		/**
		 * @return vertex buffer format
		 */
		VkFormat getFormat() const { return mFormat; }

		/**
		 * Uploads data to the GPU based on the settings provided.
		 * This function automatically allocates GPU memory if required. 
		 * Ensure reservedNumVertices >= numVertices. Capacity is calculated based on reservedNumVertices.
		 * @param data pointer to the block of data that needs to be uploaded.
		 * @param numVertices number of vertices represented by data.
		 * @param reservedNumVertices used to calculate final buffer size, needs to be >= numVertices
		 * @param error contains the error if upload operation failed
		 * @return if upload succeeded
		 */
		bool setData(void* data, size_t numVertices, size_t reservedNumVertices, utility::ErrorState& error);

	private:
		VkFormat		mFormat;
		int				mVertexSize			= -1;
	};
}

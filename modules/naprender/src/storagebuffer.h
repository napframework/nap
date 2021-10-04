/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "gpubuffer.h"

// External Includes
#include <vector>
#include <nap/numeric.h>


namespace nap
{
	/**
	 * Storage buffer, allows setting usage manually
	 * For more information on GPU buffers in general see: nap::GPUBuffer
	 * TODO: Rename to TexelBuffer
	 */
	class NAPAPI StorageBuffer : public GPUBuffer
	{
	public:
		/**
		 * Every index buffer needs to have access to the render engine.
		 * The given 'usage' controls if a mesh can be updated more than once, 
		 * and in which memory space it is placed.
		 * @param renderService the render engine
		 * @param format buffer format, defines element size in bytes
		 * @param usage how the buffer is used at runtime.
		 */
		StorageBuffer(RenderService& renderService, VkFormat format, EMeshDataUsage usage);

		/**
		 * @return vertex buffer format
		 */
		VkFormat getFormat() const { return mFormat; }

		/**
		 * @return the number of indices specified in this buffer
		 */
		std::size_t getSize() const							{ return mSize; }

		/**
		 * Uploads index data to the GPU buffer.
		 * @param data: Data to upload to the GPU
		 * @param size: The size of the buffer
		 * @param bufferUsage: Buffer usage flags
		 * @param error: contains the error message when the operation fails.
		 * @return if index data has been set correctly.
		 */
		bool setData(void* data, size_t size, VkBufferUsageFlagBits bufferUsage, utility::ErrorState& error);

	private:
		VkFormat	mFormat;			///< data format
		size_t		mSize = 0;			///< buffer size
		int			mElementSize = 0;	///< element size
	};
}

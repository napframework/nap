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
#include <glm/glm.hpp>

namespace nap
{
	/**
	 * A list of vertices on the GPU that represent a specific attribute of the geometry, for example:
	 * position, uv0, uv1, color0, color1, normals etc.
	 * For more information on buffers on the GPU, refer to: nap::GPUBuffer
	 */
	template<class T>
	class NAPAPI GPUValueBuffer : public GPUBuffer
	{
		RTTI_ENABLE(GPUBuffer)
	public:
		/**
		 * Every vertex buffer needs to have access to the render engine.
		 * The given 'usage' controls if a buffer can be updated more than once, and in which memory space it is placed. 
		 * The format defines the vertex element size in bytes.
		 * @param renderService the render engine
		 */
		GPUValueBuffer(Core& core) :
			GPUBuffer(core)
		{ }

		/**
		 * Every vertex buffer needs to have access to the render engine.
		 * The given 'usage' controls if a buffer can be updated more than once, and in which memory space it is placed.
		 * The format defines the vertex element size in bytes.
		 * @param renderService the render engine
		 * @param type the buffer object typr
		 * @param usage how the buffer is used at runtime.
		 */
		GPUValueBuffer(Core& core, EBufferObjectType type, EMeshDataUsage usage) :
			GPUBuffer(core, usage), mType(type)
		{ }

		/**
		 * Init
		 */
		bool init(utility::ErrorState& errorState) override
		{
			if (!GPUBuffer::init(errorState))
				return false;

			mElementSize = sizeof(T);
			std::vector<T> staging_buffer(mCount);

			return setDataInternal(static_cast<void*>(staging_buffer.data()), mElementSize * mCount, static_cast<VkBufferUsageFlagBits>(getBufferUsage(mType)), errorState);
		}

		/**
		 * Uploads data to the GPU based on the settings provided.
		 * This function automatically allocates GPU memory if required. 
		 * @param data pointer to the block of data that needs to be uploaded.
		 * @param the size of the data in bytes
		 * @param error contains the error if upload operation failed
		 * @return if upload succeeded
		 */
		bool setData(void* data, size_t size, utility::ErrorState& error)
		{
			return setDataInternal(data, size, static_cast<VkBufferUsageFlagBits>(getBufferUsage(mType)), error);
		}

		uint32 getSize() const { return mCount * mElementSize };

		uint32 mCount = 0;										///< Property 'Count'
		EBufferObjectType mType	= EBufferObjectType::Uniform;	///< Property 'BufferObjectType'

	private:
		int	mElementSize = -1;
	};


	//////////////////////////////////////////////////////////////////////////
	// GPU buffer type definitions
	//////////////////////////////////////////////////////////////////////////

	using GPUIntBuffer = GPUValueBuffer<int>;
	using GPUFloatBuffer = GPUValueBuffer<float>;
	using GPUVec2Buffer = GPUValueBuffer<glm::vec2>;
	using GPUVec3Buffer = GPUValueBuffer<glm::vec3>;
	using GPUVec4Buffer = GPUValueBuffer<glm::vec4>;
	using GPUMat4Buffer = GPUValueBuffer<glm::mat4>;
}

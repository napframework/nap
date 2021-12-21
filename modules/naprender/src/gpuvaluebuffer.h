/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "gpubuffer.h"
#include "uniformdeclarations.h"
#include "vulkan/vulkan_core.h"
#include "mathutils.h"
#include "bufferfillpolicy.h"

// External Includes
#include <nap/resourceptr.h>
#include <stdint.h>
#include <glm/glm.hpp>

namespace nap
{
	/**
	 * For more information on buffers on the GPU, refer to: nap::GPUBuffer
	 */
	class NAPAPI GPUValueBuffer : public GPUBuffer
	{
		RTTI_ENABLE(GPUBuffer)
	public:
		GPUValueBuffer(Core& core) :
			GPUBuffer(core)
		{ }

		GPUValueBuffer(Core& core, EBufferObjectType type, EMeshDataUsage usage) :
			GPUBuffer(core, usage), mType(type)
		{ }

		/**
		 * @return the size of the buffer in bytes
		 */
		virtual uint32 getSize() const = 0;

		/**
		 * @return the number of buffer values
		 */
		virtual uint32 getCount() const = 0;

		EBufferObjectType mType = EBufferObjectType::Uniform;	///< Property 'BufferObjectType'
	};


	/**
	 * For more information on buffers on the GPU, refer to: nap::GPUBuffer
	 */
	template<typename T>
	class NAPAPI TypedGPUValueBuffer : public GPUValueBuffer
	{
		RTTI_ENABLE(GPUValueBuffer)
	public:
		/**
		 * Every vertex buffer needs to have access to the render engine.
		 * The given 'usage' controls if a buffer can be updated more than once, and in which memory space it is placed. 
		 * The format defines the vertex element size in bytes.
		 * @param renderService the render engine
		 */
		TypedGPUValueBuffer(Core& core) :
			GPUValueBuffer(core)
		{ }

		/**
		 * Every vertex buffer needs to have access to the render engine.
		 * The given 'usage' controls if a buffer can be updated more than once, and in which memory space it is placed.
		 * The format defines the vertex element size in bytes.
		 * @param renderService the render engine
		 * @param type the buffer object typr
		 * @param usage how the buffer is used at runtime.
		 */
		TypedGPUValueBuffer(Core& core, EBufferObjectType type, EMeshDataUsage usage) :
			GPUValueBuffer(core, type, usage)
		{ }

		/**
		 * Init
		 */
		virtual bool init(utility::ErrorState& errorState) override
		{
			if (!GPUValueBuffer::init(errorState))
				return false;

			mElementSize = sizeof(T);

			uint32 buffer_size = mCount * mElementSize;
			VkBufferUsageFlagBits buffer_usage = static_cast<VkBufferUsageFlagBits>(getBufferUsage(mType));

			// If usage is DynamicRead, allocate only
			if (mUsage == EMeshDataUsage::DynamicRead)
				return allocateInternal(buffer_size, buffer_usage, errorState);

			std::vector<T> staging_buffer;
			if (mBufferFillPolicy != nullptr)
				mBufferFillPolicy->fill(mCount, staging_buffer, errorState);
			else
				staging_buffer.resize(mCount, {});

			// Prepare staging buffer upload
			return setDataInternal(staging_buffer.data(), buffer_size, buffer_size, buffer_usage, errorState);
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

		/**
		 * @return the size of the buffer in bytes
		 */
		virtual uint32 getSize() const override { return mCount * mElementSize; };

		/**
		 * @return the number of buffer values
		 */
		virtual uint32 getCount() const override { return mCount; }

		ResourcePtr<TypedValueBufferFillPolicy<T>>			mBufferFillPolicy = nullptr;	///< Property 'FillPolicy'
		uint32												mCount = 0;						///< Property 'Count'

	private:
		int	mElementSize = -1;
	};


	//////////////////////////////////////////////////////////////////////////
	// GPU buffer type definitions
	//////////////////////////////////////////////////////////////////////////

	using GPUIntBuffer = TypedGPUValueBuffer<int>;
	using GPUFloatBuffer = TypedGPUValueBuffer<float>;
	using GPUVec2Buffer = TypedGPUValueBuffer<glm::vec2>;
	using GPUVec3Buffer = TypedGPUValueBuffer<glm::vec3>;
	using GPUVec4Buffer = TypedGPUValueBuffer<glm::vec4>;
	using GPUMat4Buffer = TypedGPUValueBuffer<glm::mat4>;
}

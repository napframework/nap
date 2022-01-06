/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "gpubuffer.h"
#include "uniformdeclarations.h"
#include "vulkan/vulkan_core.h"
#include "mathutils.h"
#include "valuebufferfillpolicy.h"
#include "formatutils.h"

// External Includes
#include <nap/resourceptr.h>
#include <nap/logger.h>
#include <stdint.h>
#include <glm/glm.hpp>

namespace nap
{
	/**
	 * For more information on buffers on the GPU, refer to: nap::GPUBuffer
	 */
	class NAPAPI VertexBuffer : public GPUBuffer
	{
		RTTI_ENABLE(GPUBuffer)
	public:
		VertexBuffer(Core& core) :
			GPUBuffer(core)
		{ }

		VertexBuffer(Core& core, EMeshDataUsage usage) :
			GPUBuffer(core, usage)
		{ }

		/**
		 * @return the size of the buffer in bytes
		 */
		virtual uint32 getSize() const = 0;

		/**
		 * @return the size of a single vertex element
		 */
		virtual uint32 getElementSize() const = 0;

		/**
		 * @return the number of buffer values
		 */
		virtual uint32 getCount() const = 0;

		/**
		 * @return the buffer format
		 */
		virtual VkFormat getFormat() const = 0;

		/**
		 * Returns whether this buffer is initialized
		 */
		virtual bool isInitialized() const = 0;

		/**
		 * Uploads data to the GPU based on the settings provided.
		 * This function automatically allocates GPU memory if required.
		 * @param data pointer to the block of data that needs to be uploaded.
		 * @param numVertices the number of vertices
		 * @param reservedNumVertices the number of vertices to reserve
		 * @param error contains the error if upload operation failed
		 * @return if upload succeeded
		 */
		virtual bool setData(void* data, size_t numVertices, size_t reservedNumVertices, utility::ErrorState& error) = 0;

		uint32 mCount = 0;						///< Property 'Count' The number of vertex elements to initialize/allocate the buffer with
		bool mClear = true;						///< Property 'Clear' If no fill policy is set, performs an initial clear-to-zero transfer operation on the device buffer on init()
	};


	/**
	 * For more information on buffers on the GPU, refer to: nap::GPUBuffer
	 */
	template<typename T>
	class NAPAPI TypedVertexBuffer : public VertexBuffer
	{
		RTTI_ENABLE(VertexBuffer)
	public:
		/**
		 * Every vertex buffer needs to have access to the render engine.
		 * The given 'usage' controls if a buffer can be updated more than once, and in which memory space it is placed. 
		 * The format defines the vertex element size in bytes.
		 * @param renderService the render engine
		 */
		TypedVertexBuffer(Core& core) :
			VertexBuffer(core)
		{ }

		/**
		 * Every vertex buffer needs to have access to the render engine.
		 * The given 'usage' controls if a buffer can be updated more than once, and in which memory space it is placed.
		 * The format defines the vertex element size in bytes.
		 * @param renderService the render engine
		 * @param usage how the buffer is used at runtime.
		 */
		TypedVertexBuffer(Core& core, EMeshDataUsage usage) :
			VertexBuffer(core, usage)
		{ }

		/**
		 * Initialize this buffer. This will allocate all required staging and device buffers based on the buffer properties.
		 * If a fill policy is available, the buffer will also be uploaded to immediately.
		 */
		virtual bool init(utility::ErrorState& errorState) override
		{
			if (!VertexBuffer::init(errorState))
				return false;

			if (!errorState.check(mCount != 0, "Failed to initialize vertex buffer. Cannot allocate a buffer with zero elements."))
				return false;
			
			uint32 buffer_size = mCount * sizeof(T);

			// Ensure a buffer cannot be marked as both index and vertex
			if (!(mUsageFlags & VK_BUFFER_USAGE_INDEX_BUFFER_BIT))
				mUsageFlags |= ((mVertexShaderAccess) ? VK_BUFFER_USAGE_VERTEX_BUFFER_BIT : 0);

			// Allocate buffer memory
			if (!allocateInternal(buffer_size, mUsageFlags, errorState))
				return false;

			// Upload data when a buffer fill policy is available
			if (mBufferFillPolicy != nullptr)
			{
				if (mUsage != EMeshDataUsage::DynamicRead)
				{
					std::vector<T> staging_buffer;
					mBufferFillPolicy->fill(mCount, staging_buffer, errorState);

					// Prepare staging buffer upload
					if (!setDataInternal(staging_buffer.data(), buffer_size, buffer_size, mUsageFlags, errorState))
						return false;
				}
				else
				{
					// Warn user that buffers cannot be filled when their usage is set to DynamicRead
					nap::Logger::warn(utility::stringFormat("%s: The configured fill policy was ignored as the buffer usage is DynamicRead", mID.c_str()).c_str());
				}
			}

			// Optionally clear
			else if (mClear)
			{
				// TODO: Implement buffer clear operations (exactly like textures)
			}

			mInitialized = true;
			return true;
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
			return setDataInternal(data, size, mUsageFlags, error);
		}

		/**
		 * Uploads data to the GPU based on the settings provided.
		 * This function automatically allocates GPU memory if required.
		 * @param data pointer to the block of data that needs to be uploaded.
		 * @param numVertices the number of vertices
		 * @param reservedNumVertices the number of vertices to reserve
		 * @param error contains the error if upload operation failed
		 * @return if upload succeeded
		 */
		virtual bool setData(void* data, size_t numVertices, size_t reservedNumVertices, utility::ErrorState& error) override
		{
			return setDataInternal(data, sizeof(T), numVertices, reservedNumVertices, mUsageFlags, error);
		}

		/**
		 * @return the size of the buffer in bytes
		 */
		virtual uint32 getSize() const override { return mCount * sizeof(T); };

		/**
		 * @return the size of a single vertex element
		 */
		virtual uint32 getElementSize() const override { return sizeof(T); };

		/**
		 * @return the number of buffer values
		 */
		virtual uint32 getCount() const override { return mCount; }

		/**
		 * @return the buffer format
		 */
		virtual VkFormat getFormat() const override
		{
			return getVertexBufferFormat<T>();
		}

		/**
		 * Returns whether this buffer is initialized
		 */
		virtual bool isInitialized() const { return mInitialized; };

		ResourcePtr<TypedValueBufferFillPolicy<T>>			mBufferFillPolicy = nullptr;	///< Property 'FillPolicy'

	private:
		bool mInitialized = false;
	};


	//////////////////////////////////////////////////////////////////////////
	// GPU buffer type definitions
	//////////////////////////////////////////////////////////////////////////

	using IntVertexBuffer = TypedVertexBuffer<int>;
	using FloatVertexBuffer = TypedVertexBuffer<float>;
	using Vec2VertexBuffer = TypedVertexBuffer<glm::vec2>;
	using Vec3VertexBuffer = TypedVertexBuffer<glm::vec3>;
	using Vec4VertexBuffer = TypedVertexBuffer<glm::vec4>;

	// TODO:
	// ByteVertexBuffer -> support ByteVertexAttribute
	// DoubleVertexBuffer -> support DoubleVertexAttribute
	// Mat4VertexBuffer
}

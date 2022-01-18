/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "gpubuffer.h"
#include "shadervariabledeclarations.h"
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
	 * A value buffer property
	 * Used exclusively used as a template argument for vertex buffer type definitions
	 */
	enum EValueGPUBufferProperty : uint32
	{
		Generic = 0,		///< Generic value buffer, flags can be set by user
		Vertex = 1,			///< Vertex buffer, enables binding this buffer as a vertex attribute
		Index = 2			///< Index buffer, enables binding this buffer as an index buffer
	};


	/**
	 * GPU buffer base class for storing primitive type elements
	 */
	class NAPAPI ValueGPUBuffer : public GPUBuffer
	{
		RTTI_ENABLE(GPUBuffer)
	public:
		ValueGPUBuffer(Core& core) :
			GPUBuffer(core)
		{ }

		ValueGPUBuffer(Core& core, EMemoryUsage usage) :
			GPUBuffer(core, usage)
		{ }

		/**
		 * Uploads data to the GPU based on the settings provided.
		 * This function automatically allocates GPU memory if required.
		 * @param data pointer to the block of data that needs to be uploaded.
		 * @param the size of the data in bytes
		 * @param errorState contains the error if upload operation failed
		 * @return if upload succeeded
		 */
		virtual bool setData(void* data, size_t size, utility::ErrorState& errorState) = 0;

		/**
		 * Uploads data to the GPU based on the settings provided.
		 * This function automatically allocates GPU memory if required.
		 * @param data pointer to the block of data that needs to be uploaded.
		 * @param elementCount the number of elements
		 * @param reservedElementCount the number of elements to reserve
		 * @param errorState contains the error if upload operation failed
		 * @return if upload succeeded
		 */
		virtual bool setData(void* data, size_t elementCount, size_t reservedElementCount, utility::ErrorState& errorState) = 0;

		/**
		 * @return the buffer format
		 */
		virtual VkFormat getFormat() const = 0;

		uint32											mCount = 0;						///< Property 'Count' The number of vertex elements to initialize/allocate the buffer with
	};


	/**
	 * For more information on buffers on the GPU, refer to: nap::GPUBuffer
	 */
	template<typename T>
	class NAPAPI TypedValueGPUBuffer : public ValueGPUBuffer
	{
		RTTI_ENABLE(ValueGPUBuffer)
	public:
		/**
		 * Every vertex buffer needs to have access to the render engine.
		 * @param renderService the render engine
		 */
		TypedValueGPUBuffer(Core& core) :
			ValueGPUBuffer(core)
		{ }

		/**
		 * Every vertex buffer needs to have access to the render engine.
		 * The given 'usage' controls if a buffer can be updated more than once, and in which memory space it is placed.
		 * The format defines the vertex element size in bytes.
		 * @param renderService the render engine
		 * @param usage how the buffer is used at runtime.
		 */
		TypedValueGPUBuffer(Core& core, EMemoryUsage usage) :
			ValueGPUBuffer(core, usage)
		{ }

		/**
		 * Initialize this buffer. This will allocate all required staging and device buffers based on the buffer properties.
		 * If a fill policy is available, the buffer will also be uploaded to immediately.
		 */
		virtual bool init(utility::ErrorState& errorState) override
		{
			if (!ValueGPUBuffer::init(errorState))
				return false;

			if (!errorState.check(mCount != 0, "Failed to initialize vertex buffer. Cannot allocate a buffer with zero elements."))
				return false;

			// Compose usage flags from buffer configuration
			mUsageFlags |= getBufferUsage(mDescriptorType);

			// Calculate buffer size
			uint32 buffer_size = mCount * sizeof(T);

			// Allocate buffer memory
			if (!allocateInternal(buffer_size, mUsageFlags, errorState))
				return false;

			// Upload data when a buffer fill policy is available
			if (mBufferFillPolicy != nullptr)
			{
				if (mUsage != EMemoryUsage::DynamicRead)
				{
					// Create a staging buffer to upload
					auto staging_buffer = std::make_unique<T[]>(mCount);
					mBufferFillPolicy->fill(mCount, staging_buffer.get());

					// Prepare staging buffer upload
					if (!setDataInternal(staging_buffer.get(), buffer_size, buffer_size, mUsageFlags, errorState))
						return false;
				}
				else
				{
					// Warn user that buffers cannot be filled when their usage is set to DynamicRead
					nap::Logger::warn(utility::stringFormat("%s: The configured fill policy was ignored as the buffer usage is DynamicRead", mID.c_str()).c_str());
				}
			}

			// Optionally clear - does not count as an upload
			else if (mClear)
				GPUBuffer::requestClear();

			mInitialized = true;
			return true;
		}

		/**
		 * Uploads data to the GPU based on the settings provided.
		 * This function automatically allocates GPU memory if required. 
		 * @param data pointer to the block of data that needs to be uploaded.
		 * @param the size of the data in bytes
		 * @param errorState contains the error if upload operation failed
		 * @return if upload succeeded
		 */
		virtual bool setData(void* data, size_t size, utility::ErrorState& errorState) override
		{
			return setDataInternal(data, size, size, mUsageFlags, errorState);
		}

		/**
		 * Uploads data to the GPU based on the settings provided.
		 * This function automatically allocates GPU memory if required.
		 * @param data pointer to the block of data that needs to be uploaded.
		 * @param elementCount the number of elements
		 * @param reservedElementCount the number of elements to reserve
		 * @param errorState contains the error if upload operation failed
		 * @return if upload succeeded
		 */
		virtual bool setData(void* data, size_t elementCount, size_t reservedElementCount, utility::ErrorState& errorState) override
		{
			return setDataInternal(data, sizeof(T) * elementCount, sizeof(T) * reservedElementCount, mUsageFlags, errorState);
		}

		/**
		 * Uploads data to the GPU based on the settings provided.
		 * This function automatically allocates GPU memory if required.
		 * @param data vector containing the data to upload
		 * @param errorState contains the error if upload operation failed
		 * @return if upload succeeded
		 */
		bool setData(const std::vector<T>& data, utility::ErrorState& errorState)
		{
			return setDataInternal(data.data(), data.size() * sizeof(T), data.capacity() * sizeof(T), mUsageFlags, errorState);
		}

		/**
		 * @return the number of buffer values
		 */
		virtual uint getCount() const override							{ return mCount; }

		/**
		 * @return the size of the buffer in bytes
		 */
		virtual size_t getSize() const override							{ return mCount * sizeof(T); };

		/**
		 * @return the size of a single vertex element
		 */
		virtual uint getElementSize() const override					{ return sizeof(T); };

		/**
		 * @return the buffer format
		 */
		virtual VkFormat getFormat() const override						{ return getGPUBufferFormat<T>(); }

		/**
		 * @return the buffer usage flags
		 */
		virtual VkBufferUsageFlags getBufferUsageFlags() const override { return mUsageFlags; }

		/**
		 * @return whether this buffer is initialized
		 */
		virtual bool isInitialized() const override						{ return mInitialized; };

		ResourcePtr<TypedValueBufferFillPolicy<T>>		mBufferFillPolicy = nullptr;	///< Property 'FillPolicy'

	protected:
		// Whether the buffer was successfully initialized
		bool											mInitialized = false;

		// Usage flags that are shared over host (staging) and device (gpu) buffers
		VkBufferUsageFlags								mUsageFlags = 0;
	};


	/**
	 * Definitive GPU value buffer defined by a property
	 */
	template<typename T, EValueGPUBufferProperty PROPERTY>
	class NAPAPI TypedValuePropertyGPUBuffer : public TypedValueGPUBuffer<T>
	{
		RTTI_ENABLE(TypedValueGPUBuffer<T>)
	public:
		/**
		 * Every vertex buffer needs to have access to the render engine.
		 * @param renderService the render engine
		 */
		TypedValuePropertyGPUBuffer(Core& core) :
			TypedValueGPUBuffer<T>(core)
		{ }

		/**
		 * Every vertex buffer needs to have access to the render engine.
		 * The given 'usage' controls if a buffer can be updated more than once, and in which memory space it is placed.
		 * The format defines the vertex element size in bytes.
		 * @param renderService the render engine
		 * @param usage how the buffer is used at runtime.
		 */
		TypedValuePropertyGPUBuffer(Core& core, EMemoryUsage usage) :
			TypedValueGPUBuffer<T>(core, usage)
		{ }

		/**
		 * Initialize this buffer. This will allocate all required staging and device buffers based on the buffer properties.
		 * If a fill policy is available, the buffer will also be uploaded to immediately.
		 */
		virtual bool init(utility::ErrorState& errorState) override
		{
			// Compose usage flags from buffer configuration
			if (PROPERTY == EValueGPUBufferProperty::Index)
				mUsageFlags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

			else if (PROPERTY == EValueGPUBufferProperty::Vertex)
				mUsageFlags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

			if (!TypedValueGPUBuffer<T>::init(errorState))
				return false;

			return true;
		}
	};


	//////////////////////////////////////////////////////////////////////////
	// GPU value buffer type definitions
	//////////////////////////////////////////////////////////////////////////

	using UIntGPUBuffer			= TypedValuePropertyGPUBuffer<uint,			EValueGPUBufferProperty::Generic>;
	using IntGPUBuffer			= TypedValuePropertyGPUBuffer<int,			EValueGPUBufferProperty::Generic>;
	using FloatGPUBuffer		= TypedValuePropertyGPUBuffer<float,		EValueGPUBufferProperty::Generic>;
	using Vec2GPUBuffer			= TypedValuePropertyGPUBuffer<glm::vec2,	EValueGPUBufferProperty::Generic>;
	using Vec3GPUBuffer			= TypedValuePropertyGPUBuffer<glm::vec3,	EValueGPUBufferProperty::Generic>;
	using Vec4GPUBuffer			= TypedValuePropertyGPUBuffer<glm::vec4,	EValueGPUBufferProperty::Generic>;
	using Mat4GPUBuffer			= TypedValuePropertyGPUBuffer<glm::mat4,	EValueGPUBufferProperty::Generic>;
								  											
	using UIntVertexBuffer		= TypedValuePropertyGPUBuffer<uint,			EValueGPUBufferProperty::Vertex>;
	using IntVertexBuffer		= TypedValuePropertyGPUBuffer<int,			EValueGPUBufferProperty::Vertex>;
	using FloatVertexBuffer		= TypedValuePropertyGPUBuffer<float,		EValueGPUBufferProperty::Vertex>;
	using Vec2VertexBuffer		= TypedValuePropertyGPUBuffer<glm::vec2,	EValueGPUBufferProperty::Vertex>;
	using Vec3VertexBuffer		= TypedValuePropertyGPUBuffer<glm::vec3,	EValueGPUBufferProperty::Vertex>;
	using Vec4VertexBuffer		= TypedValuePropertyGPUBuffer<glm::vec4,	EValueGPUBufferProperty::Vertex>;
								  											
	using IndexBuffer			= TypedValuePropertyGPUBuffer<uint,			EValueGPUBufferProperty::Index>;
}

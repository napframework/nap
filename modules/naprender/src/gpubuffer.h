/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "basegpubuffer.h"
#include "vulkan/vulkan_core.h"
#include "fillpolicy.h"
#include "formatutils.h"

// External Includes
#include <nap/resourceptr.h>
#include <glm/glm.hpp>
#include <nap/logger.h>

namespace nap
{
	/**
	 * Base class for all types of one dimensional GPU buffers.
	 * Supported values for child classes such as GPUBufferNumeric<T> must be primitives that can be mapped to 
	 * VkFormat. This is enforced by the requirement to implement getFormat().
	 */
	class NAPAPI GPUBuffer : public BaseGPUBuffer
	{
		RTTI_ENABLE(BaseGPUBuffer)
	public:
		GPUBuffer(Core& core) :
			BaseGPUBuffer(core)
		{ }

		GPUBuffer(Core& core, EMemoryUsage usage) :
			BaseGPUBuffer(core, usage)
		{ }

		/**
		 * Uploads data to the GPU based on the settings provided.
		 * This function automatically allocates GPU memory if required.
		 * @param data pointer to the block of data that needs to be uploaded.
		 * @param elementCount the number of elements
		 * @param reservedElementCount the number of elements to reserve
		 * @param errorState contains the error if upload operation failed
		 * @return if upload succeeded
		 */
		virtual bool setData(const void* data, size_t elementCount, size_t reservedElementCount, utility::ErrorState& errorState) = 0;

		/**
		 * @return the buffer format
		 */
		virtual VkFormat getFormat() const = 0;

		uint32 mCount = 0;				///< Property: 'Count' The number of  elements to initialize/allocate the buffer with.
	};


	//////////////////////////////////////////////////////////////////////////
	// Numeric GPU Buffer
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Typed class for GPU numeric buffers.
	 *
	 * Allocates all required host (staging) and device buffers based on the specified properties.
	 * If a 'FillPolicy' is available, the buffer will also be uploaded to immediately. Alternatively, 'Clear' sets all
	 * of the buffer values to zero on init(). 'FillPolicy' and 'Clear' are mutually exclusive and the former has
	 * priority over the latter.
	 * 
	 * Supported types are primitive types that can be mapped to VkFormat.
	 * @tparam T primitive value data type
	 */
	template<typename T>
	class GPUBufferNumeric : public GPUBuffer
	{
		RTTI_ENABLE(GPUBuffer)
	public:
		/**
		 * Every numeric buffer needs to have access to the render engine.
		 * @param renderService the render engine
		 */
		GPUBufferNumeric(Core& core) :
			GPUBuffer(core)
		{ }

		/**
		 * Every numeric buffer needs to have access to the render engine.
		 * The given 'usage' controls if a buffer can be updated more than once, and in which memory space it is placed.
		 * The format defines the vertex element size in bytes.
		 * @param renderService the render engine
		 * @param usage how the buffer is used at runtime.
		 */
		GPUBufferNumeric(Core& core, EMemoryUsage usage) :
			GPUBuffer(core, usage)
		{ }

		/**
		 * Initialize this buffer. This will allocate all required staging and device buffers based on the specified properties.
		 * If a 'FillPolicy' is available, the buffer will also be uploaded to immediately. Alternatively, 'Clear' sets all
		 * of the buffer values to zero on init(). 'FillPolicy' and 'Clear' are mutually exclusive and the former has priority
		 * over the latter.
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Uploads data to the GPU based on the settings provided.
		 * This function automatically allocates GPU memory if required.
		 * @param data pointer to the block of data that needs to be uploaded.
		 * @param elementCount the number of elements
		 * @param reservedElementCount the number of elements to reserve
		 * @param errorState contains the error if upload operation failed
		 * @return if upload succeeded
		 */
		virtual bool setData(const void* data, size_t elementCount, size_t reservedElementCount, utility::ErrorState& errorState) override;

		/**
		 * Uploads data to the GPU based on the settings provided.
		 * This function automatically allocates GPU memory if required.
		 * @param data vector containing the data to upload
		 * @param errorState contains the error if upload operation failed
		 * @return if upload succeeded
		 */
		bool setData(const std::vector<T>& data, utility::ErrorState& errorState);

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
		virtual VkFormat getFormat() const override						{ return getVulkanFormat<T>(); }

		/**
		 * @return whether this buffer is initialized
		 */
		virtual bool isInitialized() const override						{ return mInitialized; };

		ResourcePtr<FillPolicy<T>>						mFillPolicy = nullptr;	///< Property 'FillPolicy' Optional fill policy to fill the buffer with on initialization

	protected:
		// Whether the buffer was successfully initialized
		bool											mInitialized = false;
	};


	//////////////////////////////////////////////////////////////////////////
	// Vertex Buffers
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Vertex GPU buffers.
	 *
	 * Allocates all required host (staging) and device buffers based on the specified properties.
	 * If a 'FillPolicy' is available, the buffer will also be uploaded to immediately. Alternatively, 'Clear' sets all
	 * of the buffer values to zero on init(). 'FillPolicy' and 'Clear' are mutually exclusive and the former has
	 * priority over the latter.
	 *
	 * In addition to nap::GPUBufferNumeric, this class distinguishes vertex buffers from general purpose numeric buffers.
	 * Internally, some flags are stored that help the driver identify and optimize buffers that have a specific purpose in a rendering operation.
	 * They also play a role in synchronization of compute and graphics operations.
	 *
	 * Supported types are primitive types that can be mapped to VkFormat.
	 * @tparam T primitive value data type
	 */
	template<typename T>
	class VertexBuffer final : public GPUBufferNumeric<T>
	{
		RTTI_ENABLE(GPUBufferNumeric<T>)
	public:
		/**
		 * Every vertex buffer needs to have access to the render engine.
		 * @param renderService the render engine
		 */
		VertexBuffer(Core& core) :
			GPUBufferNumeric<T>(core)
		{ }

		/**
		 * Every vertex buffer needs to have access to the render engine.
		 * The given 'usage' controls if a buffer can be updated more than once, and in which memory space it is placed.
		 * The format defines the vertex element size in bytes.
		 * @param renderService the render engine
		 * @param usage how the buffer is used at runtime.
		 * @param if the buffer is allowed to be bound to a shader as a storage buffer using a descriptor
		 */
		VertexBuffer(Core& core, EMemoryUsage usage, bool storage) :
			GPUBufferNumeric<T>(core, usage), mStorage(storage)
		{ }

		/**
		 * Initialize this buffer. This will allocate all required staging and device buffers based on the buffer properties.
		 * If a fill policy is available, the buffer will also be uploaded to immediately.
		 */
		virtual bool init(utility::ErrorState& errorState) override;

	private:
		bool mStorage = true;			///< Property: Allows the buffer to be bound to a shader as a storage buffer using a descriptor, allowing it be read and set from a shader program.
	};


	//////////////////////////////////////////////////////////////////////////
	// Index Buffer
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Index GPU buffer.
	 *
	 * Allocates all required host (staging) and device buffers based on the specified properties.
	 * If a 'FillPolicy' is available, the buffer will also be uploaded to immediately. Alternatively, 'Clear' sets all
	 * of the buffer values to zero on init(). 'FillPolicy' and 'Clear' are mutually exclusive and the former has
	 * priority over the latter.
	 *
	 * In addition to nap::GPUBufferNumeric, this class distinguishes index buffers from general purpose numeric buffers.
	 * Internally, some flags are stored that help the driver identify and optimize buffers that have a specific purpose in a rendering operation.
	 *
	 * Supported types are primitive types that can be mapped to VkFormat.
	 * @tparam T primitive value data type
	 */
	class NAPAPI IndexBuffer final : public GPUBufferNumeric<uint>
	{
		RTTI_ENABLE(GPUBufferNumeric<uint>)
	public:
		/**
		 * Every index buffer needs to have access to the render engine.
		 * @param renderService the render engine
		 */
		IndexBuffer(Core & core) : GPUBufferNumeric<uint>(core)			{ }

		/**
		 * Every index buffer needs to have access to the render engine.
		 * The given 'usage' controls if a buffer can be updated more than once, and in which memory space it is placed.
		 * The format defines the vertex element size in bytes.
		 * @param renderService the render engine
		 * @param usage how the buffer is used at runtime.
		 * @param storage if the buffer is allowed to be bound to a shader as a storage buffer using a descriptor
		 */
		IndexBuffer(Core & core, EMemoryUsage usage, bool storage) :
			GPUBufferNumeric<uint>(core, usage), mStorage(storage)		{ }

		/**
		 * Initialize this buffer. This will allocate all required staging and device buffers based on the buffer properties.
		 * If a fill policy is available, the buffer will also be uploaded to immediately.
		 */
		virtual bool init(utility::ErrorState & errorState) override;

	private:
		bool mStorage = true;			///< Allows the buffer to be bound to a shader as a storage buffer using a descriptor, allowing it be read and set from a shader program.
	};


	//////////////////////////////////////////////////////////////////////////
	// GPU numeric buffer type definitions
	//////////////////////////////////////////////////////////////////////////

	// General purpose GPU buffers
	using GPUBufferUInt			= GPUBufferNumeric<uint>;
	using GPUBufferInt			= GPUBufferNumeric<int>;
	using GPUBufferFloat		= GPUBufferNumeric<float>;
	using GPUBufferVec2			= GPUBufferNumeric<glm::vec2>;
	using GPUBufferVec3			= GPUBufferNumeric<glm::vec3>;
	using GPUBufferVec4			= GPUBufferNumeric<glm::vec4>;
	using GPUBufferMat4			= GPUBufferNumeric<glm::mat4>;

	// Vertex GPU buffers
	using VertexBufferUInt		= VertexBuffer<uint>;
	using VertexBufferInt		= VertexBuffer<int>;
	using VertexBufferFloat		= VertexBuffer<float>;
	using VertexBufferVec2		= VertexBuffer<glm::vec2>;
	using VertexBufferVec3		= VertexBuffer<glm::vec3>;
	using VertexBufferVec4		= VertexBuffer<glm::vec4>;


	//////////////////////////////////////////////////////////////////////////
	// Template Definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename T>
	bool GPUBufferNumeric<T>::init(utility::ErrorState& errorState)
	{
		if (!GPUBuffer::init(errorState))
			return false;

		if (!errorState.check(mMemoryUsage != EMemoryUsage::DynamicWrite || mCount >= 0,
			"Cannot allocate a non-DynamicWrite buffer with zero elements."))
			return false;

		// Calculate buffer size
		uint32 buffer_size = mCount * sizeof(T);

		// Allocate buffer memory
		if (!allocateInternal(buffer_size, errorState))
			return false;

		// Upload data when a buffer fill policy is available
		if (mFillPolicy != nullptr)
		{
			if (mMemoryUsage != EMemoryUsage::DynamicRead)
			{
				// Create a staging buffer to upload
				auto staging_buffer = std::make_unique<T[]>(mCount);
				mFillPolicy->fill(mCount, staging_buffer.get());

				// Prepare staging buffer upload
				if (!setDataInternal(staging_buffer.get(), buffer_size, buffer_size, errorState))
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
			BaseGPUBuffer::requestClear();

		mInitialized = true;
		return true;
	}

	template<typename T>
	bool GPUBufferNumeric<T>::setData(const void* data, size_t elementCount, size_t reservedElementCount, utility::ErrorState& errorState)
	{
		if (!setDataInternal(data, sizeof(T) * elementCount, sizeof(T) * reservedElementCount, errorState))
			return false;

		// Update count
		mCount = elementCount;
		return true;
	}

	template<typename T>
	bool GPUBufferNumeric<T>::setData(const std::vector<T>& data, utility::ErrorState& errorState)
	{
		if (!setDataInternal(data.data(), data.size() * sizeof(T), data.capacity() * sizeof(T), errorState))
			return false;

		// Update count
		mCount = data.size();
		return true;
	}

	template<typename T>
	bool VertexBuffer<T>::init(utility::ErrorState& errorState)
	{
		VkBufferUsageFlags req_usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		req_usage |= mStorage ? VK_BUFFER_USAGE_STORAGE_BUFFER_BIT : 0;
		GPUBufferNumeric<T>::ensureUsage(req_usage);
		return GPUBufferNumeric<T>::init(errorState);
	}
}

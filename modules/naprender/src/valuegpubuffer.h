/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "basegpubuffer.h"
#include "vulkan/vulkan_core.h"
#include "valuebufferfillpolicy.h"
#include "formatutils.h"

// External Includes
#include <nap/resourceptr.h>
#include <glm/glm.hpp>

namespace nap
{
	/**
	 * Base class for all types of one dimensional GPU buffers.
	 * Supported values for child classes such as TypedValueGPUBuffer<T> must be primitives that can be mapped to 
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
		virtual bool setData(void* data, size_t elementCount, size_t reservedElementCount, utility::ErrorState& errorState) = 0;

		/**
		 * @return the buffer format
		 */
		virtual VkFormat getFormat() const = 0;

		uint32	mCount = 0;		///< Property 'Count' The number of vertex elements to initialize/allocate the buffer with
	};


	//////////////////////////////////////////////////////////////////////////
	// Numeric GPU Buffer
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Typed class for GPU value buffers.
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
	class NAPAPI GPUBufferNumeric : public GPUBuffer
	{
		RTTI_ENABLE(GPUBuffer)
	public:
		/**
		 * Every value buffer needs to have access to the render engine.
		 * @param renderService the render engine
		 */
		GPUBufferNumeric(Core& core) :
			GPUBuffer(core)
		{ }

		/**
		 * Every value buffer needs to have access to the render engine.
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
		virtual bool setData(void* data, size_t elementCount, size_t reservedElementCount, utility::ErrorState& errorState) override
		{
			if (!setDataInternal(data, sizeof(T) * elementCount, sizeof(T) * reservedElementCount, mUsageFlags, errorState))
				return false;

			// Update count
			mCount = elementCount;
			return true;
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
			if (!setDataInternal(data.data(), data.size() * sizeof(T), data.capacity() * sizeof(T), mUsageFlags, errorState))
				return false;

			// Update count
			mCount = data.size();
			return true;
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

		ResourcePtr<TypedValueBufferFillPolicy<T>>		mBufferFillPolicy = nullptr;	///< Property 'FillPolicy' Optional fill policy to fill the buffer with on initialization

	protected:
		// Whether the buffer was successfully initialized
		bool											mInitialized = false;

		// Usage flags that are shared over host (staging) and device (gpu) buffers
		VkBufferUsageFlags								mUsageFlags = 0;
	};


	//////////////////////////////////////////////////////////////////////////
	// Vertex Buffers
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Definitive typed class for GPU value buffers.
	 *
	 * Allocates all required host (staging) and device buffers based on the specified properties.
	 * If a 'FillPolicy' is available, the buffer will also be uploaded to immediately. Alternatively, 'Clear' sets all
	 * of the buffer values to zero on init(). 'FillPolicy' and 'Clear' are mutually exclusive and the former has
	 * priority over the latter.
	 *
	 * In addition to nap::TypedValueGPUBuffer, this class distinguishes specialized purpose vertex and index buffers
	 * from general purpose value buffers. Internally, some flags are stored that help the driver identify and optimize
	 * buffers that have a specific purpose in a rendering operation. They also play a role in synchronization of compute
	 * and graphics operations.
	 *
	 * Supported types are primitive types that can be mapped to VkFormat.
	 * @tparam T primitive value data type
	 * @tparam PROPERTY property for identifying the buffer usage and access type
	 */
	template<typename T>
	class VertexBuffer final : public GPUBufferNumeric<T>
	{
		RTTI_ENABLE(GPUBufferNumeric<T>)
	public:
		/**
		 * Every value buffer needs to have access to the render engine.
		 * @param renderService the render engine
		 */
		VertexBuffer(Core& core) :
			GPUBufferNumeric<T>(core)
		{ }

		/**
		 * Every value buffer needs to have access to the render engine.
		 * The given 'usage' controls if a buffer can be updated more than once, and in which memory space it is placed.
		 * The format defines the vertex element size in bytes.
		 * @param renderService the render engine
		 * @param usage how the buffer is used at runtime.
		 */
		VertexBuffer(Core& core, EMemoryUsage usage) :
			GPUBufferNumeric<T>(core, usage)
		{ }

		/**
		 * Initialize this buffer. This will allocate all required staging and device buffers based on the buffer properties.
		 * If a fill policy is available, the buffer will also be uploaded to immediately.
		 */
		virtual bool init(utility::ErrorState& errorState) override;
	};


	//////////////////////////////////////////////////////////////////////////
	// Index Buffer
	//////////////////////////////////////////////////////////////////////////

	class NAPAPI IndexBuffer final : public GPUBufferNumeric<uint>
	{
		RTTI_ENABLE(GPUBufferNumeric<uint>)
	public:
		/**
		 * Every value buffer needs to have access to the render engine.
		 * @param renderService the render engine
		 */
		IndexBuffer(Core & core) : GPUBufferNumeric<uint>(core)		{ }

		/**
		 * Every value buffer needs to have access to the render engine.
		 * The given 'usage' controls if a buffer can be updated more than once, and in which memory space it is placed.
		 * The format defines the vertex element size in bytes.
		 * @param renderService the render engine
		 * @param usage how the buffer is used at runtime.
		 */
		IndexBuffer(Core & core, EMemoryUsage usage) :
			GPUBufferNumeric<uint>(core, usage)						{ }

		/**
		 * Initialize this buffer. This will allocate all required staging and device buffers based on the buffer properties.
		 * If a fill policy is available, the buffer will also be uploaded to immediately.
		 */
		virtual bool init(utility::ErrorState & errorState) override;
	};


	//////////////////////////////////////////////////////////////////////////
	// GPU value buffer type definitions
	//////////////////////////////////////////////////////////////////////////

	// General purpose GPU buffers
	using UIntGPUBuffer			= GPUBufferNumeric<uint>;
	using IntGPUBuffer			= GPUBufferNumeric<int>;
	using FloatGPUBuffer		= GPUBufferNumeric<float>;
	using Vec2GPUBuffer			= GPUBufferNumeric<glm::vec2>;
	using Vec3GPUBuffer			= GPUBufferNumeric<glm::vec3>;
	using Vec4GPUBuffer			= GPUBufferNumeric<glm::vec4>;
	using Mat4GPUBuffer			= GPUBufferNumeric<glm::mat4>;

	// Vertex GPU buffers
	using UIntVertexBuffer		= VertexBuffer<uint>;
	using IntVertexBuffer		= VertexBuffer<int>;
	using FloatVertexBuffer		= VertexBuffer<float>;
	using Vec2VertexBuffer		= VertexBuffer<glm::vec2>;
	using Vec3VertexBuffer		= VertexBuffer<glm::vec3>;
	using Vec4VertexBuffer		= VertexBuffer<glm::vec4>;


	//////////////////////////////////////////////////////////////////////////
	// Template Definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename T>
	bool VertexBuffer<T>::init(utility::ErrorState& errorState)
	{
		GPUBufferNumeric<T>::mUsageFlags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		return GPUBufferNumeric<T>::init(errorState);
	}
}

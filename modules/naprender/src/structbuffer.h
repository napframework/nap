/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "gpubuffer.h"
#include "uniform.h"
#include "structfillpolicy.h"

// External Includes
#include <nap/resourceptr.h>
#include <glm/glm.hpp>

namespace nap
{
	/**
	 * GPU Buffer for storing (nested) data structures.
	 *
	 * Allocates all required host (staging) and device buffers based on the specified properties.
	 *
	 * The layout of the struct buffer is defined by a StructBufferDescriptor. This descriptor is used to denote the 
	 * buffer layout, allocate the right amount of memory, and possibly store information on how fill the buffer
	 * accordingly. As the size of the buffer is determined by the descriptor, it is important that it matches the
	 * declaration of the shader variable it is bound to. Initialization of buffer bindings may fail otherwise.
	 * 
	 * If a 'FillPolicy' is available, the buffer will also be uploaded to immediately. Alternatively, 'Clear' sets all
	 * of the buffer values to zero on init(). 'FillPolicy' and 'Clear' are mutually exclusive and the former has
	 * priority over the latter.
	 *
	 * Supported types are primitive types that can be mapped to VkFormat.
	 * @tparam T primitive value data type
	 * @tparam PROPERTY property for identifying the buffer usage and access type
	 */
	class NAPAPI StructBuffer final : public BaseGPUBuffer
	{
		RTTI_ENABLE(BaseGPUBuffer)
	public:
		StructBuffer(Core& core) :
			BaseGPUBuffer(core)
		{ }

		StructBuffer(Core& core, EMemoryUsage usage) :
			BaseGPUBuffer(core, usage)
		{ }

		/**
		 * Initialize this buffer. This will allocate all required staging and device buffers based on the buffer properties.
		 * If a fill policy is available, the buffer will also be uploaded to immediately.
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * @return the number of elements in the buffer
		 */
		virtual uint getCount() const override							{ return mDescriptor.mCount; };

		/**
		 * @return the size of the buffer in bytes
		 */
		virtual size_t getSize() const override							{ return mDescriptor.mCount * mElementSize; };

		/**
		 * @return the element size in bytes
		 */
		virtual uint getElementSize() const override					{ return mElementSize; };

		/**
		 * @return whether this buffer is initialized
		 */
		virtual bool isInitialized() const override						{ return mInitialized; };

		/**
		 * Uploads data to the GPU based on the settings provided.
		 * This function automatically allocates GPU memory if required.
		 * @param data pointer to the block of data that needs to be uploaded.
		 * @param the size of the data in bytes
		 * @param error contains the error if upload operation failed
		 * @return if upload succeeded
		 */
		bool setData(void* data, size_t size, utility::ErrorState& error);

		ResourcePtr<StructFillPolicy>				mFillPolicy = nullptr;							///< Property 'FillPolicy' Optional fill policy to fill the buffer with on initialization
		StructBufferDescriptor						mDescriptor;									///< Property 'Descriptor' The descriptor that defines the buffer layout

	private:
		// Whether the buffer was successfully initialized
		bool mInitialized = false;

		// Cached element size
		uint mElementSize = 0;
	};
}

/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "gpubuffer.h"
#include "uniform.h"
#include "structbufferfillpolicy.h"

// External Includes
#include <nap/resourceptr.h>
#include <glm/glm.hpp>

namespace nap
{
	/**
	 * For more information on buffers on the GPU, refer to: nap::GPUBuffer
	 */
	class NAPAPI GPUStructBuffer : public GPUBuffer
	{
		RTTI_ENABLE(GPUBuffer)
	public:
		GPUStructBuffer(Core& core) :
			GPUBuffer(core)
		{ }

		GPUStructBuffer(Core& core, EMeshDataUsage usage) :
			GPUBuffer(core, usage)
		{ }

		/**
		 * Init
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Uploads data to the GPU based on the settings provided.
		 * This function automatically allocates GPU memory if required. 
		 * @param data pointer to the block of data that needs to be uploaded.
		 * @param the size of the data in bytes
		 * @param error contains the error if upload operation failed
		 * @return if upload succeeded
		 */
		bool setData(void* data, size_t size, utility::ErrorState& error);

		/**
		 * @return the size of the buffer in bytes
		 */
		size_t getSize() const						{ return mDescriptor.mCount * mElementSize; };

		/**
		 * @return the number of elements in the buffer
		 */
		int getCount() const						{ return mDescriptor.mCount; };

		/**
		 * @return the element size in bytes
		 */
		int getElementSize() const					{ return mElementSize; };

		ResourcePtr<BaseStructBufferFillPolicy>		mFillPolicy = nullptr;							///< Property 'FillPolicy'
		StructBufferDescriptor						mDescriptor;									///< Property 'Descriptor'

	private:
		int	mElementSize = -1;
	};
}

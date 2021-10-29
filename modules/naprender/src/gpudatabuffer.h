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
	class NAPAPI GPUDataBuffer : public BaseGPUBuffer
	{
	public:
		/**
		 * 
		 */
		GPUDataBuffer(RenderService& renderService, EBufferObjectType type);

		~GPUDataBuffer();

		// Copy construction not allowed
		GPUDataBuffer(const GPUDataBuffer& other) = delete;

		// Copy assignment not allowed
		GPUDataBuffer& operator=(const GPUBuffer& other) = delete;

		/**
		 * 
		 */
		bool setData(uint32 size, uint8* data, utility::ErrorState& errorState);


		/**
		 *
		 */
		bool getData(BufferData& outBufferData, utility::ErrorState& errorState);

		/**
		 * 
		 */
		virtual VkBuffer getBuffer() const override { return (mDeviceBuffer != nullptr) ? mDeviceBuffer->mBuffer : VK_NULL_HANDLE; }

		/**
		 * Called right after the buffer on the GPU has been updated.
		 */
		nap::Signal<> bufferChanged;

		EBufferObjectType mType	= EBufferObjectType::Uniform;		///< Property 'Type'

	private:
		/**
		 *
		 */
		virtual void upload(VkCommandBuffer commandBuffer) override;

		RenderService*			mRenderService = nullptr;			///< Handle to the render service
		BufferData*				mDeviceBuffer;						///< Render accessible buffers
		BufferData				mStagingBuffer;						///< Staging buffer, used when uploading static mesh geometry
		uint32					mSize = 0;							///< Current used buffer size in bytes
	};
}

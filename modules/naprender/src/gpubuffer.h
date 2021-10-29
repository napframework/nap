/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "vk_mem_alloc.h"
#include "renderutils.h"

// External Includes
#include <nap/resource.h>
#include <vulkan/vulkan_core.h>
#include <utility/dllexport.h>
#include <nap/signalslot.h>
#include <vector>
#include <utility/errorstate.h>
#include <nap/numeric.h>

namespace nap
{
	class RenderService;
	class Core;

	/**
	 * Flag that determines how the mesh data is used at runtime.
	 * A static mesh is uploaded from the CPU to the GPU exactly once. 
	 * This allows the system to remove unused buffers after the upload is complete. 
	 * If there is the need to update a mesh more frequently, even once after upload, 
	 * it is required the usage is set to 'DynamicWrite'.
	 * 
	 * Note that static meshes are often placed in a different cache on the GPU, not accessible by the CPU, which
	 * allows for faster drawing times. 'DynamicWrite' meshes are uploaded into shared CPU / GPU memory 
	 * and are therefore slower to draw. Keep this in mind when selecting the appropriate data use.
	 */
	enum class EMeshDataUsage
	{
		Static,				///< Mesh data is uploaded only once from the CPU to the GPU
		DynamicWrite,		///< Mesh data is updated more than once from the CPU to the GPU
	};


	/**
	 * Base GPU Buffer
	 */
	class NAPAPI BaseGPUBuffer
	{
		friend class RenderService;
	public:
		virtual void upload(VkCommandBuffer commandBuffer) = 0;

		virtual VkBuffer getBuffer() const = 0;
	};


	/**
	 * Defines a Vulkan buffer object on the GPU.
	 *
	 * A static buffer is updated (uploaded to) only once, a dynamic buffer can be updated more frequently but
	 * requires more resources and is 'generally' slower to draw, depending of the memory layout of the underlying hardware.
	 * It is not allowed to update a static buffer after the initial upload!
	 *
	 * Note that static meshes are often placed in a different cache on the GPU, not accessible by the CPU, which
	 * allows for faster drawing times. 'DynamicWrite' meshes are uploaded into shared CPU / GPU memory
	 * and are therefore slower to draw.
	 */
	class NAPAPI GPUBuffer : public Resource, public BaseGPUBuffer
	{
		friend class RenderService;
		RTTI_ENABLE(Resource)
	public:

		/**
		 * Every buffer needs to have access to the render engine.
		 * The given 'usage' controls if a buffer can be updated more than once 
		 * and in which memory space it is placed.
		 * @param core the nap core
		 */
		GPUBuffer(Core& core);

		/**
		 * Every buffer needs to have access to the render engine.
		 * The given 'usage' controls if a buffer can be updated more than once
		 * and in which memory space it is placed.
		 * @param core the nap core
		 * @param usage how the buffer is used at runtime.
		 */
		GPUBuffer(Core& core, EMeshDataUsage usage);

		virtual ~GPUBuffer();

		// Copy construction not allowed
		GPUBuffer(const GPUBuffer& other) = delete;
		
		// Copy assignment not allowed
		GPUBuffer& operator=(const GPUBuffer& other) = delete;

		/**
		 * @return handle to the Vulkan buffer.
		 */
		virtual VkBuffer getBuffer() const override;

		/**
		 * @return handle to the buffer data.
		 */
		const BufferData& getBufferData() const;

		/**
		 * Called right after the buffer on the GPU has been updated.
		 */
		nap::Signal<> bufferChanged;

		/**
		 * 
		 */
		bool init(utility::ErrorState& errorState) override;

		EMeshDataUsage			mUsage = EMeshDataUsage::Static;	///< Property 'Usage' How the buffer is used, static, updated frequently etc.

	protected:
		/**
		 * Updates GPU buffer content, called by derived classes.
		 * @param data pointer to the data to upload.
		 * @param elementSize size in bytes of the element to upload
		 * @param numVertices the number of vertices to upload, data should be: numVertices * elementSize.
		 * @param reservedNumVertices needs to be >= numVertices, allows the buffer to allocate more memory then required
		 * @param usage how the data is used at runtime
		 * @param error contains error when data could not be set.
		 * @return if the data was set
		 */
		bool setDataInternal(void* data, int elementSize, size_t numVertices, size_t reservedNumVertices, VkBufferUsageFlagBits usage, utility::ErrorState& error);

		bool setDataInternal(void* data, size_t size, VkBufferUsageFlagBits usage, utility::ErrorState& error);

		RenderService*			mRenderService = nullptr;			///< Handle to the render service
		std::vector<BufferData>	mRenderBuffers;						///< Render accessible buffers
		BufferData				mStagingBuffer;						///< Staging buffer, used when uploading static mesh geometry
		uint32					mSize = 0;							///< Current used buffer size in bytes
		int						mCurrentBufferIndex = 0;			///< Current render buffer index

	private:
		// Called when usage = static
		bool setDataInternalStatic(void* data, size_t size, VkBufferUsageFlagBits usage, utility::ErrorState& error);
		
		// Called when usage = dynamic write
		bool setDataInternalDynamic(void* data, size_t size, size_t reservedSize, VkBufferUsageFlagBits usage, utility::ErrorState& error);

		// Uploads data from the staging buffer into GPU buffer. Automatically called by the render service at the appropriate time.
		// Only occurs when 'usage' = 'static'. Dynamic data shares GPU / CPU memory and is updated immediately.
		virtual void upload(VkCommandBuffer commandBuffer) override;
	};

}

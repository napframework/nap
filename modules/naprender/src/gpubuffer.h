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
	enum class EMeshDataUsage : uint
	{
		Static,				///< Mesh data is uploaded only once from the CPU to the GPU
		DynamicRead,		///< Mesh data is uploaded only once from the CPU to the GPU, and frequently read from GPU to CPU
		DynamicWrite		///< Mesh data is updated more than once from the CPU to the GPU
	};


	/**
	 * Flag that determines the type of shader resource.
	 * TODO: elaborate
	 */
	enum class EDescriptorType : uint
	{
		None,				///< none
		Uniform,			///< specifies a uniform buffer descriptor. device readonly
		Storage				///< specifies a storage buffer descriptor. device read/write
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
	class NAPAPI GPUBuffer : public Resource
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
		virtual VkBuffer getBuffer() const;

		/**
		 * @return handle to the buffer data.
		 */
		virtual const BufferData& getBufferData() const;

		/**
		 * Initialize the buffer
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Implemented by derived classes
		 * @return whether this buffer is initialized
		 */
		virtual bool isInitialized() const = 0;

		/**
		 * Starts a transfer of buffer data from GPU to CPU. Use this overload to pass your own copy function. This is a non blocking call.
		 * @param copyFunction the copy function to call when the buffer data is available for download.
		 */
		void asyncGetData(std::function<void(const void*, size_t)> copyFunction);

		/**
		 * Called right after the buffer on the GPU has been updated.
		 */
		nap::Signal<> bufferChanged;

		EMeshDataUsage			mUsage = EMeshDataUsage::Static;				///< Property 'Usage' How the buffer is used, static, updated frequently etc.
		EDescriptorType			mDescriptorType = EDescriptorType::None;		///< Property 'DescriptorType' How the buffer is used on the device (uniform = readonly, storage = readwrite)

	protected:

		/**
		 * Allocates buffers, called by derived classes
		 * @param size size in bytes of the buffer to allocate
		 * @param deviceUsage how the data is used at runtime on the device (e.g. VERTEX, INDEX, UNIFORM, STORAGE)
		 * @param errorState contains error when data could not be set.
		 * @return if the data was set
		 */
		bool allocateInternal(size_t size, VkBufferUsageFlags deviceUsage, utility::ErrorState& errorState);

		/**
		 * Allocates buffers, called by derived classes
		 * @param elementSize size in bytes of a single element
		 * @param numVertices the number of vertices to allocate, data should be: numVertices * elementSize.
		 * @param reservedNumVertices needs to be >= numVertices, allows the buffer to allocate more memory than required
		 * @param deviceUsage how the data is used at runtime on the device (e.g. VERTEX, INDEX, UNIFORM, STORAGE)
		 * @param errorState contains error when data could not be set.
		 * @return if the data was set
		 */
		bool allocateInternal(int elementSize, size_t numVertices, VkBufferUsageFlags deviceUsage, utility::ErrorState& errorState);

		/**
		 * Allocates and updates GPU buffer content, called by derived classes.
		 * @param data pointer to the data to upload.
		 * @param elementSize size in bytes of the element to upload
		 * @param numVertices the number of vertices to upload, data should be: numVertices * elementSize.
		 * @param reservedNumVertices needs to be >= numVertices, allows the buffer to allocate more memory than required
		 * @param deviceUsage how the data is used at runtime on the device (e.g. VERTEX, INDEX, UNIFORM, STORAGE)
		 * @param errorState contains error when data could not be set.
		 * @return if the data was set
		 */
		bool setDataInternal(void* data, int elementSize, size_t numVertices, size_t reservedNumVertices, VkBufferUsageFlags deviceUsage, utility::ErrorState& errorState);

		/**
		 * Allocates and updates GPU buffer content, called by derived classes.
		 * @param data pointer to the data to upload.
		 * @param size size in bytes of the data to upload
		 * @param reservedSize allows the buffer to allocate more memory than required, needs to be >= size
		 * @param deviceUsage how the data is used at runtime on the device (e.g. VERTEX, INDEX, UNIFORM, STORAGE)
		 * @param errorState contains error when data could not be set.
		 * @return if the data was set
		 */
		bool setDataInternal(void* data, size_t size, size_t reservedSize, VkBufferUsageFlags deviceUsage, utility::ErrorState& errorState);

		RenderService*			mRenderService = nullptr;			///< Handle to the render service
		std::vector<BufferData>	mRenderBuffers;						///< Render accessible buffers
		std::vector<BufferData>	mStagingBuffers;					///< Staging buffers, used when uploading or downloading data
		uint32					mSize = 0;							///< Current used buffer size in bytes

		int						mCurrentRenderBufferIndex = 0;		///< Current render buffer index
		int						mCurrentStagingBufferIndex = 0;		///< Current staging buffer index
		std::vector<int>		mDownloadStagingBufferIndices;		///< Staging buffer indices associated with a frameindex

	private:
		using BufferReadCallback = std::function<void(void* data, size_t sizeInBytes)>;

		// Called when usage = static
		bool setDataInternalStatic(void* data, size_t size, utility::ErrorState& errorState);
		
		// Called when usage = dynamic write
		bool setDataInternalDynamic(void* data, size_t size, size_t reservedSize, VkBufferUsageFlags deviceUsage, utility::ErrorState& errorState);

		// Uploads data from the staging buffer into GPU buffer. Automatically called by the render service at the appropriate time.
		// Only occurs when 'usage' = 'static'. Dynamic data shares GPU / CPU memory and is updated immediately.
		void upload(VkCommandBuffer commandBuffer);

		// Downloads data from GPU buffer to staging buffer
		void download(VkCommandBuffer commandBuffer);

		// Called by the render service when download is ready
		void notifyDownloadReady(int frameIndex);

		// Clears queued texture downloads
		void clearDownloads();

		std::vector<BufferReadCallback>	mReadCallbacks;				///< Number of callbacks based on number of frames in flight
	};
}

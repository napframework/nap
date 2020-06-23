#pragma once

// Local Includes
#include "vk_mem_alloc.h"

// External Includes
#include <vulkan/vulkan_core.h>
#include <utility/dllexport.h>
#include <nap/signalslot.h>
#include <vector>
#include <utility/errorstate.h>
#include <nap/numeric.h>

namespace nap
{
	class RenderService;

	/**
	 * Flag that determines how the mesh data is used at runtime.
	 */
	enum class EMeshDataUsage
	{
		Static,				///< Data of the mesh does not change
		DynamicWrite,		///< Data of the mesh is frequently read from GPU to CPU
	};

	/**
	 * Defines a buffer object on the GPU and acts as a base class for 
	 * all Vulkan derived buffer types. This object creation / destruction 
	 * as well as the internal buffer type.
	 */
	class NAPAPI GPUBuffer
	{
		friend class RenderService;
	public:
		GPUBuffer(RenderService& renderService, EMeshDataUsage usage);

		/**
		 * Default destructor
		 */
		virtual ~GPUBuffer();

		// Copy construction not allowed
		GPUBuffer(const GPUBuffer& other) = delete;
		
		// Copy assignment not allowed
		GPUBuffer& operator=(const GPUBuffer& other) = delete;

		VkBuffer getBuffer() const;

		nap::Signal<> bufferChanged;

	protected:
		bool setDataInternal(void* data, int elementSize, size_t numVertices, size_t reservedNumVertices, VkBufferUsageFlagBits usage, utility::ErrorState& error);

	private:
		/**
		 * Simple structure used to combine buffer related data
		 */
		struct BufferData
		{
			VmaAllocation		mAllocation = VK_NULL_HANDLE;		///< Single memory allocation handle
			VmaAllocationInfo	mAllocationInfo;					///< Allocation information
			VkBuffer			mBuffer = VK_NULL_HANDLE;			///< Actual buffer
		};

		RenderService*			mRenderService = nullptr;			///< Handle to the render service
		std::vector<BufferData>	mRenderBuffers;						///< Render accessible buffers
		BufferData				mStagingBuffer;						///< Staging buffer, used when uploading static mesh geometry
		int						mCurrentBufferIndex = 0;			///< Current render buffer index
		EMeshDataUsage			mUsage;								///< How the buffer is used, static, updated frequently etc.
		uint32					mSize = 0;							///< Current used buffer size in bytes

		bool setDataInternalStatic(void* data, int elementSize, size_t numVertices, VkBufferUsageFlagBits usage, utility::ErrorState& error);
		bool setDataInternalDynamic(void* data, int elementSize, size_t numVertices, size_t reservedNumVertices, VkBufferUsageFlagBits usage, utility::ErrorState& error);
		void upload(VkCommandBuffer commandBuffer);
	};

} // opengl

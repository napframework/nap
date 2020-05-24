#pragma once

// Local Includes
#include "vulkan/vulkan_core.h"
#include "utility/dllexport.h"
#include "vk_mem_alloc.h"
#include <nap/signalslot.h>
#include <vector>

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
	public:
		GPUBuffer(RenderService& renderService, EMeshDataUsage usage);

		/**
		 * Default destructor
		 */
		virtual ~GPUBuffer();

		GPUBuffer(const GPUBuffer& other) = delete;
		GPUBuffer& operator=(const GPUBuffer& other) = delete;

		VkBuffer getBuffer() const { return mBuffers[mCurrentBufferIndex].mBuffer; }

		nap::Signal<> bufferChanged;

	protected:
		void setDataInternal(VkPhysicalDevice physicalDevice, VkDevice device, void* data, int elementSize, size_t numVertices, size_t reservedNumVertices, VkBufferUsageFlagBits usage);

	private:
		struct BufferData
		{
			VmaAllocation		mAllocation = VK_NULL_HANDLE;
			VmaAllocationInfo	mAllocationInfo;
			VkBuffer			mBuffer = VK_NULL_HANDLE;
		};

		RenderService*			mRenderService = nullptr;
		std::vector<BufferData>	mBuffers;
		int						mCurrentBufferIndex = 0;
		EMeshDataUsage			mUsage;
	};

} // opengl

#pragma once

// Local Includes
#include "vulkan/vulkan_core.h"
#include "utility/dllexport.h"

namespace opengl
{
	/**
	 * Defines a buffer object on the GPU and acts as a base class for 
	 * all Vulkan derived buffer types. This object creation / destruction 
	 * as well as the internal buffer type.
	 */
	class NAPAPI Buffer
	{
	public:
		Buffer() = default;
		/**
		 * Default destructor
		 */
		virtual ~Buffer() {}

		// Don't allow copy, TODO: implement copy
		Buffer(const Buffer& other) = delete;
		Buffer& operator=(const Buffer& other) = delete;

		VkBuffer getBuffer() const { return mBuffer; }

	protected:
		void setDataInternal(VkPhysicalDevice physicalDevice, VkDevice device, void* data, int elementSize, size_t numVertices, size_t reservedNumVertices, VkBufferUsageFlagBits usage);

	private:
		size_t			mCurCapacity = 0;			// Amount of memory reserved
		size_t			mCurSize = 0;				// defines the number of points in the buffer
		VkBuffer		mBuffer = nullptr;
		VkDeviceMemory	mMemory = nullptr;
	};

} // opengl

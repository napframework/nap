#pragma once

#include "gpubuffer.h"
#include <vector>

namespace nap
{

	class NAPAPI IndexBuffer : public GPUBuffer
	{
	public:
		IndexBuffer(VmaAllocator vmaAllocator);

		/**
		 * @return the number of indices specified in this buffer
		 */
		std::size_t getCount() const							{ return mCount; }

		/**
		 * Uploads index data to associated buffer
		 * Note that the pointer to that data needs to be of equal
		 * length as the current number of associated indices stored in the settings
		 * of this object
		 * @param indices: Index data to upload to the GPU
		 */
		void setData(VkPhysicalDevice physicalDevice, VkDevice device, const std::vector<unsigned int>& indices);

	private:
		size_t		mCount = 0;					// number of indices used to construct the triangles
	};
}

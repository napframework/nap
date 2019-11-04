#include "texture2d.h"
#include "bitmap.h"
#include "bitmaputils.h"
#include "renderservice.h"

RTTI_BEGIN_ENUM(nap::EFilterMode)
	RTTI_ENUM_VALUE(nap::EFilterMode::Nearest, "Nearest"),
	RTTI_ENUM_VALUE(nap::EFilterMode::Linear, "Linear"),
	RTTI_ENUM_VALUE(nap::EFilterMode::NearestMipmapNearest, "NearestMipmapNearest"),
	RTTI_ENUM_VALUE(nap::EFilterMode::LinearMipmapNearest, "LinearMipmapNearest"),
	RTTI_ENUM_VALUE(nap::EFilterMode::NearestMipmapLinear, "NearestMipmapLinear"),
	RTTI_ENUM_VALUE(nap::EFilterMode::LinearMipmapLinear, "LinearMipmapLinear")
RTTI_END_ENUM

RTTI_BEGIN_ENUM(nap::EWrapMode)
	RTTI_ENUM_VALUE(nap::EWrapMode::Repeat,			"Repeat"),
	RTTI_ENUM_VALUE(nap::EWrapMode::MirroredRepeat, "MirroredRepeat"),
	RTTI_ENUM_VALUE(nap::EWrapMode::ClampToEdge,	"ClampToEdge"),
	RTTI_ENUM_VALUE(nap::EWrapMode::ClampToBorder,	"ClampToBorder")
RTTI_END_ENUM

RTTI_BEGIN_CLASS(nap::TextureParameters)
	RTTI_PROPERTY("MinFilter",			&nap::TextureParameters::mMinFilter,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("MaxFilter",			&nap::TextureParameters::mMaxFilter,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("WrapVertical",		&nap::TextureParameters::mWrapVertical,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("WrapHorizontal",		&nap::TextureParameters::mWrapHorizontal,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("MaxLodLevel",		&nap::TextureParameters::mMaxLodLevel,		nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Texture2D)
	RTTI_PROPERTY("Parameters", 	&nap::Texture2D::mParameters,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Usage", 			&nap::Texture2D::mUsage,		nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////

/**
* openglFilterMap
*
* Maps Filter modes to supported GL formats
*/
using OpenglFilterMap = std::unordered_map<nap::EFilterMode, GLint>;
static const OpenglFilterMap openglFilterMap =
{
	{ nap::EFilterMode::Nearest,					GL_NEAREST},
	{ nap::EFilterMode::Linear,						GL_LINEAR },
	{ nap::EFilterMode::NearestMipmapNearest,		GL_NEAREST_MIPMAP_NEAREST },
	{ nap::EFilterMode::LinearMipmapNearest,		GL_LINEAR_MIPMAP_NEAREST },
	{ nap::EFilterMode::NearestMipmapLinear,		GL_NEAREST_MIPMAP_LINEAR },
	{ nap::EFilterMode::LinearMipmapLinear,			GL_LINEAR_MIPMAP_LINEAR}
};


/**
 *	openglWrapMap
 *
 * Maps Wrap modes to supported GL formats
 */
using OpenglWrapMap = std::unordered_map<nap::EWrapMode, GLint>;
static const OpenglWrapMap openglWrapMap =
{	 
	{ nap::EWrapMode::Repeat,						GL_REPEAT },
	{ nap::EWrapMode::MirroredRepeat,				GL_MIRRORED_REPEAT },
	{ nap::EWrapMode::ClampToEdge,					GL_CLAMP_TO_EDGE },
	{ nap::EWrapMode::ClampToBorder,				GL_CLAMP_TO_BORDER }
};


/**
 *	@return the opengl filter based on @filter
 */
static GLint getGLFilterMode(nap::EFilterMode filter)
{
	auto it = openglFilterMap.find(filter);
	assert(it != openglFilterMap.end());
	return it->second;
}


/**
 *	@return the opengl wrap mode based on @wrapmode
 */
static GLint getGLWrapMode(nap::EWrapMode wrapmode)
{
	auto it = openglWrapMap.find(wrapmode);
	assert(it != openglWrapMap.end());
	return it->second;
}


static void convertTextureParameters(const nap::TextureParameters& input, opengl::TextureParameters& output)
{
	output.minFilter	=	getGLFilterMode(input.mMinFilter);
	output.maxFilter	=	getGLFilterMode(input.mMaxFilter);
	output.wrapVertical =	getGLWrapMode(input.mWrapVertical);
	output.wrapHorizontal = getGLWrapMode(input.mWrapHorizontal);
	output.maxLodLevel =	input.mMaxLodLevel;
}


//////////////////////////////////////////////////////////////////////////

namespace nap
{
	// Returns number of components each texel has in this format
	static int getNumComponents(Bitmap::EChannels channels)
	{
		switch (channels)
		{
		case Bitmap::EChannels::R:
			return 1;

		case Bitmap::EChannels::RGB:
		case Bitmap::EChannels::BGR:
			return 3;

		case Bitmap::EChannels::RGBA:
		case Bitmap::EChannels::BGRA:
			return 4;
		}

		assert(false);
		return 0;
	}

	// Returns What the size in bytes is of a component type
	static int getComponentSize(Bitmap::EDataType type)
	{
		switch (type)
		{
		case Bitmap::EDataType::BYTE:
			return 1;
		case Bitmap::EDataType::USHORT:
			return 2;
		case Bitmap::EDataType::FLOAT:
			return 4;
		}

		assert(false);
		return 0;
	}

	Texture2D::Texture2D(RenderService& renderService) :
		mRenderer(&renderService.getRenderer())
	{
	}

	void Texture2D::initTexture(const opengl::Texture2DSettings& settings)
	{
		// Create the texture with the associated settings
		opengl::TextureParameters gl_params;
		convertTextureParameters(mParameters, gl_params);
		mTexture.init(settings, gl_params, mUsage);
	}

	namespace 
	{
		uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
		{
			VkPhysicalDeviceMemoryProperties memProperties;
			vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

			for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
				if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
					return i;
				}
			}

			return -1;
		}

		bool createBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory, utility::ErrorState& errorState)
		{
			VkBufferCreateInfo bufferInfo = {};
			bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferInfo.size = size;
			bufferInfo.usage = usage;
			bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			if (!errorState.check(vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) == VK_SUCCESS, "could not create buffer"))
				return false;

			VkMemoryRequirements memRequirements;
			vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

			VkMemoryAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);

			if (!errorState.check(vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) == VK_SUCCESS, "Could not allocate memory for buffer"))
				return false;

			if (!errorState.check(vkBindBufferMemory(device, buffer, bufferMemory, 0) == VK_SUCCESS, "Coult not bind buffer memory"))
				return false;

			return true;
		}

		bool createImage(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory, utility::ErrorState& errorState)
		{
			VkImageCreateInfo imageInfo = {};
			imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageInfo.imageType = VK_IMAGE_TYPE_2D;
			imageInfo.extent.width = width;
			imageInfo.extent.height = height;
			imageInfo.extent.depth = 1;
			imageInfo.mipLevels = 1;
			imageInfo.arrayLayers = 1;
			imageInfo.format = format;
			imageInfo.tiling = tiling;
			imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageInfo.usage = usage;
			imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			if (!errorState.check(vkCreateImage(device, &imageInfo, nullptr, &image) == VK_SUCCESS, "Failed to create image"))
				return false;

			VkMemoryRequirements memRequirements;
			vkGetImageMemoryRequirements(device, image, &memRequirements);

			VkMemoryAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);

			if (!errorState.check(vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) == VK_SUCCESS, "Failed to allocate image memory"))
				return false;

			vkBindImageMemory(device, image, imageMemory, 0);
			return true;
		}

	VkFormat getTextureFormat(const Bitmap& bitmap)
		{
			switch (bitmap.getChannels())
			{
			case Bitmap::EChannels::R:
				{
					switch (bitmap.getDataType())
					{
					case nap::Bitmap::EDataType::BYTE:
						return VK_FORMAT_R8_UNORM;
					case nap::Bitmap::EDataType::FLOAT:
						return VK_FORMAT_R32_SFLOAT;
					case nap::Bitmap::EDataType::USHORT:
						return VK_FORMAT_R16_UNORM;
					}
					break;
				}
			case Bitmap::EChannels::RGB:
				{
					switch (bitmap.getDataType())
					{
					case nap::Bitmap::EDataType::BYTE:
						return VK_FORMAT_R8G8B8_UNORM;
					case nap::Bitmap::EDataType::FLOAT:
						return VK_FORMAT_R32G32B32_SFLOAT;
					case nap::Bitmap::EDataType::USHORT:
						return VK_FORMAT_R16G16B16_UNORM;
					}
					break;
				}
			case Bitmap::EChannels::BGR:
				{
					switch (bitmap.getDataType())
					{
					case nap::Bitmap::EDataType::BYTE:
						return VK_FORMAT_B8G8R8_UNORM;
					case nap::Bitmap::EDataType::FLOAT:
						return VK_FORMAT_UNDEFINED;
					case nap::Bitmap::EDataType::USHORT:
						return VK_FORMAT_UNDEFINED;
					}
					break;
				}
			case Bitmap::EChannels::RGBA:
				{
					switch (bitmap.getDataType())
					{
					case nap::Bitmap::EDataType::BYTE:
						return VK_FORMAT_R8G8B8A8_UNORM;
					case nap::Bitmap::EDataType::FLOAT:
						return VK_FORMAT_R32G32B32A32_SFLOAT;
					case nap::Bitmap::EDataType::USHORT:
						return VK_FORMAT_R16G16B16A16_UNORM;
					}
					break;
				}
			case Bitmap::EChannels::BGRA:
				{
					switch (bitmap.getDataType())
					{
					case nap::Bitmap::EDataType::BYTE:
						return VK_FORMAT_B8G8R8A8_UNORM;
					case nap::Bitmap::EDataType::FLOAT:
						return VK_FORMAT_UNDEFINED;
					case nap::Bitmap::EDataType::USHORT:
						return VK_FORMAT_UNDEFINED;
					}
					break;
				}
			}
			return VK_FORMAT_UNDEFINED;
		}

		VkCommandBuffer beginSingleTimeCommands(Renderer& renderer) 
		{
			VkCommandBufferAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandPool = renderer.getCommandPool();
			allocInfo.commandBufferCount = 1;

			VkCommandBuffer commandBuffer;
			vkAllocateCommandBuffers(renderer.getDevice(), &allocInfo, &commandBuffer);

			VkCommandBufferBeginInfo beginInfo = {};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

			vkBeginCommandBuffer(commandBuffer, &beginInfo);

			return commandBuffer;
		}

		void endSingleTimeCommands(Renderer& renderer, VkCommandBuffer commandBuffer) 
		{
			vkEndCommandBuffer(commandBuffer);

			VkSubmitInfo submitInfo = {};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &commandBuffer;

			vkQueueSubmit(renderer.getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
			vkQueueWaitIdle(renderer.getGraphicsQueue());

			vkFreeCommandBuffers(renderer.getDevice(), renderer.getCommandPool(), 1, &commandBuffer);
		}

		void transitionImageLayout(Renderer& renderer, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) 
		{
			VkCommandBuffer commandBuffer = beginSingleTimeCommands(renderer);

			VkImageMemoryBarrier barrier = {};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = oldLayout;
			barrier.newLayout = newLayout;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = image;
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;

			VkPipelineStageFlags sourceStage;
			VkPipelineStageFlags destinationStage;

			if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) 
			{
				barrier.srcAccessMask = 0;
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

				sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			}
			else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) 
			{
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
				destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			}
			else 
			{
				throw std::invalid_argument("unsupported layout transition!");
			}

			vkCmdPipelineBarrier(
				commandBuffer,
				sourceStage, destinationStage,
				0,
				0, nullptr,
				0, nullptr,
				1, &barrier
			);

			endSingleTimeCommands(renderer, commandBuffer);
		}

		void copyBufferToImage(Renderer& renderer, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
			VkCommandBuffer commandBuffer = beginSingleTimeCommands(renderer);

			VkBufferImageCopy region = {};
			region.bufferOffset = 0;
			region.bufferRowLength = 0;
			region.bufferImageHeight = 0;
			region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.mipLevel = 0;
			region.imageSubresource.baseArrayLayer = 0;
			region.imageSubresource.layerCount = 1;
			region.imageOffset = { 0, 0, 0 };
			region.imageExtent = {
				width,
				height,
				1
			};

			vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

			endSingleTimeCommands(renderer, commandBuffer);
		}

		bool createImageView(VkDevice device, VkImage image, VkFormat format, VkImageView& imageView, utility::ErrorState& errorState) 
		{
			VkImageViewCreateInfo viewInfo = {};
			viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewInfo.image = image;
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewInfo.format = format;
			viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.subresourceRange.baseArrayLayer = 0;
			viewInfo.subresourceRange.layerCount = 1;

			if (!errorState.check(vkCreateImageView(device, &viewInfo, nullptr, &imageView) == VK_SUCCESS, "Failed to create texture image view"))
				return false;

			return true;
		}
	}	

	bool Texture2D::initFromBitmap(const Bitmap& bitmap, bool compressed, utility::ErrorState& errorState)
	{
		assert(!bitmap.empty());

		VkDevice device = mRenderer->getDevice();
		VkPhysicalDevice physicalDevice = mRenderer->getPhysicalDevice();

		VkDeviceSize imageSize = getNumComponents(bitmap.getChannels()) * getComponentSize(bitmap.getDataType()) * bitmap.getWidth() * bitmap.getHeight();
		
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		if (!createBuffer(device, physicalDevice, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory, errorState))
			return false;

		void* data;
		vkMapMemory(mRenderer->getDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
		memcpy(data, bitmap.getData(), static_cast<size_t>(imageSize));
		vkUnmapMemory(device, stagingBufferMemory);

		VkFormat texture_format = getTextureFormat(bitmap);
		if (!errorState.check(texture_format != VK_FORMAT_UNDEFINED, "Unsupported texture format"))
			return false;

		if (!createImage(device, physicalDevice, bitmap.getWidth(), bitmap.getHeight(), texture_format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mTextureImage, mTextureImageMemory, errorState))
			return false;

		transitionImageLayout(*mRenderer, mTextureImage, texture_format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		copyBufferToImage(*mRenderer, stagingBuffer, mTextureImage, static_cast<uint32_t>(bitmap.getWidth()), static_cast<uint32_t>(bitmap.getHeight()));
		transitionImageLayout(*mRenderer, mTextureImage, texture_format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);

		if (!createImageView(device, mTextureImage, texture_format, mTextureView, errorState))
			return false;

		return true;
	}


	const glm::vec2 Texture2D::getSize() const
	{
		return glm::vec2(mTexture.getSettings().mWidth, mTexture.getSettings().mHeight);
	}


	int Texture2D::getWidth() const
	{
		return static_cast<int>(mTexture.getSettings().mWidth);
	}


	int Texture2D::getHeight() const
	{
		return static_cast<int>(mTexture.getSettings().mHeight);
	}

	void Texture2D::update(const Bitmap& bitmap)
	{
		update(bitmap.getData());
	}


	void Texture2D::update(const void* data, int pitch)
	{
		mTexture.setData(data, pitch);
	}


	void Texture2D::getData(Bitmap& bitmap)
	{
		if (bitmap.empty())
			bitmap.initFromTexture(mTexture.getSettings());

		mTexture.getData(bitmap.getData(), bitmap.getSizeInBytes());
	}


	nap::uint Texture2D::getHandle() const
	{
		return getTexture().getTextureId();
	}


	void Texture2D::startGetData()
	{
		getTexture().asyncStartGetData();
	}


	void Texture2D::endGetData(Bitmap& bitmap)
	{
		if (bitmap.empty())
			bitmap.initFromTexture(mTexture.getSettings());

		mTexture.getData(bitmap.getData(), bitmap.getSizeInBytes());
	}
}

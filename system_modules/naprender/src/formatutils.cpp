#include "formatutils.h"

namespace nap
{
	VkBufferUsageFlags getVulkanBufferUsage(EDescriptorType descriptorType)
	{
		switch (descriptorType)
		{
		case EDescriptorType::Uniform:
			return VkBufferUsageFlagBits::VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		case EDescriptorType::Storage:
			return VkBufferUsageFlagBits::VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		default:
			NAP_ASSERT_MSG(false, "Unsupported descriptor type");
			return 0;
		}
	}


	VkDescriptorType getVulkanDescriptorType(EDescriptorType descriptorType)
	{
		switch (descriptorType)
		{
		case EDescriptorType::Uniform:
			return VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		case EDescriptorType::Storage:
			return VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		default:
			NAP_ASSERT_MSG(false, "Unsupported descriptor type");
			return VkDescriptorType::VK_DESCRIPTOR_TYPE_MAX_ENUM;
		}
	}


	VkFormat getVulkanFormat(rtti::TypeInfo type)
	{
		static const std::map<rtti::TypeInfo, VkFormat> format_table =
		{
			{RTTI_OF(uint),			VkFormat::VK_FORMAT_R32_UINT},
			{RTTI_OF(int),			VkFormat::VK_FORMAT_R32_SINT},
			{RTTI_OF(float),		VkFormat::VK_FORMAT_R32_SFLOAT},
			{RTTI_OF(glm::vec2),	VkFormat::VK_FORMAT_R32G32_SFLOAT},
			{RTTI_OF(glm::vec3),	VkFormat::VK_FORMAT_R32G32B32_SFLOAT},
			{RTTI_OF(glm::vec4),	VkFormat::VK_FORMAT_R32G32B32A32_SFLOAT}
		};

		const auto it = format_table.find(type);
		if (it != format_table.end())
			return it->second;

		NAP_ASSERT_MSG(false, "Unsupported vertex buffer type");
		return VkFormat::VK_FORMAT_UNDEFINED;
	}
}

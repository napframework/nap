// Local Includes
#include "renderservice.h"
#include "renderablemeshcomponent.h"
#include "rendercomponent.h"
#include "renderwindow.h"
#include "transformcomponent.h"
#include "cameracomponent.h"
#include "renderglobals.h"
#include "mesh.h"
#include "depthsorter.h"
#include "vertexbuffer.h"
#include "texture2d.h"
#include "descriptorsetcache.h"
#include "descriptorsetallocator.h"
#include "sdlhelpers.h"

// External Includes
#include <nap/core.h>
#include <glm/gtx/matrix_decompose.hpp>
#include <rtti/factory.h>
#include <nap/resourcemanager.h>
#include <nap/logger.h>
#include <sceneservice.h>
#include <scene.h>
#include <SDL_vulkan.h>
#include <glslang/Public/ShaderLang.h>
#include <nap/assert.h>

RTTI_BEGIN_ENUM(nap::RenderServiceConfiguration::EPhysicalDeviceType)
	RTTI_ENUM_VALUE(nap::RenderServiceConfiguration::EPhysicalDeviceType::Integrated,	"Integrated"),
	RTTI_ENUM_VALUE(nap::RenderServiceConfiguration::EPhysicalDeviceType::Discrete,		"Discrete"),
	RTTI_ENUM_VALUE(nap::RenderServiceConfiguration::EPhysicalDeviceType::Virtual,		"Integrated"),
	RTTI_ENUM_VALUE(nap::RenderServiceConfiguration::EPhysicalDeviceType::CPU,			"CPU")
RTTI_END_ENUM

RTTI_BEGIN_CLASS(nap::RenderServiceConfiguration)
	RTTI_PROPERTY("PreferredGPU",		&nap::RenderServiceConfiguration::mPreferredGPU,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Layers",				&nap::RenderServiceConfiguration::mLayers,						nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Extensions",			&nap::RenderServiceConfiguration::mAdditionalExtensions,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("EnableHighDPI",		&nap::RenderServiceConfiguration::mEnableHighDPIMode,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ShowLayers",			&nap::RenderServiceConfiguration::mPrintAvailableLayers,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ShowExtensions",		&nap::RenderServiceConfiguration::mPrintAvailableExtensions,	nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderService)
	RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// Static Methods
	//////////////////////////////////////////////////////////////////////////

	/**
	 * @return VK physical device type
	 */
	static VkPhysicalDeviceType getPhysicalDeviceType(RenderServiceConfiguration::EPhysicalDeviceType devType)
	{
		switch(devType)
		{
		case RenderServiceConfiguration::EPhysicalDeviceType::Discrete:
			return VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
		case RenderServiceConfiguration::EPhysicalDeviceType::Integrated:
			return VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
		case RenderServiceConfiguration::EPhysicalDeviceType::CPU:
			return VK_PHYSICAL_DEVICE_TYPE_CPU;
		case RenderServiceConfiguration::EPhysicalDeviceType::Virtual:
			return VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU;
		default:
			assert(false);
		}
		return VK_PHYSICAL_DEVICE_TYPE_OTHER;
	}


	/**
	 * @return max sample count associated with the given physical device
	 */
	static VkSampleCountFlagBits getMaxSampleCount(VkPhysicalDevice physicalDevice)
	{
		VkPhysicalDeviceProperties physicalDeviceProperties;
		vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
		VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
		if (counts & VK_SAMPLE_COUNT_64_BIT)	{ return VK_SAMPLE_COUNT_64_BIT; }
		if (counts & VK_SAMPLE_COUNT_32_BIT)	{ return VK_SAMPLE_COUNT_32_BIT; }
		if (counts & VK_SAMPLE_COUNT_16_BIT)	{ return VK_SAMPLE_COUNT_16_BIT; }
		if (counts & VK_SAMPLE_COUNT_8_BIT)		{ return VK_SAMPLE_COUNT_8_BIT; }
		if (counts & VK_SAMPLE_COUNT_4_BIT)		{ return VK_SAMPLE_COUNT_4_BIT; }
		if (counts & VK_SAMPLE_COUNT_2_BIT)		{ return VK_SAMPLE_COUNT_2_BIT; }
		return VK_SAMPLE_COUNT_1_BIT;
	}


	/**
	 * @return Vulkan topology mode based on given NAP draw mode
	 */
	static VkPrimitiveTopology getTopology(EDrawMode drawMode)
	{
		switch (drawMode)
		{
			case EDrawMode::Points:
				return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
			case EDrawMode::Lines:
				return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
			case EDrawMode::LineStrip:
				return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
			case EDrawMode::Triangles:
				return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			case EDrawMode::TriangleStrip:
				return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
			case EDrawMode::TriangleFan:
				return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
			default:
			{
				assert(false);
				return VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
			}
		}
	}


	/**
	 * @return Vulkan cull mode based on given NAP cull mode
	 */
	static VkCullModeFlagBits getCullMode(ECullMode mode)
	{
		switch (mode)
		{
			case ECullMode::Back:
				return VK_CULL_MODE_BACK_BIT;
			case ECullMode::Front:
				return VK_CULL_MODE_FRONT_BIT;
			case ECullMode::FrontAndBack:
				return VK_CULL_MODE_FRONT_AND_BACK;
			case ECullMode::None:
				return VK_CULL_MODE_NONE;
			default:
			{
				assert(false);
				return VK_CULL_MODE_NONE;
			}
		}
	}


	/**
	 * @return device type name
	 */
	static std::string getDeviceTypeName(VkPhysicalDeviceType type)
	{
		switch (type)
		{
		case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
			return "Discrete";
		case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
			return "Integrated";
		case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
			return "Virtual";
		case VK_PHYSICAL_DEVICE_TYPE_CPU:
			return "CPU";
		default:
			return "Unknown";
		}
	}


	/**
	 * @return the set of required device extension names
	 */
	static const std::vector<std::string>& getRequiredDeviceExtensionNames()
	{
		const static std::vector<std::string> layers = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
		return layers;
	}


	/**
	 * Callback that receives a debug message from Vulkan
	 */
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType,
		uint64_t obj,
		size_t location,
		int32_t code,
		const char* layerPrefix,
		const char* msg,
		void* userData)
	{
		nap::Logger::info("Validation Layer [%s]: %s", layerPrefix, msg);
		return VK_FALSE;
	}


	/**
	 * Creates a debug report callback object
	 */
	static VkResult createDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback)
	{
		auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
		return func != nullptr ? func(instance, pCreateInfo, pAllocator, pCallback) : VK_ERROR_EXTENSION_NOT_PRESENT;
	}


	/**
	 * Destroys a debug report callback object
	 */
	static void destroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT pCallback)
	{
		auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
		if (func != nullptr)
			func(instance, pCallback, nullptr);
	}


	/**
	 * Sets up the vulkan messaging callback specified above
	 */
	static bool setupDebugCallback(VkInstance instance, VkDebugReportCallbackEXT& callback, utility::ErrorState& errorState)
	{
		VkDebugReportCallbackCreateInfoEXT createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
		createInfo.pfnCallback = debugCallback;

		if (!errorState.check(createDebugReportCallbackEXT(instance, &createInfo, nullptr, &callback) == VK_SUCCESS, "Unable to create debug report callback extension"))
			return false;
		return true;
	}


	/**
	 * Returns all available vulkan layers
	 */
	static bool getAvailableVulkanLayers(const std::vector<std::string>& requestedLayers, bool print, std::vector<std::string>& outLayers, utility::ErrorState& errorState)
	{
		// Figure out the amount of available layers
		// Layers are used for debugging / validation etc / profiling..
		unsigned int instance_layer_count = 0;
		if (!errorState.check(vkEnumerateInstanceLayerProperties(&instance_layer_count, NULL) == VK_SUCCESS, "Unable to query vulkan instance layer property count"))
			return false;

		std::vector<VkLayerProperties> instance_layers(instance_layer_count);
		if (!errorState.check(vkEnumerateInstanceLayerProperties(&instance_layer_count, instance_layers.data()) == VK_SUCCESS, "Unable to retrieve vulkan instance layer names"))
			return false;
		if (print) { Logger::info("Found %d Vulkan layers:", instance_layer_count); }

		outLayers.clear();
		for (int index = 0; index < instance_layers.size(); ++index)
		{
			VkLayerProperties& layer = instance_layers[index];
			if (print) { Logger::info("%d: %s", index, layer.layerName); }
			const auto found_it = std::find_if(requestedLayers.begin(), requestedLayers.end(), [&](const auto& it)
			{
				return it == std::string(layer.layerName);
			});

			if (found_it != requestedLayers.end())
				outLayers.emplace_back(layer.layerName);
		}
		return true;
	}


	bool getAvailableVulkanInstanceExtensions(SDL_Window* window, std::vector<std::string>& outExtensions, utility::ErrorState& errorState)
	{
		// Figure out the amount of extensions vulkan needs to interface with the os windowing system 
		// This is necessary because vulkan is a platform agnostic API and needs to know how to interface with the windowing system
		unsigned int ext_count = 0;
		if (!errorState.check(SDL_Vulkan_GetInstanceExtensions(window, &ext_count, nullptr) == SDL_TRUE, "Unable to query the number of Vulkan instance extensions"))
			return false;

		// Use the amount of extensions queried before to retrieve the names of the extensions
		std::vector<const char*> ext_names(ext_count);
		if (!errorState.check(SDL_Vulkan_GetInstanceExtensions(window, &ext_count, ext_names.data()) == SDL_TRUE, "Unable to query the number of Vulkan instance extension names"))
			return false;

		// Store
		for (unsigned int i = 0; i < ext_count; i++)
			outExtensions.emplace_back(ext_names[i]);

		// Add debug display extension, we need this to relay debug messages
		outExtensions.emplace_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
		return true;
	}


	/**
	* Creates a vulkan instance using all the available instance extensions and layers
	* @return if the instance was created successfully
	*/
	bool createVulkanInstance(const std::vector<std::string>& layerNames, const std::vector<std::string>& extensionNames, VkInstance& outInstance, uint32& outVersion, utility::ErrorState& errorState)
	{
		// Copy layers
		std::vector<const char*> layer_names;
		for (const auto& layer : layerNames)
			layer_names.emplace_back(layer.c_str());

		// Copy extensions
		std::vector<const char*> ext_names;
		for (const auto& ext : extensionNames)
			ext_names.emplace_back(ext.c_str());

		// Get the supported vulkan instance version
		uint32 current_api(0);
		if (!errorState.check(vkEnumerateInstanceVersion(&current_api) == VK_SUCCESS,
			"Unable Query instance-level version of Vulkan before instance creation"))
			return false;

		// Create api version without patch, not used when creating instance
		outVersion = VK_MAKE_VERSION(VK_VERSION_MAJOR(current_api), VK_VERSION_MINOR(current_api), 0);

		// initialize the VkApplicationInfo structure
		VkApplicationInfo app_info = {};
		app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		app_info.pNext = NULL;
		app_info.pApplicationName = "NAP";
		app_info.applicationVersion = 1;
		app_info.pEngineName = "NAP";
		app_info.engineVersion = 1;
		app_info.apiVersion = outVersion;

		// initialize the VkInstanceCreateInfo structure
		VkInstanceCreateInfo inst_info = {};
		inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		inst_info.pNext = NULL;
		inst_info.flags = 0;
		inst_info.pApplicationInfo = &app_info;
		inst_info.enabledExtensionCount = static_cast<uint32_t>(ext_names.size());
		inst_info.ppEnabledExtensionNames = ext_names.data();
		inst_info.enabledLayerCount = static_cast<uint32_t>(layer_names.size());
		inst_info.ppEnabledLayerNames = layer_names.data();

		// Create vulkan runtime instance
		VkResult res = vkCreateInstance(&inst_info, NULL, &outInstance);
		if (res == VK_SUCCESS)
			return true;
		
		// Add error message and return
		errorState.fail(res == VK_ERROR_INCOMPATIBLE_DRIVER ?
			"Unable to create Vulkan instance, cannot find a compatible Vulkan driver" :
			"Unable to create Vulkan instance: error: %d", static_cast<int>(res));
		return false;
	}


	/**
	 * Selects a device based on user preference, min required api version and queue family requirements
	 */
	static bool selectPhysicalDevice(VkInstance instance, VkPhysicalDeviceType preferredType, uint32 minAPIVersion, VkPhysicalDevice& outDevice, VkPhysicalDeviceProperties& outProperties, VkPhysicalDeviceFeatures& outFeatures, int& outQueueFamilyIndex, utility::ErrorState& errorState)
	{
		// Get number of available physical devices, needs to be at least 1
		unsigned int physical_device_count(0);
		vkEnumeratePhysicalDevices(instance, &physical_device_count, nullptr);
		if (!errorState.check(physical_device_count != 0, "No physical devices found"))
			return false;

		// Now get the devices
		std::vector<VkPhysicalDevice> physical_devices(physical_device_count);
		vkEnumeratePhysicalDevices(instance, &physical_device_count, physical_devices.data());

		// Show device information
		Logger::info("Found %d GPU(s):", physical_device_count);
		
		// Pick the right GPU, based on type preference, api-version and queue support
		std::vector<VkPhysicalDeviceProperties> physical_device_properties(physical_devices.size());
		VkQueueFlags required_flags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT;

		std::vector<int> valid_devices;
		std::vector<int> queue_indices;
		int preferred_idx = -1;
		for (int index = 0; index < physical_devices.size(); ++index)
		{
			// Get current physical device
			VkPhysicalDevice physical_device = physical_devices[index];

			// Get properties associated with device
			VkPhysicalDeviceProperties& properties = physical_device_properties[index];
			vkGetPhysicalDeviceProperties(physical_device, &properties);
			Logger::info("%d: %s, type: %s, version: %d.%d", index, properties.deviceName, getDeviceTypeName(properties.deviceType).c_str(), VK_VERSION_MAJOR(properties.apiVersion), VK_VERSION_MINOR(properties.apiVersion));

			// If the supported api version < required by currently used api, continue
			if (properties.apiVersion < minAPIVersion)
			{
				Logger::warn("%d: Incompatible driver, min required api version: %d.%d", index, VK_VERSION_MAJOR(minAPIVersion), VK_VERSION_MINOR(minAPIVersion));
				continue;
			}

			// Find the number queues this device supports
			uint32 family_queue_count(0);
			vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &family_queue_count, nullptr);
			if (family_queue_count == 0)
			{
				Logger::warn("%d: No queue families available", index);
				continue;
			}

			// Extract the properties of all the queue families
			// We want to make sure that we have a queue that supports the required flags
			std::vector<VkQueueFamilyProperties> queue_properties(family_queue_count);
			vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &family_queue_count, queue_properties.data());

			// Make sure the family of commands contains an option to issue graphical commands.
			int queue_node_index = -1;
			for (unsigned int i = 0; i < family_queue_count; i++)
			{
				if (queue_properties[i].queueCount > 0 && queue_properties[i].queueFlags & required_flags)
				{
					queue_node_index = i;
					break;
				}
			}
			
			// No compatible queue found
			if (queue_node_index < 0)
			{
				Logger::warn("%d: Unable to find compatible queue family", index);
				continue;
			}

			// This is at least a compatible device
			valid_devices.emplace_back(index);
			queue_indices.emplace_back(index);
			nap::Logger::info("%d: Compatible", index);

			// Check if it's the preferred type, if so select it
			preferred_idx = properties.deviceType == preferredType && preferred_idx < 0 ? index : preferred_idx;
		}

		// if there are no valid devices, bail.
		if (!errorState.check(!valid_devices.empty(), "No compatible device found"))
			return false;

		// If the preferred GPU is found, use that one, otherwise first compatible one
		int device_idx = preferred_idx;
		if (preferred_idx < 0)
		{
			nap::Logger::warn("Unable to find preferred device, selecting first compatible one");
			device_idx = 0;
		}

		// Set the output variables
		outDevice = physical_devices[device_idx];
		outProperties = physical_device_properties[device_idx];
		outQueueFamilyIndex = queue_indices[device_idx];

		// Extract device features
		VkPhysicalDeviceFeatures selected_device_featues;
		vkGetPhysicalDeviceFeatures(physical_devices[device_idx], &outFeatures);
		nap::Logger::info("Selected device: %d", device_idx, physical_device_properties[device_idx].deviceName);
		return true;
	}


	/**
	 * Creates the logical device based on the selected physical device, queue index and required extensions
	 */
	static bool createLogicalDevice(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceFeatures& physicalDeviceFeatures, unsigned int queueFamilyIndex, const std::vector<std::string>& layerNames, const std::unordered_set<std::string>& extensionNames, bool print, VkDevice& outDevice, utility::ErrorState& errorState)
	{
		// Copy layer names
		std::vector<const char*> layer_names;
		for (const auto& layer : layerNames)
			layer_names.emplace_back(layer.c_str());

		// Get the number of available extensions for our graphics card
		uint32_t device_property_count(0);
		if (!errorState.check(vkEnumerateDeviceExtensionProperties(physicalDevice, NULL, &device_property_count, NULL) == VK_SUCCESS, "Unable to acquire device extension property count"))
			return false;
		if (print) { Logger::info("Found %d Vulkan device extensions:", device_property_count); }

		// Acquire their actual names
		std::vector<VkExtensionProperties> device_properties(device_property_count);
		if (!errorState.check(vkEnumerateDeviceExtensionProperties(physicalDevice, NULL, &device_property_count, device_properties.data()) == VK_SUCCESS, "Unable to acquire device extension property names"))
			return false;

		// Match names against requested extension
		std::vector<const char*> device_property_names;
		for (int index = 0; index < device_properties.size(); ++index)
		{
			const VkExtensionProperties& ext_property = device_properties[index];
			if (print) { Logger::info("%d: %s", index, ext_property.extensionName); }

			auto it = extensionNames.find(std::string(ext_property.extensionName));
			if (it != extensionNames.end())
				device_property_names.emplace_back(ext_property.extensionName);
		}

		// Make sure we found all required extensions
		if (!errorState.check(extensionNames.size() == device_property_names.size(), "Unable to find all required extensions"))
			return false;

		// Log the extensions we can use
		for (const auto& name : device_property_names)
			Logger::info("Applying device extension: %s", name);

		// Create queue information structure used by device based on the previously fetched queue information from the physical device
		// We create one command processing queue for graphics
		VkDeviceQueueCreateInfo queue_create_info;
		queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_create_info.queueFamilyIndex = queueFamilyIndex;
		queue_create_info.queueCount = 1;
		std::vector<float> queue_prio = { 1.0f };
		queue_create_info.pQueuePriorities = queue_prio.data();
		queue_create_info.pNext = NULL;
		queue_create_info.flags = NULL;

		// Enable specific features, we could also enable all supported features here.
		VkPhysicalDeviceFeatures device_features {0};
		device_features.sampleRateShading = physicalDeviceFeatures.sampleRateShading;

		// Device creation information	
		VkDeviceCreateInfo create_info;
		create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		create_info.queueCreateInfoCount = 1;
		create_info.pQueueCreateInfos = &queue_create_info;
		create_info.ppEnabledLayerNames = layer_names.data();
		create_info.enabledLayerCount = static_cast<uint32_t>(layer_names.size());
		create_info.ppEnabledExtensionNames = device_property_names.data();
		create_info.enabledExtensionCount = static_cast<uint32_t>(device_property_names.size());
		create_info.pNext = NULL;
		create_info.pEnabledFeatures = &device_features;
		create_info.flags = NULL;

		// Finally we're ready to create a new device
		if (!errorState.check(vkCreateDevice(physicalDevice, &create_info, nullptr, &outDevice) == VK_SUCCESS, "Failed to create logical device"))
			return false;

		return true;
	}


	/**
	 * Creates a command pool associated with the given queue index
	 */
	static bool createCommandPool(VkPhysicalDevice physicalDevice, VkDevice device, unsigned int graphicsQueueIndex, VkCommandPool& commandPool)
	{
		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = graphicsQueueIndex;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		return vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) == VK_SUCCESS;
	}


	/**
	 * Select depth format to use
	 */
	static bool findDepthFormat(VkPhysicalDevice physicalDevice, VkFormat& outFormat)
	{
		static const std::vector<VkFormat> candidates = { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };
		for (VkFormat format : candidates)
		{
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);
			if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
			{
				outFormat = format;
				return true;
			}
		}
		return false;
	}


	/**
	 *  Create a command buffer, using the given pool
	 */
	static bool createCommandBuffer(VkDevice device, VkCommandPool commandPool, VkCommandBuffer& commandBuffer, utility::ErrorState& errorState)
	{
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		if (!errorState.check(vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer) == VK_SUCCESS, "Failed to allocate command buffer"))
			return false;

		return true;
	}


	/**
	 * Create CPU - GPU synchronization primitive
	 */
	static bool createFence(VkDevice device, VkFence& fence, utility::ErrorState& errorState)
	{
		VkFenceCreateInfo fenceInfo = {};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		return errorState.check(vkCreateFence(device, &fenceInfo, nullptr, &fence) == VK_SUCCESS, "Failed to create sync objects");
	}


	/**
	 * Get vulkan depth and stencil creation information, based on current material state.
	 */
	static VkPipelineDepthStencilStateCreateInfo getDepthStencilCreateInfo(MaterialInstance& materialInstance)
	{
		VkPipelineDepthStencilStateCreateInfo depth_stencil = {};
		depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depth_stencil.depthTestEnable = VK_FALSE;
		depth_stencil.depthWriteEnable = VK_FALSE;
		depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		depth_stencil.depthBoundsTestEnable = VK_FALSE;
		depth_stencil.stencilTestEnable = VK_FALSE;

		// If the depth mode is inherited from the blend mode, determine the correct depth mode to use
		EDepthMode depth_mode = materialInstance.getDepthMode();
		if (depth_mode == EDepthMode::InheritFromBlendMode)
			depth_mode = materialInstance.getBlendMode() == EBlendMode::Opaque ? EDepthMode::ReadWrite : EDepthMode::ReadOnly;

		// Update depth configuration based on blend mode
		switch (depth_mode)
		{
			case EDepthMode::ReadWrite:
			{
				depth_stencil.depthTestEnable  = VK_TRUE;
				depth_stencil.depthWriteEnable = VK_TRUE;
				break;
			}
			case EDepthMode::ReadOnly:
			{
				depth_stencil.depthTestEnable  = VK_TRUE;
				depth_stencil.depthWriteEnable = VK_FALSE;
				break;
			}
			case EDepthMode::WriteOnly:
			{
				depth_stencil.depthTestEnable  = VK_FALSE;
				depth_stencil.depthWriteEnable = VK_TRUE;
				break;
			}
			case EDepthMode::NoReadWrite:
			{
				depth_stencil.depthTestEnable  = VK_FALSE;
				depth_stencil.depthWriteEnable = VK_FALSE;
				break;
			}
		default:
			assert(false);
		}
		return depth_stencil;
	}


	/**
	 * Get color blend state based on material settings.
	 */
	static VkPipelineColorBlendAttachmentState getColorBlendAttachmentState(MaterialInstance& materialInstance)
	{
		VkPipelineColorBlendAttachmentState color_blend_attachment_state = {};
		color_blend_attachment_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		color_blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;
		color_blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;

		EBlendMode blend_mode = materialInstance.getBlendMode();
		switch (blend_mode)
		{
			case EBlendMode::Opaque:
			{
				color_blend_attachment_state.blendEnable = VK_FALSE;
				break;
			}
			case EBlendMode::AlphaBlend:
			{
				color_blend_attachment_state.blendEnable = VK_TRUE;
				color_blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
				color_blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
				color_blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
				color_blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
				break;
			}
			case EBlendMode::Additive:
			{
				color_blend_attachment_state.blendEnable = VK_TRUE;
				color_blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
				color_blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
				color_blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
				color_blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
				break;
			}
		}
		return color_blend_attachment_state;
	}


	/**
	 * Creates a new Vulkan pipeline based on the provided settings
	 */
	static bool createGraphicsPipeline(VkDevice device, 
		MaterialInstance& materialInstance, 
		EDrawMode drawMode,  
		ECullWindingOrder windingOrder, 
		VkRenderPass renderPass, 
		VkSampleCountFlagBits sampleCount, 
		bool enableSampleShading,
		ECullMode cullMode, 
		VkPipelineLayout& pipelineLayout, 
		VkPipeline& graphicsPipeline, 
		utility::ErrorState& errorState)
	{
		Material& material = materialInstance.getMaterial();
		const Shader& shader = material.getShader();

		std::vector<VkVertexInputBindingDescription> bindingDescriptions;
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

		// Use the mapping in the material to bind mesh vertex attrs to shader vertex attrs
		uint32_t shader_attribute_binding = 0;
		for (auto& kvp : shader.getAttributes())
		{
			const VertexAttributeDeclaration* shader_vertex_attribute = kvp.second.get();
			bindingDescriptions.push_back({ shader_attribute_binding, (uint32_t)getVertexSize(shader_vertex_attribute->mFormat), VK_VERTEX_INPUT_RATE_VERTEX });
			attributeDescriptions.push_back({ (uint32_t)shader_vertex_attribute->mLocation, shader_attribute_binding, shader_vertex_attribute->mFormat, 0 });

			shader_attribute_binding++;
		}

		VkShaderModule vertShaderModule = shader.getVertexModule();
		VkShaderModule fragShaderModule = shader.getFragmentModule();

		VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		vertexInputInfo.vertexBindingDescriptionCount = (int)bindingDescriptions.size();
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = getTopology(drawMode);
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkDynamicState dynamic_states[2] = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};

		VkPipelineDynamicStateCreateInfo dynamic_state_create_info = {};
		dynamic_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamic_state_create_info.dynamicStateCount = 2;
		dynamic_state_create_info.pDynamicStates = dynamic_states;

		VkPipelineViewportStateCreateInfo viewportState = {};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = nullptr;
		viewportState.scissorCount = 1;
		viewportState.pScissors = nullptr;

		VkPipelineRasterizationStateCreateInfo rasterizer = {};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = getCullMode(cullMode);
		rasterizer.frontFace = windingOrder == ECullWindingOrder::Clockwise ? VK_FRONT_FACE_CLOCKWISE : VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;

		VkPipelineMultisampleStateCreateInfo multisampling = {};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = static_cast<int>(enableSampleShading);
		multisampling.rasterizationSamples = sampleCount;
		multisampling.pNext = nullptr;
		multisampling.flags = 0;
		multisampling.minSampleShading = 1.0f;

		VkPipelineDepthStencilStateCreateInfo depthStencil = getDepthStencilCreateInfo(materialInstance);
		VkPipelineColorBlendAttachmentState colorBlendAttachment = getColorBlendAttachmentState(materialInstance);

		VkPipelineColorBlendStateCreateInfo colorBlending = {};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;

		VkDescriptorSetLayout set_layout = material.getShader().getDescriptorSetLayout();

		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &set_layout;
		pipelineLayoutInfo.pushConstantRangeCount = 0;

		if (!errorState.check(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) == VK_SUCCESS, "Failed to create pipeline layout"))
			return false;

		VkGraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDepthStencilState = &depthStencil;
		pipelineInfo.layout = pipelineLayout;
		pipelineInfo.renderPass = renderPass;
		pipelineInfo.pDynamicState = &dynamic_state_create_info;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		if (!errorState.check(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) == VK_SUCCESS, "Failed to create graphics pipeline"))
			return false;
		return true;
	}


	//////////////////////////////////////////////////////////////////////////
	// Render Service
	//////////////////////////////////////////////////////////////////////////

	RenderService::RenderService(ServiceConfiguration* configuration) :
		Service(configuration) { }


	void RenderService::getDependentServices(std::vector<rtti::TypeInfo>& dependencies)
	{
		dependencies.emplace_back(RTTI_OF(SceneService));
	}


	bool RenderService::addWindow(RenderWindow& window, utility::ErrorState& errorState)
	{
		mWindows.emplace_back(&window);
		windowAdded.trigger(window);
		return true;
	}


	void RenderService::removeWindow(RenderWindow& window)
	{
		WindowList::iterator pos = std::find_if(mWindows.begin(), mWindows.end(), [&](auto val)
		{
			return val == &window;
		});
		assert(pos != mWindows.end());
		windowRemoved.trigger(window);
		mWindows.erase(pos);

	}


	RenderWindow* RenderService::findWindow(void* nativeWindow) const
	{
		WindowList::const_iterator pos = std::find_if(mWindows.begin(), mWindows.end(), [&](auto val)
		{
			return val->getNativeWindow() == nativeWindow;
		});
		return pos != mWindows.end() ? *pos : nullptr;
	}


	RenderWindow* RenderService::findWindow(uint id) const
	{
		WindowList::const_iterator pos = std::find_if(mWindows.begin(), mWindows.end(), [&](auto val)
		{
			return val->getNumber() == id;
		});
		return pos != mWindows.end() ? *pos : nullptr;
	}


	void RenderService::addEvent(WindowEventPtr windowEvent)
	{
		nap::Window* window = findWindow(windowEvent->mWindow);
		assert(window != nullptr);
		window->addEvent(std::move(windowEvent));
	}


	RenderService::Pipeline RenderService::getOrCreatePipeline(IRenderTarget& renderTarget, IMesh& mesh, MaterialInstance& materialInstance, utility::ErrorState& errorState)
	{
		Material& material = materialInstance.getMaterial();
		const Shader& shader = material.getShader();

		EDrawMode draw_mode = mesh.getMeshInstance().getDrawMode();
		ECullMode cull_mode = mesh.getMeshInstance().getCullMode();
		
		// Create pipeline key based on draw properties
		PipelineKey pipeline_key(shader, draw_mode, 
			materialInstance.getDepthMode(), 
			materialInstance.getBlendMode(), 
			renderTarget.getWindingOrder(), 
			renderTarget.getColorFormat(), 
			renderTarget.getDepthFormat(), 
			renderTarget.getSampleCount(), 
			renderTarget.getSampleShadingEnabled(),
			cull_mode);

		// Find key in cache and use previously created pipeline
		PipelineCache::iterator pos = mPipelineCache.find(pipeline_key);
		if (pos != mPipelineCache.end())
			return pos->second;

		// Otherwise create new pipeline
		Pipeline pipeline;
		if (createGraphicsPipeline(mDevice, materialInstance,
			draw_mode,
			renderTarget.getWindingOrder(),
			renderTarget.getRenderPass(),
			renderTarget.getSampleCount(),
			renderTarget.getSampleShadingEnabled(),
			cull_mode,
			pipeline.mLayout, pipeline.mPipeline, errorState))
		{
			mPipelineCache.emplace(std::make_pair(pipeline_key, pipeline));
			return pipeline;
		}

		NAP_ASSERT_MSG(false, "Unable to create new pipeline");
		return Pipeline();
	}

	nap::RenderableMesh RenderService::createRenderableMesh(IMesh& mesh, MaterialInstance& materialInstance, utility::ErrorState& errorState)
	{
		const Material& material = materialInstance.getMaterial();
		const Shader& shader = material.getShader();

		// Verify that this mesh and material combination is valid
		for (auto& kvp : shader.getAttributes())
		{
			const VertexAttributeDeclaration* shader_vertex_attribute = kvp.second.get();

			const Material::VertexAttributeBinding* material_binding = material.findVertexAttributeBinding(kvp.first);
			if (!errorState.check(material_binding != nullptr, "Unable to find binding %s for shader %s in material %s", kvp.first.c_str(), material.getShader().mVertPath.c_str(), material.mID.c_str()))
				return RenderableMesh();

			const VertexAttributeBuffer* vertex_buffer = mesh.getMeshInstance().getGPUMesh().findVertexAttributeBuffer(material_binding->mMeshAttributeID);
			if (!errorState.check(vertex_buffer != nullptr, "Unable to find vertex attribute %s in mesh %s", material_binding->mMeshAttributeID.c_str(), mesh.mID.c_str()))
				return RenderableMesh();

			if (!errorState.check(shader_vertex_attribute->mFormat == vertex_buffer->getFormat(), "Shader vertex attribute format does not match mesh attribute format for attribute %s in mesh %s", material_binding->mMeshAttributeID.c_str(), mesh.mID.c_str()))
			return RenderableMesh();
		}

		return RenderableMesh(mesh, materialInstance);
	}


	// Shut down render service
	RenderService::~RenderService()
	{
		mEmptyTexture.reset();
	}


	// Render all objects in scene graph using specified camera
	void RenderService::renderObjects(IRenderTarget& renderTarget, CameraComponentInstance& camera)
	{
		renderObjects(renderTarget, camera, std::bind(&RenderService::sortObjects, this, std::placeholders::_1, std::placeholders::_2));
	}


	// Render all objects in scene graph using specified camera
	void RenderService::renderObjects(IRenderTarget& renderTarget, CameraComponentInstance& camera, const SortFunction& sortFunction)
	{
		// Get all render-able components
		// Only gather renderable components that can be rendered using the given caera
		std::vector<nap::RenderableComponentInstance*> render_comps;
		std::vector<nap::RenderableComponentInstance*> entity_render_comps;
		for (Scene* scene : mSceneService->getScenes())
		{
			for (EntityInstance* entity : scene->getEntities())
			{
				entity_render_comps.clear();
				entity->getComponentsOfType<nap::RenderableComponentInstance>(entity_render_comps);
				for (const auto& comp : entity_render_comps) 
				{
					if (comp->isSupported(camera))
						render_comps.emplace_back(comp);
				}
			}
		}

		// Render these objects
		renderObjects(renderTarget, camera, render_comps, sortFunction);
	}


	void RenderService::sortObjects(std::vector<RenderableComponentInstance*>& comps, const CameraComponentInstance& camera)
	{
		// Split into front to back and back to front meshes
		std::vector<nap::RenderableComponentInstance*> front_to_back;
		front_to_back.reserve(comps.size());
		std::vector<nap::RenderableComponentInstance*> back_to_front;
		back_to_front.reserve(comps.size());

		for (nap::RenderableComponentInstance* component : comps)
		{
			nap::RenderableMeshComponentInstance* renderable_mesh = rtti_cast<RenderableMeshComponentInstance>(component);
			if (renderable_mesh != nullptr)
			{
				nap::RenderableMeshComponentInstance* renderable_mesh = static_cast<RenderableMeshComponentInstance*>(component);
				EBlendMode blend_mode = renderable_mesh->getMaterialInstance().getBlendMode();	
				if (blend_mode == EBlendMode::AlphaBlend)
					back_to_front.emplace_back(component);
				else
					front_to_back.emplace_back(component);
			}
			else
			{
				front_to_back.emplace_back(component);
			}
		}

		// Sort front to back and render those first
		DepthSorter front_to_back_sorter(DepthSorter::EMode::FrontToBack, camera.getViewMatrix());
		std::sort(front_to_back.begin(), front_to_back.end(), front_to_back_sorter);

		// Then sort back to front and render these
		DepthSorter back_to_front_sorter(DepthSorter::EMode::BackToFront, camera.getViewMatrix());
		std::sort(back_to_front.begin(), back_to_front.end(), back_to_front_sorter);

		// concatinate both in to the output
		comps.clear();
		comps.insert(comps.end(), std::make_move_iterator(front_to_back.begin()), std::make_move_iterator(front_to_back.end()));
		comps.insert(comps.end(), std::make_move_iterator(back_to_front.begin()), std::make_move_iterator(back_to_front.end()));
	}


	void RenderService::renderObjects(IRenderTarget& renderTarget, CameraComponentInstance& camera, const std::vector<RenderableComponentInstance*>& comps)
	{
		renderObjects(renderTarget, camera, comps, std::bind(&RenderService::sortObjects, this, std::placeholders::_1, std::placeholders::_2));
	}


	void RenderService::renderObjects(IRenderTarget& renderTarget, CameraComponentInstance& camera, const std::vector<RenderableComponentInstance*>& comps, const SortFunction& sortFunction)
	{
		assert(mCurrentCommandBuffer != nullptr);	// BeginRendering is not called if this assert is fired	

		// Sort objects to render
		std::vector<RenderableComponentInstance*> components_to_render = comps;
		sortFunction(components_to_render, camera);

		// Before we render, we always set aspect ratio. This avoids overly complex
		// responding to various changes in render target sizes.
		camera.setRenderTargetSize(renderTarget.getBufferSize());

		// Make sure we update our render state associated with the current context
		//updateRenderState();

		// Extract camera projection matrix
		const glm::mat4x4 projection_matrix = camera.getRenderProjectionMatrix();

		// Extract view matrix
		glm::mat4x4 view_matrix = camera.getViewMatrix();

		// Draw components only when camera is supported
		for (auto& comp : components_to_render)
		{
			if (!comp->isSupported(camera))
			{
				nap::Logger::warn("Unable to render component: %s, unsupported camera %s", 
					comp->mID.c_str(), camera.get_type().get_name().to_string().c_str());
				continue;
			}
			comp->draw(renderTarget, mCurrentCommandBuffer, view_matrix, projection_matrix);
		}
	}


	bool RenderService::initEmptyTexture(nap::utility::ErrorState& errorState)
	{
		SurfaceDescriptor settings;
		settings.mWidth = 16;
		settings.mHeight = 16;
		settings.mChannels = ESurfaceChannels::RGBA;
		settings.mDataType = ESurfaceDataType::BYTE;
		
		mEmptyTexture = std::make_unique<Texture2D>(getCore());
		return mEmptyTexture->init(settings, false, Texture2D::EClearMode::FillWithZero,  errorState);
	}


	// Set the currently active renderer
	bool RenderService::init(nap::utility::ErrorState& errorState)
	{
		// Get handle to scene service
		mSceneService = getCore().getService<SceneService>();
		assert(mSceneService != nullptr);

		// Initialize SDL video
		if (!errorState.check(SDL::initVideo(), "Failed to init SDL"))
			return false;

		// Initialize shader compiler
		if (!errorState.check(ShInitialize() != 0, "Failed to initialize shader compiler"))
			return false;

		// Store render settings, used for initialization and global window creation
		mEnableHighDPIMode	= getConfiguration<RenderServiceConfiguration>()->mEnableHighDPIMode;

		// Get available vulkan extensions, necessary for interfacing with native window
		// SDL takes care of this call and returns, next to the default VK_KHR_surface a platform specific extension
		// When initializing the vulkan instance these extensions have to be enabled in order to create a valid
		// surface later on.
		std::vector<std::string> found_extensions;
		SDL_Window* dummy_window = SDL_CreateWindow("Dummy", 0, 0, 32, 32, SDL_WINDOW_VULKAN | SDL_WINDOW_HIDDEN);
		bool got_extensions = getAvailableVulkanInstanceExtensions(dummy_window, found_extensions, errorState);
		SDL_DestroyWindow(dummy_window);
		if (!got_extensions)
			return false;

		// Get available vulkan layer extensions, notify when not all could be found
		std::vector<std::string> found_layers;
#ifndef NDEBUG
		// Get all available vulkan layers
		const std::vector<std::string>& requested_layers = getConfiguration<RenderServiceConfiguration>()->mLayers;
		bool print_layers = getConfiguration<RenderServiceConfiguration>()->mPrintAvailableLayers;
		if (!getAvailableVulkanLayers(requested_layers, print_layers, found_layers, errorState))
			return false;

		// Warn when not all requested layers could be found
		if (found_layers.size() != requested_layers.size())
			nap::Logger::warn("Not all requested layers were found");

		// Print the ones we're enabling
		for (const auto& layer : found_layers)
			Logger::info("Applying layer: %s", layer.c_str());
#endif // NDEBUG

		// Create Vulkan Instance together with required extensions and layers
		if (!createVulkanInstance(found_layers, found_extensions, mInstance, mAPIVersion, errorState))
			return false;

		// Vulkan messaging callback
		setupDebugCallback(mInstance, mDebugCallback, errorState);

		// Select GPU after successful creation of a vulkan instance
		VkPhysicalDeviceFeatures	physical_device_features;
		VkPhysicalDeviceProperties	physical_device_properties;

		VkPhysicalDeviceType pref_gpu = getPhysicalDeviceType(getConfiguration<RenderServiceConfiguration>()->mPreferredGPU);
		if (!selectPhysicalDevice(mInstance, pref_gpu, mAPIVersion, mPhysicalDevice, mPhysicalDeviceProperties, mPhysicalDeviceFeatures, mGraphicsQueueIndex, errorState))
			return false;

		// Figure out how many rasterization samples we can use and if sample rate shading is supported
		mMaxRasterizationSamples = getMaxSampleCount(mPhysicalDevice);
		nap::Logger::info("Max number of rasterization samples: %d", (int)(mMaxRasterizationSamples));
		mSampleShadingSupported = mPhysicalDeviceFeatures.sampleRateShading > 0;
		nap::Logger::info("Sample rate shading: %s", mSampleShadingSupported ? "Supported" : "Not Supported");

		// Create unique set of extensions out of required and additional requested ones
		std::vector<std::string> required_ext_names = getRequiredDeviceExtensionNames();
		std::vector<std::string> addition_ext_names = getConfiguration<RenderServiceConfiguration>()->mAdditionalExtensions;
		required_ext_names.insert(required_ext_names.end(), addition_ext_names.begin(), addition_ext_names.end());
		std::unordered_set<std::string> unique_ext_names(required_ext_names.size());
		for (const auto& ext : required_ext_names)
			unique_ext_names.emplace(ext);

		// Create a logical device that interfaces with the physical device.
		bool print_extensions = getConfiguration<RenderServiceConfiguration>()->mPrintAvailableExtensions;
		if (!createLogicalDevice(mPhysicalDevice, mPhysicalDeviceFeatures, mGraphicsQueueIndex, found_layers, unique_ext_names, print_extensions, mDevice, errorState))
			return false;

		// Create command pool
		if (!errorState.check(createCommandPool(mPhysicalDevice, mDevice, mGraphicsQueueIndex, mCommandPool), "Failed to create Command Pool"))
			return false;

		// 
		if (!errorState.check(findDepthFormat(mPhysicalDevice, mDepthFormat), "Unable to find depth format"))
			return false;

		vkGetDeviceQueue(mDevice, mGraphicsQueueIndex, 0, &mGraphicsQueue);

		VmaAllocatorCreateInfo allocatorInfo = {};
		allocatorInfo.physicalDevice = mPhysicalDevice;
		allocatorInfo.device = mDevice;

		if (!errorState.check(vmaCreateAllocator(&allocatorInfo, &mVulkanAllocator) == VK_SUCCESS, "Failed to create Vulkan Memory Allocator"))
			return false;

		mDescriptorSetAllocator = std::make_unique<DescriptorSetAllocator>(mDevice);

		if (!initEmptyTexture(errorState))
			return false;
		
		mFramesInFlight.resize(getMaxFramesInFlight());
		for (int frame_index = 0; frame_index != mFramesInFlight.size(); ++frame_index)
		{
			Frame& frame = mFramesInFlight[frame_index];
			if (!createFence(mDevice, frame.mFence, errorState))
				return false;

			if (!createCommandBuffer(mDevice, mCommandPool, frame.mUploadCommandBuffer, errorState))
				return false;

			if (!createCommandBuffer(mDevice, mCommandPool, frame.mDownloadCommandBuffers, errorState))
				return false;

			if (!createCommandBuffer(mDevice, mCommandPool, frame.mHeadlessCommandBuffers, errorState))
				return false;
		}

		mInitialized = true;
		return true;
	}

	void RenderService::waitDeviceIdle()
	{
		// When we're shutting down or realtime editing, we need to ensure all vulkan resources are no longer in use
		// Otherwise, during resource destruction, the vulkan specific resources will not be freed correctly.
		// To do this, we wait for the device to be idle
		VkResult result = vkDeviceWaitIdle(mDevice);
		assert(result == VK_SUCCESS);

		// Now that we know the device is idle, we can destroy any currently queued vulkan objects, since they're
		// guaranteed to no longer be in use
		for (int frameIndex = 0; frameIndex < mFramesInFlight.size(); ++frameIndex)
			processVulkanDestructors(frameIndex);

		// Since we know the device is idle at this point, we can destroy vulkan objects without going
		// through the queue. This is reset when beginFrame is called again.
		mCanDestroyVulkanObjectsImmediately = true;
	}


	void RenderService::waitForFence(int frameIndex)
	{
		vkWaitForFences(mDevice, 1, &mFramesInFlight[frameIndex].mFence, VK_TRUE, UINT64_MAX);
	}


	void RenderService::getFormatProperties(VkFormat format, VkFormatProperties& outProperties)
	{
		vkGetPhysicalDeviceFormatProperties(mPhysicalDevice, format, &outProperties);
	}


	uint32 RenderService::getVulkanVersionMajor() const
	{
		return VK_VERSION_MAJOR(mAPIVersion);
	}


	uint32 RenderService::getVulkanVersionMinor() const
	{
		return VK_VERSION_MINOR(mAPIVersion);
	}


	void RenderService::preShutdown()
	{
		waitDeviceIdle();
	}


	void RenderService::preResourcesLoaded()
	{
		waitDeviceIdle();
	}


	// Shut down renderer
	void RenderService::shutdown()
	{
		for (auto kvp : mPipelineCache)
		{
			vkDestroyPipeline(mDevice, kvp.second.mPipeline, nullptr);
			vkDestroyPipelineLayout(mDevice, kvp.second.mLayout, nullptr);
		}
		mPipelineCache.clear();

		for (Frame& frame : mFramesInFlight)
		{
			assert(frame.mQueuedVulkanObjectDestructors.empty());
			vkFreeCommandBuffers(mDevice, mCommandPool, 1, &frame.mHeadlessCommandBuffers);
			vkFreeCommandBuffers(mDevice, mCommandPool, 1, &frame.mUploadCommandBuffer);
			vkFreeCommandBuffers(mDevice, mCommandPool, 1, &frame.mDownloadCommandBuffers);
			vkDestroyFence(mDevice, frame.mFence, nullptr);
		}

		mFramesInFlight.clear();
		mEmptyTexture.reset();
		mDescriptorSetCaches.clear();
		mDescriptorSetAllocator.reset();

		if (mVulkanAllocator != nullptr)
		{
			vmaDestroyAllocator(mVulkanAllocator);
			mVulkanAllocator = nullptr;
		}

		if (mCommandPool != nullptr)
		{
			vkDestroyCommandPool(mDevice, mCommandPool, nullptr);
			mCommandPool = nullptr;
		}

		if (mDevice != nullptr)
		{
			vkDestroyDevice(mDevice, nullptr);
			mDevice = nullptr;
		}

		if (mDebugCallback != nullptr)
		{
			destroyDebugReportCallbackEXT(mInstance, mDebugCallback);
			mDebugCallback = nullptr;
		}

		if (mInstance != nullptr)
		{
			vkDestroyInstance(mInstance, nullptr);
			mInstance = nullptr;
		}

		ShFinalize();
		SDL::shutdownVideo();
		mInitialized = false;
	}
	
	void RenderService::transferData(VkCommandBuffer commandBuffer, const std::function<void()>& transferFunction)
	{
		vkResetCommandBuffer(commandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		// Begin recording commands
		VkResult result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
		assert(result == VK_SUCCESS);

		// Perform transfer
		transferFunction();

		// End recoding commands
		result = vkEndCommandBuffer(commandBuffer);
		assert(result == VK_SUCCESS);

		// Submit command queue
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;
		result = vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		assert(result == VK_SUCCESS);
	}


	void RenderService::uploadData()
	{
		// Fetch upload command buffer to use
		VkCommandBuffer commandBuffer = mFramesInFlight[mCurrentFrameIndex].mUploadCommandBuffer;

		// Transfer data to the GPU, including texture data and general purpose render buffers.
		transferData(commandBuffer, [commandBuffer, this]()
		{
			for (Texture2D* texture : mTexturesToUpload)
				texture->upload(commandBuffer);
			mTexturesToUpload.clear();

			for (GPUBuffer* buffer : mBuffersToUpload)
				buffer->upload(commandBuffer);
			mBuffersToUpload.clear();
		});
	}


	void RenderService::downloadData()
	{
		// Push the download of a texture onto the commandbuffer
		Frame& frame = mFramesInFlight[mCurrentFrameIndex];
		VkCommandBuffer commandBuffer = frame.mDownloadCommandBuffers;
		transferData(commandBuffer, [commandBuffer, this, &frame]()
		{
			for (Texture2D* texture : frame.mTextureDownloads)
				texture->download(commandBuffer);
		});
	}


	void RenderService::updateTextureDownloads()
	{
		// Here we check if any pending texture downloads are ready. We always know for sure that textures
		// for the current frame are ready (if this function is called after wait for fence). So we could just
		// call notify for all outstanding texture request for the current frame. However, this could mean that
		// we are waiting longer than necessary. For example, if we have 3 frames in flight, but the texture is 
		// ready one frame after the download started, we could already notify the texture. For this reason we
		// check the fence status of all frames. If a frame has been completed on GPU, we know for sure that the
		// upload has finished as well, and we can notify the texture that it has finished.
		for (int frame_index = 0; frame_index != mFramesInFlight.size(); ++frame_index)
		{
			Frame& frame = mFramesInFlight[frame_index];
			if (!frame.mTextureDownloads.empty() && vkGetFenceStatus(mDevice, frame.mFence) == VK_SUCCESS)
			{
				for (Texture2D* texture : frame.mTextureDownloads)
					texture->notifyDownloadReady(frame_index);

				frame.mTextureDownloads.clear();
			}
		}
	}


	void RenderService::processVulkanDestructors(int frameIndex)
	{
		for (VulkanObjectDestructor& destructor : mFramesInFlight[frameIndex].mQueuedVulkanObjectDestructors)
			destructor(*this);

		mFramesInFlight[frameIndex].mQueuedVulkanObjectDestructors.clear();
	}


	void RenderService::beginFrame()
	{
		// When we start rendering a frame, we cannot destroy vulkan objects immediately, they must be pushed on the
		// destructor queue instead. This flag is set to true for cases of real-time editing and the destruction sequence,
		// where Vulkan object must be destroyed immediately.
		mCanDestroyVulkanObjectsImmediately = false;
		mIsRenderingFrame = true;

		vkWaitForFences(mDevice, 1, &mFramesInFlight[mCurrentFrameIndex].mFence, VK_TRUE, UINT64_MAX);

		// We call updateTextureDownloads after we have waited for the fence. Otherwise it may happen that we check the fence
		// status which could still not be signaled at that point, causing the notify not to be called. If we then wait for
		// the fence anyway, we missed the opportunity to notify textures that downloads were ready. Because we reset the fence
		// next, we could delay the notification for a full frame cycle. So this call is purposely put inbetween the wait and reset
		// of the fence.
		updateTextureDownloads();

		for (auto& kvp : mDescriptorSetCaches)
			kvp.second->release(mCurrentFrameIndex);

		// Destroy all vulkan resources associated with current frame
		processVulkanDestructors(mCurrentFrameIndex);

		// Upload data before rendering new set
		uploadData();
	}


	void RenderService::endFrame()
	{
		// We reset the fences at the end of the frame to make sure that multiple waits on the same fence (using WaitForFence) complete correctly.
		vkResetFences(mDevice, 1, &mFramesInFlight[mCurrentFrameIndex].mFence);

		// Push any texture downloads on the commandbuffer
		downloadData();

		// We perform a no-op submit that will ensure that a fence will be signaled when all of the commands for all of 
		// the commandbuffers that we submitted will be completed. This is how we can synchronize the CPU frame to the GPU.
		vkQueueSubmit(mGraphicsQueue, 0, VK_NULL_HANDLE, mFramesInFlight[mCurrentFrameIndex].mFence);
		mCurrentFrameIndex = (mCurrentFrameIndex + 1) % 2;

		mIsRenderingFrame = false;
	}


	bool RenderService::beginHeadlessRecording()
	{
		assert(mCurrentCommandBuffer == nullptr);
		mCurrentCommandBuffer = mFramesInFlight[mCurrentFrameIndex].mHeadlessCommandBuffers;
		vkResetCommandBuffer(mCurrentCommandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		VkResult result = vkBeginCommandBuffer(mCurrentCommandBuffer, &beginInfo);
		assert(result == VK_SUCCESS);
		return true;
	}


	void RenderService::endHeadlessRecording()
	{
		assert(mCurrentCommandBuffer != nullptr);
		VkResult result = vkEndCommandBuffer(mCurrentCommandBuffer);
		assert(result == VK_SUCCESS);

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &mCurrentCommandBuffer;

		result = vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		assert(result == VK_SUCCESS);
		mCurrentCommandBuffer = nullptr;
	}


	bool RenderService::beginRecording(RenderWindow& renderWindow)
	{
		assert(mCurrentCommandBuffer == nullptr);
		assert(mCurrentRenderWindow == nullptr);

		mCurrentCommandBuffer = renderWindow.makeActive();
		if (mCurrentCommandBuffer == nullptr)
			return false;

		mCurrentRenderWindow = &renderWindow;
		return true;
	}

	void RenderService::endRecording()
	{
		assert(mCurrentCommandBuffer != nullptr);
		assert(mCurrentRenderWindow != nullptr);
		mCurrentRenderWindow->swap();
		mCurrentCommandBuffer = nullptr;
		mCurrentRenderWindow = nullptr;
	}


	void RenderService::queueVulkanObjectDestructor(const VulkanObjectDestructor& function)
	{
		// If it's possible to destroy vulkan objects immediately (i.e. when we know the device is idle during shutdown/realtime editing),
		// we call the lambda immediately.
		if (mCanDestroyVulkanObjectsImmediately)
		{
			function(*this);
			return;
		}

		// Otherwise, we queue the lamdba on the destructor queue. Note that we use the *previous* frame index to
		// add the object to. This is to ensure that objects can be queued for destruction outside of the render frame (i.e during update).	
		assert(isInitialized());
		int previousFrameIndex =  mIsRenderingFrame ? mCurrentFrameIndex : mCurrentFrameIndex - 1;
		if (previousFrameIndex < 0)
			previousFrameIndex = mFramesInFlight.size() - 1;

		mFramesInFlight[previousFrameIndex].mQueuedVulkanObjectDestructors.emplace_back(function);
	}


	void RenderService::update(double deltaTime)
	{
		for (const auto& window : mWindows)
		{
			window->processEvents();
		}
	}


	DescriptorSetCache& RenderService::getOrCreateDescriptorSetCache(VkDescriptorSetLayout layout)
	{
		DescriptorSetCacheMap::iterator pos = mDescriptorSetCaches.find(layout);
		if (pos != mDescriptorSetCaches.end())
			return *pos->second;

		std::unique_ptr<DescriptorSetCache> allocator = std::make_unique<DescriptorSetCache>(*this, layout, *mDescriptorSetAllocator);
		auto inserted = mDescriptorSetCaches.insert(std::make_pair(layout, std::move(allocator)));
		return *inserted.first->second;
	}


	void RenderService::removeTextureRequests(Texture2D& texture)
	{
		// When textures are destroyed, we also need to remove any pending texture requests
		mTexturesToUpload.erase(&texture);

		for (Frame& frame : mFramesInFlight)
		{
			frame.mTextureDownloads.erase(std::remove_if(frame.mTextureDownloads.begin(), frame.mTextureDownloads.end(), [&texture](Texture2D* existingTexture)
			{
				return existingTexture == &texture;
			}), frame.mTextureDownloads.end());
		}
	}


	void RenderService::requestTextureUpload(Texture2D& texture)
	{
		mTexturesToUpload.insert(&texture);
	}


	void RenderService::requestTextureDownload(Texture2D& texture)
	{
		// We push a texture download specifically for this frame. When the fence for that frame is signaled,
		// we now the download has been processed by the GPU, and we can send the texture a notification that
		// transfer has completed.
		mFramesInFlight[mCurrentFrameIndex].mTextureDownloads.push_back(&texture);
	}


	void RenderService::removeBufferRequests(GPUBuffer& buffer)
	{
		// When buffers are destroyed, we also need to remove any pending upload requests
		mBuffersToUpload.erase(&buffer);
	}


	void RenderService::requestBufferUpload(GPUBuffer& buffer)
	{
		mBuffersToUpload.insert(&buffer);
	}


	VkSampleCountFlagBits RenderService::getMaxRasterizationSamples() const
	{
		return mMaxRasterizationSamples;
	}


	bool RenderService::getRasterizationSamples(ERasterizationSamples requestedSamples, VkSampleCountFlagBits& outSamples, nap::utility::ErrorState& errorState)
	{
		outSamples = requestedSamples == ERasterizationSamples::Max ? mMaxRasterizationSamples :
			(int)(requestedSamples) > (int)mMaxRasterizationSamples ? mMaxRasterizationSamples : (VkSampleCountFlagBits)(requestedSamples);

		return errorState.check((int)requestedSamples <= (int)mMaxRasterizationSamples,
			"Requested rasterization sample count of: %d exceeds hardware limit of: %d", (int)(requestedSamples), (int)mMaxRasterizationSamples);
	}


	bool RenderService::sampleShadingSupported() const
	{
		return mSampleShadingSupported;
	}


	VkImageAspectFlags RenderService::getDepthAspectFlags() const
	{
		return mDepthFormat != VK_FORMAT_D32_SFLOAT ? 
			VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT : 
			VK_IMAGE_ASPECT_DEPTH_BIT;
	}
}

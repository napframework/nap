/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

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
#include <mathutils.h>

RTTI_BEGIN_ENUM(nap::RenderServiceConfiguration::EPhysicalDeviceType)
	RTTI_ENUM_VALUE(nap::RenderServiceConfiguration::EPhysicalDeviceType::Integrated,	"Integrated"),
	RTTI_ENUM_VALUE(nap::RenderServiceConfiguration::EPhysicalDeviceType::Discrete,		"Discrete"),
	RTTI_ENUM_VALUE(nap::RenderServiceConfiguration::EPhysicalDeviceType::Virtual,		"Integrated"),
	RTTI_ENUM_VALUE(nap::RenderServiceConfiguration::EPhysicalDeviceType::CPU,			"CPU")
RTTI_END_ENUM

RTTI_BEGIN_CLASS(nap::RenderServiceConfiguration)
	RTTI_PROPERTY("Headless",			&nap::RenderServiceConfiguration::mHeadless,					nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("PreferredGPU",		&nap::RenderServiceConfiguration::mPreferredGPU,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Layers",				&nap::RenderServiceConfiguration::mLayers,						nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Extensions",			&nap::RenderServiceConfiguration::mAdditionalExtensions,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("VulkanMajor",		&nap::RenderServiceConfiguration::mVulkanVersionMajor,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("VulkanMinor",		&nap::RenderServiceConfiguration::mVulkanVersionMinor,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("EnableHighDPI",		&nap::RenderServiceConfiguration::mEnableHighDPIMode,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ShowLayers",			&nap::RenderServiceConfiguration::mPrintAvailableLayers,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ShowExtensions",		&nap::RenderServiceConfiguration::mPrintAvailableExtensions,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("AnisotropicSamples",	&nap::RenderServiceConfiguration::mAnisotropicFilterSamples,	nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderService)
	RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// Dummy Window Wrapper
	//////////////////////////////////////////////////////////////////////////
	
	/**
	 * Used by the render service to temporarily bind and destroy information.
	 * This information is required by the render service (on initialization) to extract
	 * all required Vulkan surface extensions and select a queue that can present images,
	 * next to render and transfer functionality.
	 */
	struct DummyWindow
	{
		~DummyWindow()
		{
			if (mSurface != VK_NULL_HANDLE)		{ assert(mInstance != nullptr);  vkDestroySurfaceKHR(mInstance, mSurface, nullptr); }
			if (mWindow != nullptr)				{ SDL_DestroyWindow(mWindow); }
		}
		SDL_Window*	mWindow = nullptr;
		VkInstance	mInstance = VK_NULL_HANDLE;
		VkSurfaceKHR mSurface = VK_NULL_HANDLE;
	};


	//////////////////////////////////////////////////////////////////////////
	// Compatible physical device
	//////////////////////////////////////////////////////////////////////////




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
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(physicalDevice, &properties);
		VkSampleCountFlags counts = properties.limits.framebufferColorSampleCounts & properties.limits.framebufferDepthSampleCounts;
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
		const static std::vector<std::string> layers = 
		{ 
			VK_KHR_MAINTENANCE1_EXTENSION_NAME
		};
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
		nap::Logger::warn("Vulkan Layer [%s]:\n%s", layerPrefix, msg);
		return VK_FALSE;
	}


	/**
	 * Creates a debug report callback object
	 */
	static VkResult createDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback)
	{
		auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
		return func != VK_NULL_HANDLE ? func(instance, pCreateInfo, pAllocator, pCallback) : VK_ERROR_EXTENSION_NOT_PRESENT;
	}


	/**
	 * Destroys a debug report callback object
	 */
	static void destroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT pCallback)
	{
		auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
		if (func != VK_NULL_HANDLE)
			func(instance, pCallback, nullptr);
	}


	/**
	 * Sets up the vulkan messaging callback specified above
	 */
	static bool setupDebugCallback(VkInstance instance, VkDebugReportCallbackEXT& callback, utility::ErrorState& errorState)
	{
		VkDebugReportCallbackCreateInfoEXT create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		create_info.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
		create_info.pfnCallback = debugCallback;

		if (!errorState.check(createDebugReportCallbackEXT(instance, &create_info, nullptr, &callback) == VK_SUCCESS, "Unable to create debug report callback extension"))
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
		uint32 instance_layer_count = 0;
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


	static bool getSurfaceInstanceExtensions(SDL_Window* window, std::vector<std::string>& outExtensions, utility::ErrorState& errorState)
	{
		// Figure out the amount of extensions vulkan needs to interface with the os windowing system 
		// This is necessary because vulkan is a platform agnostic API and needs to know how to interface with the windowing system
		unsigned int ext_count = 0;
		if (!errorState.check(SDL_Vulkan_GetInstanceExtensions(window, &ext_count, nullptr) == SDL_TRUE, "Unable to find any valid SDL Vulkan instance extensions, is the Vulkan driver installed?"))
			return false;

		// Use the amount of extensions queried before to retrieve the names of the extensions
		std::vector<const char*> ext_names(ext_count);
		if (!errorState.check(SDL_Vulkan_GetInstanceExtensions(window, &ext_count, ext_names.data()) == SDL_TRUE, "Unable to get the number of SDL Vulkan extension names"))
			return false;

		// Store
		for (unsigned int i = 0; i < ext_count; i++)
			outExtensions.emplace_back(ext_names[i]);
		return true;
	}


	/**
	 * Creates the vulkan surface that is rendered to by the device using SDL
	 */
	static bool createSurface(SDL_Window* window, VkInstance instance, VkSurfaceKHR& outSurface, utility::ErrorState& errorState)
	{
		// Use SDL to create the surface
		if (!errorState.check(SDL_Vulkan_CreateSurface(window, instance, &outSurface) == SDL_TRUE, "Unable to create Vulkan compatible surface using SDL"))
			return false;
		return true;
	}


	/**
	* Creates a vulkan instance using all the available instance extensions and layers
	* @return if the instance was created successfully
	*/
	static bool createVulkanInstance(const std::vector<std::string>& layerNames, const std::vector<std::string>& extensionNames, uint32 requestedVersion, VkInstance& outInstance, utility::ErrorState& errorState)
	{
		// Copy layers
		std::vector<const char*> layer_names;
		layer_names.reserve(layerNames.size());
		for (const auto& layer : layerNames)
			layer_names.emplace_back(layer.c_str());

		// Copy extensions
		std::vector<const char*> ext_names;
		ext_names.reserve(extensionNames.size());
		for (const auto& ext : extensionNames)
			ext_names.emplace_back(ext.c_str());

		// Get the supported vulkan instance version, only supported by newer (1.1) loaders.
		// We therefore first find out if the function is exposed, if so use it.
		uint32 instance_version(0);
		PFN_vkEnumerateInstanceVersion enum_instance_version_fn = (PFN_vkEnumerateInstanceVersion)vkGetInstanceProcAddr(VK_NULL_HANDLE, "vkEnumerateInstanceVersion");
		if (enum_instance_version_fn != nullptr)
		{
			if (!errorState.check(enum_instance_version_fn(&instance_version) == VK_SUCCESS,
				"Unable Query instance-level version of Vulkan before instance creation"))
				return false;
		}
		else 
		{
			// Otherwise current version is known to be 1.0
			instance_version = VK_MAKE_VERSION(1, 0, 0);
		}

		// Log used SDK version
		uint32 major_version = VK_VERSION_MAJOR(instance_version);
		uint32 minor_version = VK_VERSION_MINOR(instance_version);
		uint32 patch_version = VK_VERSION_PATCH(instance_version);
		nap::Logger::info("Vulkan instance version: %d.%d.%d", major_version, minor_version, patch_version);

		uint32 req_version_major = VK_VERSION_MAJOR(requestedVersion);
		uint32 req_version_minor = VK_VERSION_MINOR(requestedVersion);
		nap::Logger::info("Vulkan requested version: %d.%d.%d", req_version_major, req_version_minor, 0);
		
		// Ensure the found instance version is compatible
		if (!errorState.check(instance_version >= requestedVersion, "Incompatible Vulkan instance, min required version: %d.%d",
			req_version_major, req_version_minor))
			return false;

		// initialize the VkApplicationInfo structure
		VkApplicationInfo app_info = {};
		app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		app_info.pNext = NULL;
		app_info.pApplicationName = "NAP";
		app_info.applicationVersion = 1;
		app_info.pEngineName = "NAP";
		app_info.engineVersion = 1;
		app_info.apiVersion = requestedVersion;

		// initialize the VkInstanceCreateInfo structure
		VkInstanceCreateInfo inst_info = {};
		inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		inst_info.pNext = NULL;
		inst_info.flags = 0;
		inst_info.pApplicationInfo = &app_info;
		inst_info.enabledExtensionCount = static_cast<uint32_t>(ext_names.size());
		inst_info.ppEnabledExtensionNames = ext_names.empty() ? nullptr : ext_names.data();
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
	 * Selects a device based on user preference, min required api version and queue family requirements.
	 * If a surface is provided (is not VK_NULL_HANDLE), the queue must also support presentation to that given type of surface.
	 */
	static bool selectPhysicalDevice(VkInstance instance, VkPhysicalDeviceType preferredType, uint32 minAPIVersion, VkSurfaceKHR presentSurface, PhysicalDevice& outDevice ,utility::ErrorState& errorState)
	{
		// Get number of available physical devices, needs to be at least 1
		uint32 physical_device_count(0);
		vkEnumeratePhysicalDevices(instance, &physical_device_count, nullptr);
		if (!errorState.check(physical_device_count != 0, "No physical device (GPU) found"))
			return false;

		// Now get the devices
		std::vector<VkPhysicalDevice> physical_devices(physical_device_count);
		vkEnumeratePhysicalDevices(instance, &physical_device_count, physical_devices.data());

		// Show device information
		Logger::info("Found %d GPU(s):", physical_device_count);
		
		// Pick the right GPU, based on type preference, api-version and queue support
		std::vector<VkPhysicalDeviceProperties> physical_device_properties(physical_devices.size());
		VkQueueFlags required_flags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT;

		// All valid physical devices
		std::vector<PhysicalDevice> valid_devices;

		// Iterate over every available physical device and gather valid ones.
		// A valid physical device has a graphics and compute queue, if presentSurface != NULL,
		// present functionality is also required.
		int preferred_idx = -1;
		for (int device_idx = 0; device_idx < physical_devices.size(); ++device_idx)
		{
			// Get current physical device
			VkPhysicalDevice physical_device = physical_devices[device_idx];

			// Get properties associated with device
			VkPhysicalDeviceProperties& properties = physical_device_properties[device_idx];
			vkGetPhysicalDeviceProperties(physical_device, &properties);
			Logger::info("%d: %s, type: %s, version: %d.%d", device_idx, properties.deviceName, getDeviceTypeName(properties.deviceType).c_str(), VK_VERSION_MAJOR(properties.apiVersion), VK_VERSION_MINOR(properties.apiVersion));

			// If the supported api version < required by currently used api, continue
			if (properties.apiVersion < minAPIVersion)
			{
				Logger::warn("%d: Incompatible driver, min required api version: %d.%d", device_idx, VK_VERSION_MAJOR(minAPIVersion), VK_VERSION_MINOR(minAPIVersion));
				continue;
			}

			// Find the number queues this device supports
			uint32 family_queue_count(0);
			vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &family_queue_count, nullptr);
			if (family_queue_count == 0)
			{
				Logger::warn("%d: No queue families available", device_idx);
				continue;
			}

			// Extract the properties of all the queue families
			// We want to make sure that we have a queue that supports the required flags
			std::vector<VkQueueFamilyProperties> queue_properties(family_queue_count);
			vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &family_queue_count, queue_properties.data());

			// Make sure the queue family supports both graphical and transfer commands.
			int selected_queue_idx = -1;
			for (uint32 i = 0; i < family_queue_count; i++)
			{
				if (queue_properties[i].queueFlags & required_flags)
				{
					// Make sure this family supports presentation to the given surface
					// If running headless this check is not performed.
					if (presentSurface != VK_NULL_HANDLE)
					{
						VkBool32 supports_presentation = false;
						vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, presentSurface, &supports_presentation);
						if (supports_presentation == 0)
							continue;
					}

					// Compatible
					Logger::info("%d: Compatible queue found at index: %d", device_idx, i);
					selected_queue_idx = i;
					break;
				}
			}
			
			// Ensure there's a compatible queue for this device
			if (selected_queue_idx < 0)
			{
				Logger::warn("%d: Unable to find compatible queue family", device_idx);
				continue;
			}

			// Add it as a compatible device
			valid_devices.emplace_back(PhysicalDevice(physical_device, properties, selected_queue_idx));

			// Check if it's the preferred type, if so select it.
			preferred_idx = properties.deviceType == preferredType && preferred_idx < 0 ? 
				valid_devices.size() -1 : preferred_idx;
		}

		// if there are no valid devices, bail.
		if (!errorState.check(!valid_devices.empty(), "No compatible device found"))
			return false;

		// If the preferred GPU is found, use that one, otherwise first compatible one
		int selected_idx = preferred_idx;
		if (preferred_idx < 0)
		{
			nap::Logger::warn("Unable to find preferred device, selecting first compatible one");
			selected_idx = 0;
		}

		// Set the output
		outDevice = valid_devices[selected_idx];
		nap::Logger::info("Selected device: %d", selected_idx, outDevice.getProperties().deviceName);
		return true;
	}


	/**
	 * Creates the logical device based on the selected physical device, queue index and required extensions
	 */
	static bool createLogicalDevice(const PhysicalDevice& physicalDevice, const std::vector<std::string>& layerNames, const std::unordered_set<std::string>& extensionNames, bool print, VkDevice& outDevice, utility::ErrorState& errorState)
	{
		// Copy layer names
		std::vector<const char*> layer_names;
		for (const auto& layer : layerNames)
			layer_names.emplace_back(layer.c_str());

		// Get the number of available extensions for our graphics card
		uint32_t device_property_count(0);
		if (!errorState.check(vkEnumerateDeviceExtensionProperties(physicalDevice.getHandle(), NULL, &device_property_count, NULL) == VK_SUCCESS, "Unable to acquire device extension property count"))
			return false;
		if (print) { Logger::info("Found %d Vulkan device extensions:", device_property_count); }

		// Acquire their actual names
		std::vector<VkExtensionProperties> device_properties(device_property_count);
		if (!errorState.check(vkEnumerateDeviceExtensionProperties(physicalDevice.getHandle(), NULL, &device_property_count, device_properties.data()) == VK_SUCCESS, "Unable to acquire device extension property names"))
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
		VkDeviceQueueCreateInfo queue_create_info = { };
		queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_create_info.queueFamilyIndex = physicalDevice.getQueueIndex();
		queue_create_info.queueCount = 1;
		std::vector<float> queue_prio = { 1.0f };
		queue_create_info.pQueuePriorities = queue_prio.data();
		queue_create_info.pNext = nullptr;
		queue_create_info.flags = 0;

		// Enable specific features, we could also enable all supported features here.
		VkPhysicalDeviceFeatures device_features {0};
		device_features.sampleRateShading = physicalDevice.getFeatures().sampleRateShading;
		device_features.samplerAnisotropy = physicalDevice.getFeatures().samplerAnisotropy;
		device_features.largePoints = physicalDevice.getFeatures().largePoints;
		device_features.wideLines = physicalDevice.getFeatures().wideLines;

		// Device creation information	
		VkDeviceCreateInfo create_info = { };
		create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		create_info.queueCreateInfoCount = 1;
		create_info.pQueueCreateInfos = &queue_create_info;
		create_info.ppEnabledLayerNames = layer_names.data();
		create_info.enabledLayerCount = static_cast<uint32_t>(layer_names.size());
		create_info.ppEnabledExtensionNames = device_property_names.data();
		create_info.enabledExtensionCount = static_cast<uint32_t>(device_property_names.size());
		create_info.pNext = nullptr;
		create_info.pEnabledFeatures = &device_features;
		create_info.flags = 0;

		// Finally we're ready to create a new device
		if (!errorState.check(vkCreateDevice(physicalDevice.getHandle(), &create_info, nullptr, &outDevice) == VK_SUCCESS, "Failed to create logical device"))
			return false;

		return true;
	}


	/**
	 * Creates a command pool associated with the given queue index
	 */
	static bool createCommandPool(VkPhysicalDevice physicalDevice, VkDevice device, uint32 graphicsQueueIndex, VkCommandPool& commandPool)
	{
		VkCommandPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		pool_info.queueFamilyIndex = graphicsQueueIndex;
		pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		return vkCreateCommandPool(device, &pool_info, nullptr, &commandPool) == VK_SUCCESS;
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
		VkCommandBufferAllocateInfo alloc_info = {};
		alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		alloc_info.commandPool = commandPool;
		alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		alloc_info.commandBufferCount = 1;

		if (!errorState.check(vkAllocateCommandBuffers(device, &alloc_info, &commandBuffer) == VK_SUCCESS, "Failed to allocate command buffer"))
			return false;

		return true;
	}


	/**
	 * Create CPU - GPU synchronization primitive
	 */
	static bool createFence(VkDevice device, VkFence& fence, utility::ErrorState& errorState)
	{
		VkFenceCreateInfo fence_info = {};
		fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		return errorState.check(vkCreateFence(device, &fence_info, nullptr, &fence) == VK_SUCCESS, "Failed to create sync objects");
	}


	/**
	 * Get vulkan depth and stencil creation information, based on current material state.
	 */
	static VkPipelineDepthStencilStateCreateInfo getDepthStencilCreateInfo(const MaterialInstance& materialInstance)
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
	static VkPipelineColorBlendAttachmentState getColorBlendAttachmentState(const MaterialInstance& materialInstance)
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
		const MaterialInstance& materialInstance, 
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
		const Material& material = materialInstance.getMaterial();
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

		VkShaderModule vert_shader_module = shader.getVertexModule();
		VkShaderModule frag_shader_module = shader.getFragmentModule();

		VkPipelineShaderStageCreateInfo vert_shader_stage_info = {};
		vert_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vert_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vert_shader_stage_info.module = vert_shader_module;
		vert_shader_stage_info.pName = "main";

		VkPipelineShaderStageCreateInfo frag_shader_stage_info = {};
		frag_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		frag_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		frag_shader_stage_info.module = frag_shader_module;
		frag_shader_stage_info.pName = "main";

		VkPipelineShaderStageCreateInfo shader_stages[] = { vert_shader_stage_info, frag_shader_stage_info };

		VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
		vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		vertex_input_info.vertexBindingDescriptionCount = (int)bindingDescriptions.size();
		vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertex_input_info.pVertexBindingDescriptions = bindingDescriptions.data();
		vertex_input_info.pVertexAttributeDescriptions = attributeDescriptions.data();

		VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
		input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		input_assembly.topology = getTopology(drawMode);
		input_assembly.primitiveRestartEnable = VK_FALSE;

		VkDynamicState dynamic_states[3] = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR,
			VK_DYNAMIC_STATE_LINE_WIDTH
		};

		VkPipelineDynamicStateCreateInfo dynamic_state_create_info = {};
		dynamic_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamic_state_create_info.dynamicStateCount = 3;
		dynamic_state_create_info.pDynamicStates = dynamic_states;

		VkPipelineViewportStateCreateInfo viewport_state = {};
		viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewport_state.viewportCount = 1;
		viewport_state.pViewports = nullptr;
		viewport_state.scissorCount = 1;
		viewport_state.pScissors = nullptr;

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

		VkPipelineLayoutCreateInfo pipeline_layout_info = {};
		pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipeline_layout_info.setLayoutCount = 1;
		pipeline_layout_info.pSetLayouts = &set_layout;
		pipeline_layout_info.pushConstantRangeCount = 0;

		if (!errorState.check(vkCreatePipelineLayout(device, &pipeline_layout_info, nullptr, &pipelineLayout) == VK_SUCCESS, "Failed to create pipeline layout"))
			return false;

		VkPipelineDepthStencilStateCreateInfo depth_stencil = getDepthStencilCreateInfo(materialInstance);

		VkGraphicsPipelineCreateInfo pipeline_info = {};
		pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipeline_info.stageCount = 2;
		pipeline_info.pStages = shader_stages;
		pipeline_info.pVertexInputState = &vertex_input_info;
		pipeline_info.pInputAssemblyState = &input_assembly;
		pipeline_info.pViewportState = &viewport_state;
		pipeline_info.pRasterizationState = &rasterizer;
		pipeline_info.pMultisampleState = &multisampling;
		pipeline_info.pColorBlendState = &colorBlending;
		pipeline_info.pDepthStencilState = &depth_stencil;
		pipeline_info.layout = pipelineLayout;
		pipeline_info.renderPass = renderPass;
		pipeline_info.pDynamicState = &dynamic_state_create_info;
		pipeline_info.subpass = 0;
		pipeline_info.basePipelineHandle = VK_NULL_HANDLE;

		if (!errorState.check(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &graphicsPipeline) == VK_SUCCESS, "Failed to create graphics pipeline"))
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


	RenderService::Pipeline RenderService::getOrCreatePipeline(const IRenderTarget& renderTarget, const IMesh& mesh, const MaterialInstance& materialInstance, utility::ErrorState& errorState)
	{
		const Material& material = materialInstance.getMaterial();
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


	RenderService::Pipeline RenderService::getOrCreatePipeline(const IRenderTarget& renderTarget, const RenderableMesh& mesh, utility::ErrorState& errorState)
	{
		return getOrCreatePipeline(renderTarget, mesh.getMesh(), mesh.getMaterialInstance(), errorState);
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
			if (!errorState.check(material_binding != nullptr, "Unable to find binding %s for shader %s in material %s", kvp.first.c_str(), material.getShader().getDisplayName().c_str(), material.mID.c_str()))
				return RenderableMesh();

			const VertexBuffer* vertex_buffer = mesh.getMeshInstance().getGPUMesh().findVertexBuffer(material_binding->mMeshAttributeID);
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
		assert(mCurrentCommandBuffer != VK_NULL_HANDLE);	// BeginRendering is not called if this assert is fired	

		// Sort objects to render
		std::vector<RenderableComponentInstance*> components_to_render = comps;
		sortFunction(components_to_render, camera);

		// Before we render, we always set aspect ratio. This avoids overly complex
		// responding to various changes in render target sizes.
		camera.setRenderTargetSize(renderTarget.getBufferSize());

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
		return mEmptyTexture->init(settings, false, Texture2D::EClearMode::Clear, 0, errorState);
	}


	// Set the currently active renderer
	bool RenderService::init(nap::utility::ErrorState& errorState)
	{
		// Get handle to scene service
		mSceneService = getCore().getService<SceneService>();
		assert(mSceneService != nullptr);

		// Initialize SDL video
		mSDLInitialized = SDL::initVideo(errorState);
		if (!errorState.check(mSDLInitialized, "Failed to init SDL Video"))
			return false;

		// Initialize shader compiler
		mShInitialized = ShInitialize() != 0;
		if (!errorState.check(mShInitialized, "Failed to initialize shader compiler"))
			return false;

		// Store render settings, used for initialization and global window creation
		nap::RenderServiceConfiguration* render_config = getConfiguration<RenderServiceConfiguration>();
		mEnableHighDPIMode = render_config->mEnableHighDPIMode;

		// Store if we are running headless, there is no display device (monitor) attached to the GPU.
		mHeadless = render_config->mHeadless;

		// Temporary window used to bind an SDL_Window and Vulkan surface together. 
		// Allows for easy destruction of previously created and assigned resources when initialization fails.
		DummyWindow dummy_window;
		
		// Get available vulkan instance extensions using SDL.
		// Returns, next to the default VK_KHR_surface, a platform specific extension.
		// These extensions have to be enabled in order to create a swapchain and a handle to a presentable surface.
		// When running headless we don't present so don't need the extensions.
		std::vector<std::string> instance_extensions;
		if (!mHeadless)
		{
			// Create dummy window and verify creation
			dummy_window.mWindow = SDL_CreateWindow("Dummy", 0, 0, 32, 32, SDL_WINDOW_VULKAN | SDL_WINDOW_HIDDEN);
			if (!errorState.check(dummy_window.mWindow != nullptr, "Unable to create SDL window"))
				return false;
			
			// Get all available vulkan instance extensions, required to create a presentable surface.
			// It also provides a way to determine whether a queue family in a physical device supports presenting to particular surface.
			if (!getSurfaceInstanceExtensions(dummy_window.mWindow, instance_extensions, errorState))
				return false;
		}

		// Add debug display extension, we need this to relay debug messages
		instance_extensions.emplace_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

		// Get available vulkan layer extensions, notify when not all could be found
		std::vector<std::string> found_layers;
#ifndef NDEBUG
		// Get all available vulkan layers
		const std::vector<std::string>& requested_layers = render_config->mLayers;
		bool print_layers = render_config->mPrintAvailableLayers;
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
		mAPIVersion = VK_MAKE_VERSION(render_config->mVulkanVersionMajor, render_config->mVulkanVersionMinor, 0);
		if (!createVulkanInstance(found_layers, instance_extensions, mAPIVersion, mInstance, errorState))
			return false;

		// Vulkan messaging callback
		setupDebugCallback(mInstance, mDebugCallback, errorState);

		// Create presentation surface if not running headless. Can only do this after creation of instance.
		// Used to select a queue family that next to Graphics and Transfer commands supports presentation.
		if (!mHeadless)
		{
			dummy_window.mInstance = mInstance;
			if (!createSurface(dummy_window.mWindow, mInstance, dummy_window.mSurface, errorState))
				return false;
		}

		// Get the preferred physical device to select
		VkPhysicalDeviceType pref_gpu = getPhysicalDeviceType(render_config->mPreferredGPU);
		if (!selectPhysicalDevice(mInstance, pref_gpu, mAPIVersion, dummy_window.mSurface, mPhysicalDevice, errorState))
			return false;

		// Sample physical device features and notify
		mMaxRasterizationSamples = getMaxSampleCount(mPhysicalDevice.getHandle());
		nap::Logger::info("Max number of rasterization samples: %d", (int)(mMaxRasterizationSamples));
		mSampleShadingSupported = mPhysicalDevice.getFeatures().sampleRateShading > 0;
		nap::Logger::info("Sample rate shading: %s", mSampleShadingSupported ? "Supported" : "Not Supported");
		mAnisotropicFilteringSupported = mPhysicalDevice.getFeatures().samplerAnisotropy > 0;
		nap::Logger::info("Anisotropic filtering: %s", mAnisotropicFilteringSupported ? "Supported" : "Not Supported");
		mAnisotropicSamples = mAnisotropicFilteringSupported ? render_config->mAnisotropicFilterSamples : 1;
		nap::Logger::info("Max anisotropic filter samples: %d", mAnisotropicSamples);
		mWideLinesSupported = mPhysicalDevice.getFeatures().wideLines > 0;
		nap::Logger::info("Wide lines: %s", mWideLinesSupported ? "Supported" : "Not Supported");
		mLargePointsSupported = mPhysicalDevice.getFeatures().largePoints > 0;
		nap::Logger::info("Large points: %s", mLargePointsSupported ? "Supported" : "Not Supported");

		// Get extensions that are required for NAP render engine to function.
		std::vector<std::string> required_ext_names = getRequiredDeviceExtensionNames();

		// Add swapchain when not running headless. Adds the ability to present rendered results to a surface.
		if (!mHeadless) { required_ext_names.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME); };

		// Add additional requests
		required_ext_names.insert(required_ext_names.end(), render_config->mAdditionalExtensions.begin(), render_config->mAdditionalExtensions.end());
		
		// Create unique set
		std::unordered_set<std::string> unique_ext_names(required_ext_names.size());
		for (const auto& ext : required_ext_names)
			unique_ext_names.emplace(ext);

		// Create a logical device that interfaces with the physical device.
		bool print_extensions = render_config->mPrintAvailableExtensions;
		if (!createLogicalDevice(mPhysicalDevice, found_layers, unique_ext_names, print_extensions, mDevice, errorState))
			return false;

		// Create command pool
		if (!errorState.check(createCommandPool(mPhysicalDevice.getHandle(), mDevice, mPhysicalDevice.getQueueIndex(), mCommandPool), "Failed to create Command Pool"))
			return false;

		// Determine depth format for the current device
		if (!errorState.check(findDepthFormat(mPhysicalDevice.getHandle(), mDepthFormat), "Unable to find depth format"))
			return false;

		// Get a compatible queue that will process commands, graphics / transfer needs to be supported
		vkGetDeviceQueue(mDevice, mPhysicalDevice.getQueueIndex(), 0, &mQueue);

		VmaAllocatorCreateInfo allocatorInfo = {};
		allocatorInfo.physicalDevice = mPhysicalDevice.getHandle();
		allocatorInfo.device = mDevice;
        allocatorInfo.vulkanApiVersion = mAPIVersion;
        allocatorInfo.instance = mInstance;

		if (!errorState.check(vmaCreateAllocator(&allocatorInfo, &mVulkanAllocator) == VK_SUCCESS, "Failed to create Vulkan Memory Allocator"))
			return false;

		// Create allocator for descriptor sets
		mDescriptorSetAllocator = std::make_unique<DescriptorSetAllocator>(mDevice);

		// Initialize an empty texture. This texture is used as the default for any samplers that don't have a texture bound to them in the data.
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


	Material* RenderService::getOrCreateMaterial(rtti::TypeInfo shaderType, utility::ErrorState& error)
	{
		// Ensure it's a shader
		std::string shader_name = shaderType.get_name().to_string();
		if (!error.check(shaderType.is_derived_from(RTTI_OF(nap::Shader)), "Requested type: %s is not a shader", shader_name.c_str()))
			return nullptr;

		// Find it in our map
		auto it = mMaterials.find(shaderType);
		if (it != mMaterials.end())
		{		
			// Add error message if material isn't valid, ie: not initalized
			error.check(it->second->valid(), "Shader not initialized");
			return it->second->mMaterial.get();
		}

		// Create video shader and material instance
		std::unique_ptr<Shader>shader(shaderType.create<Shader>({ getCore() }));
		shader->mID = utility::stringFormat("%s_%s", shader_name.c_str(), math::generateUUID().c_str());
		
		std::unique_ptr<Material> material = std::make_unique<Material>(getCore());
		material->mID = utility::stringFormat("Material_%s_%s", shader_name.c_str(), math::generateUUID().c_str());
		material->mShader = shader.get();

		// Initialization failed, invalid entry
		if (!shader->init(error) || !material->init(error))
		{
			// Add invalid combo
			mMaterials.emplace(std::make_pair(shaderType, std::make_unique<UniqueMaterial>()));
			return nullptr;
		}

		// Initialization succeeded, valid entry
		auto inserted = mMaterials.emplace(std::make_pair(shaderType, std::make_unique<UniqueMaterial>(std::move(shader), std::move(material))));
		return inserted.first->second->mMaterial.get();
	}


	void RenderService::getFormatProperties(VkFormat format, VkFormatProperties& outProperties)
	{
		vkGetPhysicalDeviceFormatProperties(mPhysicalDevice.getHandle(), format, &outProperties);
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
	    if(isInitialized())
		    waitDeviceIdle();
	}


	void RenderService::preResourcesLoaded()
	{
	    assert(isInitialized());
		waitDeviceIdle();
	}


	// Shut down renderer
	void RenderService::shutdown()
	{
		mMaterials.clear();
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

		if (mVulkanAllocator != VK_NULL_HANDLE)
		{
			vmaDestroyAllocator(mVulkanAllocator);
			mVulkanAllocator = VK_NULL_HANDLE;
		}

		if (mCommandPool != VK_NULL_HANDLE)
		{
			vkDestroyCommandPool(mDevice, mCommandPool, nullptr);
			mCommandPool = VK_NULL_HANDLE;
		}

		if (mDevice != VK_NULL_HANDLE)
		{
			vkDestroyDevice(mDevice, nullptr);
			mDevice = VK_NULL_HANDLE;
		}

		if (mDebugCallback != VK_NULL_HANDLE)
		{
			destroyDebugReportCallbackEXT(mInstance, mDebugCallback);
			mDebugCallback = VK_NULL_HANDLE;
		}

		if (mInstance != VK_NULL_HANDLE)
		{
			vkDestroyInstance(mInstance, nullptr);
			mInstance = VK_NULL_HANDLE;
		}

		if (mShInitialized)
		{
			ShFinalize();
			mShInitialized = false;
		}

		if (mSDLInitialized)
		{
			SDL::shutdownVideo();
			mSDLInitialized = false;
		}

		mInitialized = false;
	}
	
	void RenderService::transferData(VkCommandBuffer commandBuffer, const std::function<void()>& transferFunction)
	{
		vkResetCommandBuffer(commandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

		VkCommandBufferBeginInfo begin_info = {};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		// Begin recording commands
		VkResult result = vkBeginCommandBuffer(commandBuffer, &begin_info);
		assert(result == VK_SUCCESS);

		// Perform transfer
		transferFunction();

		// End recording commands
		result = vkEndCommandBuffer(commandBuffer);
		assert(result == VK_SUCCESS);

		// Submit command queue
		VkSubmitInfo submit_info = {};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &commandBuffer;
		result = vkQueueSubmit(mQueue, 1, &submit_info, VK_NULL_HANDLE);
		//assert(result == VK_SUCCESS);
	}


	void RenderService::uploadData()
	{
		// Fetch upload command buffer to use
		VkCommandBuffer commandBuffer = mFramesInFlight[mCurrentFrameIndex].mUploadCommandBuffer;

		// Transfer data to the GPU, including texture data and general purpose render buffers.
		transferData(commandBuffer, [commandBuffer, this]()
		{
			for (Texture2D* texture : mTexturesToClear)
				texture->clear(commandBuffer);
			mTexturesToClear.clear();

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
		// Push the download of a texture onto the command buffer
		Frame& frame = mFramesInFlight[mCurrentFrameIndex];
		VkCommandBuffer commandBuffer = frame.mDownloadCommandBuffers;
		transferData(commandBuffer, [commandBuffer, &frame]()
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

		// We wait for the fence for the current frame. This ensures that, when the wait completes, the command buffer
		// that the fence belongs to, and all resources referenced from it, are available for (re)use.
		// Notice that there are multiple other VkQueueSubmits that are performed by RenderWindow(s), and headless 
		// rendering. All those submits do not trigger a fence. They are all part of the same frame, so when the frame
		// fence has been signaled, we can be assured that all resources for the entire frame, including resources used 
		// by other VkQueueSubmits, are free to use.
		vkWaitForFences(mDevice, 1, &mFramesInFlight[mCurrentFrameIndex].mFence, VK_TRUE, UINT64_MAX);
		
		// We call updateTextureDownloads after we have waited for the fence. Otherwise it may happen that we check the fence
		// status which could still not be signaled at that point, causing the notify not to be called. If we then wait for
		// the fence anyway, we missed the opportunity to notify textures that downloads were ready. Because we reset the fence
		// next, we could delay the notification for a full frame cycle. So this call is purposely put inbetween the wait and reset
		// of the fence.
		updateTextureDownloads();

		// Release the DescriptorSets that were used for this frame index. This ensures that the DescriptorSets
		// can be re-allocated as part of this frame's rendering.
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
		assert(vkGetFenceStatus(mDevice, mFramesInFlight[mCurrentFrameIndex].mFence) == VK_SUCCESS);
		vkResetFences(mDevice, 1, &mFramesInFlight[mCurrentFrameIndex].mFence);

		// Push any texture downloads on the command buffer
		downloadData();

		// We perform a no-op submit that will ensure that a fence will be signaled when all of the commands for all of 
		// the command buffers that we submitted will be completed. This is how we can synchronize the CPU frame to the GPU.
		vkQueueSubmit(mQueue, 0, VK_NULL_HANDLE, mFramesInFlight[mCurrentFrameIndex].mFence);
		mCurrentFrameIndex = (mCurrentFrameIndex + 1) % 2;
		mIsRenderingFrame = false;
	}


	bool RenderService::beginHeadlessRecording()
	{
		assert(mCurrentCommandBuffer == VK_NULL_HANDLE);
		mCurrentCommandBuffer = mFramesInFlight[mCurrentFrameIndex].mHeadlessCommandBuffers;
		vkResetCommandBuffer(mCurrentCommandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

		VkCommandBufferBeginInfo begin_info = {};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		VkResult result = vkBeginCommandBuffer(mCurrentCommandBuffer, &begin_info);
		assert(result == VK_SUCCESS);

		return true;
	}


	void RenderService::endHeadlessRecording()
	{
		assert(mCurrentCommandBuffer != VK_NULL_HANDLE);
		VkResult result = vkEndCommandBuffer(mCurrentCommandBuffer);
		assert(result == VK_SUCCESS);

		VkSubmitInfo submit_info = {};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &mCurrentCommandBuffer;
		result = vkQueueSubmit(mQueue, 1, &submit_info, VK_NULL_HANDLE);
		assert(result == VK_SUCCESS);

		mCurrentCommandBuffer = VK_NULL_HANDLE;
	}


	bool RenderService::beginRecording(RenderWindow& renderWindow)
	{
		assert(mCurrentCommandBuffer == VK_NULL_HANDLE);
		assert(mCurrentRenderWindow  == VK_NULL_HANDLE);

		// Ask the window to begin recording commands.
		mCurrentCommandBuffer = renderWindow.beginRecording();
		if (mCurrentCommandBuffer == VK_NULL_HANDLE)
			return false;

		mCurrentRenderWindow = &renderWindow;
		return true;
	}


	void RenderService::endRecording()
	{
		assert(mCurrentCommandBuffer != VK_NULL_HANDLE);
		assert(mCurrentRenderWindow  != VK_NULL_HANDLE);
		
		// Stop recording, submit queue and ask for presentation
		mCurrentRenderWindow->endRecording();
		mCurrentCommandBuffer = VK_NULL_HANDLE;
		mCurrentRenderWindow  = VK_NULL_HANDLE;
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
		mTexturesToClear.erase(&texture);
		mTexturesToUpload.erase(&texture);

		for (Frame& frame : mFramesInFlight)
		{
			frame.mTextureDownloads.erase(std::remove_if(frame.mTextureDownloads.begin(), frame.mTextureDownloads.end(), [&texture](Texture2D* existingTexture)
			{
				return existingTexture == &texture;
			}), frame.mTextureDownloads.end());
		}
	}


	void RenderService::requestTextureClear(Texture2D& texture)
	{
		mTexturesToClear.insert(&texture);
	}


	void RenderService::requestTextureUpload(Texture2D& texture)
	{
		mTexturesToUpload.insert(&texture);
	}


	void RenderService::requestTextureDownload(Texture2D& texture)
	{
		// We push a texture download specifically for this frame. When the fence for that frame is signaled,
		// we know the download has been processed by the GPU, and we can send the texture a notification that
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


	bool RenderService::anisotropicFilteringSupported() const
	{
		return mAnisotropicFilteringSupported;
	}


	VkImageAspectFlags RenderService::getDepthAspectFlags() const
	{
		return mDepthFormat != VK_FORMAT_D32_SFLOAT ? 
			VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT : 
			VK_IMAGE_ASPECT_DEPTH_BIT;
	}


	RenderService::UniqueMaterial::UniqueMaterial(std::unique_ptr<Shader> shader, std::unique_ptr<Material> material) :
		mShader(std::move(shader)), mMaterial(std::move(material))
	{ }


	bool RenderService::UniqueMaterial::valid() const
	{
		return mShader != nullptr && mMaterial != nullptr;
	}
}


nap::PhysicalDevice::PhysicalDevice(VkPhysicalDevice device, const VkPhysicalDeviceProperties& properties, int queueIndex) :
	mDevice(device), mProperties(properties), mQueueIndex(queueIndex)
{
	vkGetPhysicalDeviceFeatures(mDevice, &mFeatures);
}

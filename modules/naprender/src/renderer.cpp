#include "renderer.h"
#include "SDL_vulkan.h"
#include "vulkan/vulkan.h"
#include "vulkan/vulkan_core.h"
#include "nsdlgl.h"

// External Includes
#include <utility/errorstate.h>
#include "glslang/Public/ShaderLang.h"
#include "nap/logger.h"

RTTI_DEFINE_CLASS(nap::Renderer)

RTTI_BEGIN_CLASS(nap::RendererSettings)
	RTTI_PROPERTY("DoubleBuffer",			&nap::RendererSettings::mDoubleBuffer,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("EnableMultiSampling",	&nap::RendererSettings::mEnableMultiSampling,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("MultiSamples",			&nap::RendererSettings::mMultiSamples,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("HighDPIMode",			&nap::RendererSettings::mEnableHighDPIMode,		nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
	/**
	*	@return the set of layers to be initialized with Vulkan
	*/
	const std::set<std::string>& getRequestedLayerNames()
	{
		static std::set<std::string> layers;
		if (layers.empty())
		{
			layers.emplace("VK_LAYER_NV_optimus");
			layers.emplace("VK_LAYER_LUNARG_standard_validation");
		}
		return layers;
	}


	/**
	* @return the set of required device extension names
	*/
	const std::set<std::string>& getRequestedDeviceExtensionNames()
	{
		static std::set<std::string> layers;
		if (layers.empty())
		{
			layers.emplace(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		}
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
		Logger::info("Validation Layer [%s]: %s", layerPrefix, msg);
		return VK_FALSE;
	}

	VkResult createDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback)
	{
		auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
		if (func != nullptr)
		{
			return func(instance, pCreateInfo, pAllocator, pCallback);
		}
		else
		{
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	/**
	*	Sets up the vulkan messaging callback specified above
	*/
	bool setupDebugCallback(VkInstance instance, VkDebugReportCallbackEXT& callback, utility::ErrorState& errorState)
	{
		VkDebugReportCallbackCreateInfoEXT createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
		createInfo.pfnCallback = debugCallback;

		if (!errorState.check(createDebugReportCallbackEXT(instance, &createInfo, nullptr, &callback) == VK_SUCCESS, "Unable to create debug report callback extension"))
			return false;

		return true;
	}


	bool getAvailableVulkanLayers(std::vector<std::string>& outLayers, utility::ErrorState& errorState)
	{
		// Figure out the amount of available layers
		// Layers are used for debugging / validation etc / profiling..
		unsigned int instance_layer_count = 0;
		if (!errorState.check(vkEnumerateInstanceLayerProperties(&instance_layer_count, NULL) == VK_SUCCESS, "Unable to query vulkan instance layer property count"))
			return false;

		std::vector<VkLayerProperties> instance_layers(instance_layer_count);
		if (!errorState.check(vkEnumerateInstanceLayerProperties(&instance_layer_count, instance_layers.data()) == VK_SUCCESS, "Unable to retrieve vulkan instance layer names"))
			return false;

		Logger::info("Found %d instance layers:", instance_layer_count);
		
		const std::set<std::string>& requested_layers = getRequestedLayerNames();
		outLayers.clear();
		for (int index = 0; index < instance_layers.size(); ++index)
		{
			VkLayerProperties& layer = instance_layers[index];
			Logger::info("%d: %s: %s", index, layer.layerName, layer.description);
			
			if (requested_layers.find(std::string(layer.layerName)) != requested_layers.end())
				outLayers.emplace_back(layer.layerName);
		}

		// Print the ones we're enabling
		Logger::info("");
		for (const auto& layer : outLayers)
			Logger::info("Applying layer: %s", layer.c_str());
		
		return true;
	}


	bool getAvailableVulkanExtensions(SDL_Window* window, std::vector<std::string>& outExtensions, utility::ErrorState& errorState)
	{
		// Figure out the amount of extensions vulkan needs to interface with the os windowing system 
		// This is necessary because vulkan is a platform agnostic API and needs to know how to interface with the windowing system
		unsigned int ext_count = 0;
		if (!errorState.check(SDL_Vulkan_GetInstanceExtensions(window, &ext_count, nullptr), "Unable to query the number of Vulkan instance extensions"))
			return false;

		// Use the amount of extensions queried before to retrieve the names of the extensions
		std::vector<const char*> ext_names(ext_count);
		if (!errorState.check(SDL_Vulkan_GetInstanceExtensions(window, &ext_count, ext_names.data()), "Unable to query the number of Vulkan instance extension names"))
			return false;

		// Display names
		Logger::info("Found %d Vulkan instance extensions:", ext_count);
		for (unsigned int i = 0; i < ext_count; i++)
		{
			Logger::info("%d: %s", i, ext_names[i]);
			outExtensions.emplace_back(ext_names[i]);
		}

		// Add debug display extension, we need this to relay debug messages
		outExtensions.emplace_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
		return true;
	}


	/**
	* Creates a vulkan instance using all the available instance extensions and layers
	* @return if the instance was created successfully
	*/
	bool createVulkanInstance(const std::vector<std::string>& layerNames, const std::vector<std::string>& extensionNames, VkInstance& outInstance, utility::ErrorState& errorState)
	{
		// Copy layers
		std::vector<const char*> layer_names;
		for (const auto& layer : layerNames)
			layer_names.emplace_back(layer.c_str());

		// Copy extensions
		std::vector<const char*> ext_names;
		for (const auto& ext : extensionNames)
			ext_names.emplace_back(ext.c_str());

		// Get the suppoerted vulkan instance version
		unsigned int api_version;
		vkEnumerateInstanceVersion(&api_version);

		// initialize the VkApplicationInfo structure
		VkApplicationInfo app_info = {};
		app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		app_info.pNext = NULL;
		app_info.pApplicationName = "NAP";
		app_info.applicationVersion = 1;
		app_info.pEngineName = "NAP";
		app_info.engineVersion = 1;
		app_info.apiVersion = api_version;	// Note: this is the *requested* version, which is the version the installed vulkan driver supports. If the device itself does not support this version, a lower version is returned. See selectGPU

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
		Logger::info("Initializing Vulkan instance");
		VkResult res = vkCreateInstance(&inst_info, NULL, &outInstance);
		switch (res)
		{
		case VK_SUCCESS:
			break;
		case VK_ERROR_INCOMPATIBLE_DRIVER:
			errorState.fail("Unable to create vulkan instance, cannot find a compatible Vulkan driver");
			return false;
		default:
			errorState.fail("Unable to create Vulkan instance: unknown error");
			return false;
		}

		return true;
	}


	/**
	* Allows the user to select a GPU (physical device)
	* @return if query, selection and assignment was successful
	* @param outDevice the selected physical device (gpu)
	* @param outQueueFamilyIndex queue command family that can handle graphics commands
	*/
	bool selectGPU(VkInstance instance, VkPhysicalDevice& outDevice, uint32_t& outDeviceVersion, unsigned int& outQueueFamilyIndex, utility::ErrorState& errorState)
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
		Logger::info("Found %d GPUs:", physical_device_count);
		
		std::vector<VkPhysicalDeviceProperties> physical_device_properties(physical_devices.size());
		for (int index = 0; index < physical_devices.size(); ++index)
		{
			VkPhysicalDevice physical_device = physical_devices[index];
			VkPhysicalDeviceProperties& properties = physical_device_properties[index];
			vkGetPhysicalDeviceProperties(physical_device, &properties);
			
			Logger::info("%d: %s (%d.%d)", index, properties.deviceName, VK_VERSION_MAJOR(properties.apiVersion), VK_VERSION_MINOR(properties.apiVersion));
		}

		VkPhysicalDevice selected_device = physical_devices[0];

		// Find the number queues this device supports, we want to make sure that we have a queue that supports graphics commands
		unsigned int family_queue_count(0);
		vkGetPhysicalDeviceQueueFamilyProperties(selected_device, &family_queue_count, nullptr);
		if (!errorState.check(family_queue_count != 0, "Device has no family of queues associated with it"))
			return false;

		// Extract the properties of all the queue families
		std::vector<VkQueueFamilyProperties> queue_properties(family_queue_count);
		vkGetPhysicalDeviceQueueFamilyProperties(selected_device, &family_queue_count, queue_properties.data());

		// Make sure the family of commands contains an option to issue graphical commands.
		unsigned int queue_node_index = -1;
		for (unsigned int i = 0; i < family_queue_count; i++)
		{
			if (queue_properties[i].queueCount > 0 && queue_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				queue_node_index = i;
				break;
			}
		}

		if (!errorState.check(queue_node_index >= 0, "Unable to find graphics command queue on device"))
			return false;

		// Set the output variables
		outDevice = selected_device;
		outDeviceVersion = physical_device_properties[0].apiVersion;	// The actual version of the device, which may be different from what we requested in the ApplicationInfo
		outQueueFamilyIndex = queue_node_index;
		return true;
	}


	/**
	*	Creates a logical device
	*/
	bool createLogicalDevice(VkPhysicalDevice& physicalDevice, unsigned int queueFamilyIndex, const std::vector<std::string>& layerNames, VkDevice& outDevice, utility::ErrorState& errorState)
	{
		// Copy layer names
		std::vector<const char*> layer_names;
		for (const auto& layer : layerNames)
			layer_names.emplace_back(layer.c_str());


		// Get the number of available extensions for our graphics card
		uint32_t device_property_count(0);
		if (!errorState.check(vkEnumerateDeviceExtensionProperties(physicalDevice, NULL, &device_property_count, NULL) == VK_SUCCESS, "Unable to acquire device extension property count"))
			return false;

		Logger::info("Found %d device extensions", device_property_count);

		// Acquire their actual names
		std::vector<VkExtensionProperties> device_properties(device_property_count);
		if (!errorState.check(vkEnumerateDeviceExtensionProperties(physicalDevice, NULL, &device_property_count, device_properties.data()) == VK_SUCCESS, "Unable to acquire device extension property names"))
			return false;

		// Match names against requested extension
		std::vector<const char*> device_property_names;
		const std::set<std::string>& required_extension_names = getRequestedDeviceExtensionNames();
		for (int index = 0; index < device_properties.size(); ++index)
		{
			const VkExtensionProperties& ext_property = device_properties[index];

			Logger::info("%d: %s", index, ext_property.extensionName);			

			auto it = required_extension_names.find(std::string(ext_property.extensionName));
			if (it != required_extension_names.end())
				device_property_names.emplace_back(ext_property.extensionName);
			
		}

		if (!errorState.check(required_extension_names.size() == device_property_names.size(), "Unable to find all required extensions"))
			return false;

		for (const auto& name : device_property_names)
			Logger::info("Applying device extension %s", name);

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
		create_info.pEnabledFeatures = NULL;
		create_info.flags = NULL;

		// Finally we're ready to create a new device
		if (!errorState.check(vkCreateDevice(physicalDevice, &create_info, nullptr, &outDevice) == VK_SUCCESS, "Failed to create logical device"))
			return false;

		return true;
	}

	bool createCommandPool(VkPhysicalDevice physicalDevice, VkDevice device, unsigned int graphicsQueueIndex, VkCommandPool& commandPool) 
	{
		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = graphicsQueueIndex;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;		

		return vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) == VK_SUCCESS;
	}

	static bool findDepthFormat(VkPhysicalDevice physicalDevice, VkFormat& outFormat)
	{
		std::vector<VkFormat> candidates = { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };

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


	//////////////////////////////////////////////////////////////////////////

	bool Renderer::init(const RendererSettings& rendererSettings, utility::ErrorState& errorState)
	{
		if (!errorState.check(opengl::initVideo(), "Failed to init SDL"))
			return false;

		if (!errorState.check(ShInitialize() != 0, "Failed to initialize shader compiler"))
			return false;

		// Store render settings, used for initialization and global window creation
		mSettings = rendererSettings;

		// Get available vulkan extensions, necessary for interfacing with native window
		// SDL takes care of this call and returns, next to the default VK_KHR_surface a platform specific extension
		// When initializing the vulkan instance these extensions have to be enabled in order to create a valid
		// surface later on.
		std::vector<std::string> found_extensions;
		SDL_Window* dummy_window = SDL_CreateWindow("Dummy", 0, 0, 32, 32, SDL_WINDOW_VULKAN | SDL_WINDOW_HIDDEN);
		bool got_extensions = getAvailableVulkanExtensions(dummy_window, found_extensions, errorState);
		SDL_DestroyWindow(dummy_window);

		if (!got_extensions)
			return false;

		// Get available vulkan layer extensions, notify when not all could be found
		std::vector<std::string> found_layers;
		if (!getAvailableVulkanLayers(found_layers, errorState))
			return false;

		// Warn when not all requested layers could be found
		if (!errorState.check(found_layers.size() == getRequestedLayerNames().size(), "Not all requested layers were found"))
			return false;

		// Create Vulkan Instance
		if (!createVulkanInstance(found_layers, found_extensions, mInstance, errorState))
			return false;

		// Vulkan messaging callback
		setupDebugCallback(mInstance, mDebugCallback, errorState);

		// Select GPU after succsessful creation of a vulkan instance
		if (!selectGPU(mInstance, mPhysicalDevice, mPhysicalDeviceVersion, mGraphicsQueueIndex, errorState))
			return false;

		// Create a logical device that interfaces with the physical device
		if (!createLogicalDevice(mPhysicalDevice, mGraphicsQueueIndex, found_layers, mDevice, errorState))
			return false;

		if (!errorState.check(createCommandPool(mPhysicalDevice, mDevice, mGraphicsQueueIndex, mCommandPool), "Failed to create commandpool"))
			return false;

		if (!errorState.check(findDepthFormat(mPhysicalDevice, mDepthFormat), "Unable to find depth format"))
			return false;

		vkGetDeviceQueue(mDevice, mGraphicsQueueIndex, 0, &mGraphicsQueue);

		return true;
	}


	// Create an opengl window
	std::shared_ptr<GLWindow> Renderer::createRenderWindow(const RenderWindowSettings& settings, const std::string& inID, utility::ErrorState& errorState)
	{
		// Copy settings and set dpi mode
		RenderWindowSettings window_settings = settings;
		window_settings.highdpi = mSettings.mEnableHighDPIMode;

#if 0
		// The primary window always exists. This is necessary to initialize openGL, and we need a window and an associated GL context for creating
		// resources before a window resource becomes available. The first RenderWindow that is created will share the primary window with the Renderer.
		// The settings that are passed here are applied to the primary window.
		// Because of the nature of the real-time editing system, when Windows are edited, new Windows will be created before old Windows are destroyed.
		// We need to make sure that an edit to a RenderWindow that was previously created as primary window will not create a new window, but share the 
		// existing primary window. We do this by testing if the IDs are the same.
		if (mPrimaryWindow.unique() || mPrimaryWindowID == inID)
		{
			mPrimaryWindowID = inID;
			mPrimaryWindow->applySettings(window_settings);
			return mPrimaryWindow;
		}
#endif

		// Construct and return new window
		std::shared_ptr<GLWindow> new_window = std::make_shared<GLWindow>();
		if (!new_window->init(window_settings, *this, errorState))
			return nullptr;

		return new_window;
	}


	// Closes all opengl systems
	void Renderer::shutdown()
	{
		opengl::shutdown();
	}
}

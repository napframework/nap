#include "renderer.h"
#include "SDL_vulkan.h"
#include "vulkan/vulkan.h"
#include "vulkan/vulkan_core.h"

// External Includes
#include <nopengl.h>
#include <utility/errorstate.h>

#include "glslang/Public/ShaderLang.h"

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
		std::cout << "validation layer: " << layerPrefix << ": " << msg << std::endl;
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
	bool setupDebugCallback(VkInstance instance, VkDebugReportCallbackEXT& callback)
	{
		VkDebugReportCallbackCreateInfoEXT createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
		createInfo.pfnCallback = debugCallback;

		if (createDebugReportCallbackEXT(instance, &createInfo, nullptr, &callback) != VK_SUCCESS)
		{
			std::cout << "unable to create debug report callback extension\n";
			return false;
		}
		return true;
	}


	bool getAvailableVulkanLayers(std::vector<std::string>& outLayers)
	{
		// Figure out the amount of available layers
		// Layers are used for debugging / validation etc / profiling..
		unsigned int instance_layer_count = 0;
		VkResult res = vkEnumerateInstanceLayerProperties(&instance_layer_count, NULL);
		if (res != VK_SUCCESS)
		{
			std::cout << "unable to query vulkan instance layer property count\n";
			return false;
		}

		std::vector<VkLayerProperties> instance_layer_names(instance_layer_count);
		res = vkEnumerateInstanceLayerProperties(&instance_layer_count, instance_layer_names.data());
		if (res != VK_SUCCESS)
		{
			std::cout << "unable to retrieve vulkan instance layer names\n";
			return false;
		}

		// Display layer names and find the ones we specified above
		std::cout << "found " << instance_layer_count << " instance layers:\n";
		std::vector<const char*> valid_instance_layer_names;
		const std::set<std::string>& lookup_layers = getRequestedLayerNames();
		int count(0);
		outLayers.clear();
		for (const auto& name : instance_layer_names)
		{
			std::cout << count << ": " << name.layerName << ": " << name.description << "\n";
			auto it = lookup_layers.find(std::string(name.layerName));
			if (it != lookup_layers.end())
				outLayers.emplace_back(name.layerName);
			count++;
		}

		// Print the ones we're enabling
		std::cout << "\n";
		for (const auto& layer : outLayers)
			std::cout << "applying layer: " << layer.c_str() << "\n";
		return true;
	}


	bool getAvailableVulkanExtensions(SDL_Window* window, std::vector<std::string>& outExtensions)
	{
		// Figure out the amount of extensions vulkan needs to interface with the os windowing system 
		// This is necessary because vulkan is a platform agnostic API and needs to know how to interface with the windowing system
		unsigned int ext_count = 0;
		if (!SDL_Vulkan_GetInstanceExtensions(window, &ext_count, nullptr))
		{
			std::cout << "Unable to query the number of Vulkan instance extensions\n";
			return false;
		}

		// Use the amount of extensions queried before to retrieve the names of the extensions
		std::vector<const char*> ext_names(ext_count);
		if (!SDL_Vulkan_GetInstanceExtensions(window, &ext_count, ext_names.data()))
		{
			std::cout << "Unable to query the number of Vulkan instance extension names\n";
			return false;
		}

		// Display names
		std::cout << "found " << ext_count << " Vulkan instance extensions:\n";
		for (unsigned int i = 0; i < ext_count; i++)
		{
			std::cout << i << ": " << ext_names[i] << "\n";
			outExtensions.emplace_back(ext_names[i]);
		}

		// Add debug display extension, we need this to relay debug messages
		outExtensions.emplace_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
		std::cout << "\n";
		return true;
	}


	/**
	* Creates a vulkan instance using all the available instance extensions and layers
	* @return if the instance was created successfully
	*/
	bool createVulkanInstance(const std::vector<std::string>& layerNames, const std::vector<std::string>& extensionNames, VkInstance& outInstance)
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
		app_info.apiVersion = VK_API_VERSION_1_0;

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
		std::cout << "initializing Vulkan instance\n\n";
		VkResult res = vkCreateInstance(&inst_info, NULL, &outInstance);
		switch (res)
		{
		case VK_SUCCESS:
			break;
		case VK_ERROR_INCOMPATIBLE_DRIVER:
			std::cout << "unable to create vulkan instance, cannot find a compatible Vulkan ICD\n";
			return false;
		default:
			std::cout << "unable to create Vulkan instance: unknown error\n";
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
	bool selectGPU(VkInstance instance, VkPhysicalDevice& outDevice, unsigned int& outQueueFamilyIndex)
	{
		// Get number of available physical devices, needs to be at least 1
		unsigned int physical_device_count(0);
		vkEnumeratePhysicalDevices(instance, &physical_device_count, nullptr);
		if (physical_device_count == 0)
		{
			std::cout << "No physical devices found\n";
			return false;
		}

		// Now get the devices
		std::vector<VkPhysicalDevice> physical_devices(physical_device_count);
		vkEnumeratePhysicalDevices(instance, &physical_device_count, physical_devices.data());

		// Show device information
		std::cout << "found " << physical_device_count << " GPU(s):\n";
		int count(0);
		std::vector<VkPhysicalDeviceProperties> physical_device_properties(physical_devices.size());
		for (auto& physical_device : physical_devices)
		{
			vkGetPhysicalDeviceProperties(physical_device, &(physical_device_properties[count]));
			std::cout << count << ": " << physical_device_properties[count].deviceName << "\n";
			count++;
		}

		// Select one if more than 1 is available
		unsigned int selection_id = 0;
		if (physical_device_count > 1)
		{
			while (true)
			{
				std::cout << "select device: ";
				std::cin >> selection_id;
				if (selection_id >= physical_device_count || selection_id < 0)
				{
					std::cout << "invalid selection, expected a value between 0 and " << physical_device_count - 1 << "\n";
					continue;
				}
				break;
			}
		}
		std::cout << "selected: " << physical_device_properties[selection_id].deviceName << "\n";
		VkPhysicalDevice selected_device = physical_devices[selection_id];

		// Find the number queues this device supports, we want to make sure that we have a queue that supports graphics commands
		unsigned int family_queue_count(0);
		vkGetPhysicalDeviceQueueFamilyProperties(selected_device, &family_queue_count, nullptr);
		if (family_queue_count == 0)
		{
			std::cout << "device has no family of queues associated with it\n";
			return false;
		}

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

		if (queue_node_index < 0)
		{
			std::cout << "Unable to find a queue command family that accepts graphics commands\n";
			return false;
		}

		// Set the output variables
		outDevice = selected_device;
		outQueueFamilyIndex = queue_node_index;
		return true;
	}


	/**
	*	Creates a logical device
	*/
	bool createLogicalDevice(VkPhysicalDevice& physicalDevice,
		unsigned int queueFamilyIndex,
		const std::vector<std::string>& layerNames,
		VkDevice& outDevice)
	{
		// Copy layer names
		std::vector<const char*> layer_names;
		for (const auto& layer : layerNames)
			layer_names.emplace_back(layer.c_str());


		// Get the number of available extensions for our graphics card
		uint32_t device_property_count(0);
		if (vkEnumerateDeviceExtensionProperties(physicalDevice, NULL, &device_property_count, NULL) != VK_SUCCESS)
		{
			std::cout << "Unable to acquire device extension property count\n";
			return false;
		}
		std::cout << "\nfound " << device_property_count << " device extensions\n";

		// Acquire their actual names
		std::vector<VkExtensionProperties> device_properties(device_property_count);
		if (vkEnumerateDeviceExtensionProperties(physicalDevice, NULL, &device_property_count, device_properties.data()) != VK_SUCCESS)
		{
			std::cout << "Unable to acquire device extension property names\n";
			return false;
		}

		// Match names against requested extension
		std::vector<const char*> device_property_names;
		const std::set<std::string>& required_extension_names = getRequestedDeviceExtensionNames();
		int count = 0;
		for (const auto& ext_property : device_properties)
		{
			std::cout << count << ": " << ext_property.extensionName << "\n";
			auto it = required_extension_names.find(std::string(ext_property.extensionName));
			if (it != required_extension_names.end())
			{
				device_property_names.emplace_back(ext_property.extensionName);
			}
			count++;
		}

		// Warn if not all required extensions were found
		if (required_extension_names.size() != device_property_names.size())
		{
			std::cout << "not all required device extensions are supported!\n";
			return false;
		}

		std::cout << "\n";
		for (const auto& name : device_property_names)
			std::cout << "applying device extension: " << name << "\n";

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
		VkResult res = vkCreateDevice(physicalDevice, &create_info, nullptr, &outDevice);
		if (res != VK_SUCCESS)
		{
			std::cout << "failed to create logical device!\n";
			return false;
		}
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
		bool got_extensions = getAvailableVulkanExtensions(dummy_window, found_extensions);
		SDL_DestroyWindow(dummy_window);

		if (!got_extensions)
			return false;

		// Get available vulkan layer extensions, notify when not all could be found
		std::vector<std::string> found_layers;
		if (!getAvailableVulkanLayers(found_layers))
			return false;

		// Warn when not all requested layers could be found
		if (found_layers.size() != getRequestedLayerNames().size())
			std::cout << "warning! not all requested layers could be found!\n";

		// Create Vulkan Instance
		if (!createVulkanInstance(found_layers, found_extensions, mInstance))
			return false;

		// Vulkan messaging callback
		setupDebugCallback(mInstance, mDebugCallback);

		// Select GPU after succsessful creation of a vulkan instance (jeeeej no global states anymore)
		if (!selectGPU(mInstance, mPhysicalDevice, mGraphicsQueueIndex))
			return false;

		// Create a logical device that interfaces with the physical device
		if (!createLogicalDevice(mPhysicalDevice, mGraphicsQueueIndex, found_layers, mDevice))
			return false;

		if (!createCommandPool(mPhysicalDevice, mDevice, mGraphicsQueueIndex, mCommandPool))
			return false;

		if (!findDepthFormat(mPhysicalDevice, mDepthFormat))
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

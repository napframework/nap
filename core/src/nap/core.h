/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "modulemanager.h"
#include "resourcemanager.h"
#include "service.h"
#include "coreextension.h"
#include "projectinfo.h"
#include "timer.h"

// External Includes
#include <rtti/factory.h>
#include <rtti/rtti.h>
#include <rtti/deserializeresult.h>
#include <unordered_set>
#include <utility/dllexport.h>
#include <unordered_map>
#include <vector>

// Default name to use when writing the file that contains all the settings for the NAP services.
inline constexpr char DEFAULT_SERVICE_CONFIG_FILENAME[] = "config.json";
inline constexpr char PROJECT_INFO_FILENAME[] = "project.json";

// Build configuration eg. "Clang-Debug-x86_64"
#define STRINGIZE(x) #x
#define STRINGIZE_VALUE_OF(x) STRINGIZE(x)
inline constexpr char sBuildConf[] = STRINGIZE_VALUE_OF(NAP_BUILD_CONF);
inline constexpr char sBuildType[] = STRINGIZE_VALUE_OF(NAP_BUILD_TYPE);
inline constexpr char sBuildArch[] = STRINGIZE_VALUE_OF(NAP_BUILD_ARCH);

namespace nap
{
	using ServiceConfigMap = std::unordered_map<rtti::TypeInfo, ServiceConfiguration*>;

	/**
	 * Core manages the object graph, modules and services
	 * Core is required in every NAP application and should be the first object that is created and
	 * initialized. There should only be only 1 instance of Core in your application
	 *
	 * After creation, initialize the core engine by invoking initializeEngine(). This will
	 * load all the available modules and their dependencies including services.
	 * When all modules are loaded all available services are initialized.
	 * Initialization occurs based on the Service dependency tree. So when Service B points to A,
	 * Service A is initialized before B. After initialization all module specific resources and their
	 * contexts are available for object creation using the ResourceManager.
	 *
	 * Every instance of Core is initialized against a nap::ProjectInfo resource, which is declared inside a `project.json` file. 
	 * The nap::ProjectInfo file contains information such as the project name, version and which modules are 
	 * required for the project to run. It also contains a link to a data file, which contains the actual application content and
	 * an optional link to a service configuration file, which contains service configuration information.
	 *
	 * The linked path mapping file provides core with additional information on how to resolve paths to all required modules.
	 * Using this information both the editor and application are able to load the requested modules and initialize all required services.
	 *
	 * It is therefore required that core is able to load a valid project.json file, that contains a 'nap::ProjectInfo' resource.
	 * example of a project.json file:
	 *
	 * ~~~~~
	 *	{
	 *		"Type": "nap::ProjectInfo",
	 *		"mID": "ProjectInfo",
	 *		"Title": "MyProject",
	 *		"Version": "1.0.0",
	 *		"RequiredModules": 
	 *		[
	 *			"mod_napapp",
	 *			"mod_napimgui"
	 *		],
	 *		"Data": "data/myapp.json",
	 *		"ServiceConfig": "",
	 *		"PathMapping": "cache/path_mapping.json"
	 *	}
	 * ~~~~~
	 * 
	 * Call update inside your app loop to update all available services. When exiting the application
	 * invoke shutdown. This will close all operating services in the reverse order of their dependency tree
	 */
	class NAPAPI Core final
	{
		RTTI_ENABLE()
	public:
		// Unique services handle.
		// Ensures services are destroyed after being initialized.
		class Services;
		using ServicesHandle = std::unique_ptr<Services>;

		/**
		 * Default Constructor
		 */
		Core();

		/**
		 * Extension constructor.
		 * Used to add additional information to Core.
		 * This is the case on specific platforms, where additional information is required to correctly initialize Core.
		 * @param coreExtension the extension to associated with this instance of core, owned by core after construction.
		 */
		Core(std::unique_ptr<CoreExtension> coreExtension);

		/**
		 * Destructor
		 */
		virtual ~Core();

		/**
		 * Loads all modules in to the core environment and creates all the associated services.
		 *
		 * Every instance of Core is initialized against a nap::ProjectInfo resource, which is declared inside a `project.json` file.
		 * The nap::ProjectInfo file contains information such as the project name, version and which modules are
		 * required for the project to run. It also contains an optional link to a service configuration file, 
		 * which holds service configuration information.
		 *
		 * The linked path mapping file provides core with additional information on how to resolve paths to all required modules.
		 * Using this information both the editor and application are able to load the requested modules and initialize all required services.
		 *
		 * It is therefore required that core is able to find and load a valid project.json file that contains a 'nap::ProjectInfo' resource.
		 *
		 * @param error contains the error code when initialization fails
		 * @return if initialization succeeded
		 */
		bool initializeEngine(utility::ErrorState& error);

		/**
		 * Loads all modules in to the core environment and creates all the associated services.
		 *
		 * Every instance of Core is initialized against a nap::ProjectInfo resource, which is declared inside a `project.json` file.
		 * The nap::ProjectInfo file contains information such as the project name, version and which modules are
		 * required for the project to run. It also contains an optional link to a service configuration file,
		 * which holds service configuration information.
		 *
		 * The linked path mapping file provides core with additional information on how to resolve paths to all required modules.
		 * Using this information both the editor and application are able to load the requested modules and initialize all required services.
		 *
		 * It is therefore required that core is able to find and load the provided project.json file that contains a 'nap::ProjectInfo' resource.
		 *
		 * @param projectInfofile absolute path to the project file on disk.
		 * @param context whether initializing for application or editor
		 * @param error contains the error code when initialization fails
		 * @return if initialization succeeded
		 */
		bool initializeEngine(const std::string& projectInfofile, ProjectInfo::EContext context, utility::ErrorState& error);

		/**
		 * Attempts to initialize all registered services. Call this after initializeEngine().
		 * Initialization occurs based on service dependencies, this means that if service B depends on Service A,
		 * Service A is initialized before service B etc.
		 * This call returns a handle that, when destroyed, shuts down all services in the right order.
		 * @param errorState contains the error message when initialization fails
		 * @return handle that manages services on success, nullptr if initialization fails
		 */
		Core::ServicesHandle initializeServices(utility::ErrorState& errorState);

		/**
		 * Returns if core is initialized.
		 * @return true if the engine initialized successfully, false otherwise. 
		 */
		bool isInitialized() const;

		/**
		 * Initialize python interpreter so we can have components running python scripts
		 */
		bool initializePython(utility::ErrorState& error);

		/**
		 * Starts core, call this after initializing the engine, just before starting the application loop.
		 */
		void start();

		/**
		 * Updates all services. This happens in 3 distinct steps.
		 * First the resource file is reloaded. After that all services are updated, the last step is the
		 * the update of the entities and their respective components managed by the resource manager
		 * @param updateFunction application callback that is invoked after updating all the services but before render.
		 * Input parameter is deltaTime
		 * @return deltaTime between update calls in seconds
		 */
		double update(std::function<void(double)>& updateFunction);

		/**
		* The resource manager holds all the entities and components currently loaded by Core.
		* @return the resource manager.
		*/
		ResourceManager* getResourceManager()							{ return mResourceManager.get(); }

		/**
		 * @return the ModuleManager for this core
		 */
		const ModuleManager& getModuleManager() const					{ return *mModuleManager; }

		/**
		 * @return number of elapsed time in milliseconds after invoking start
		 */
		uint32 getTicks() const;

		/**
		* @return number of elapsed seconds after invoking start
		*/
		double getElapsedTime() const;

		/**
		 * @return core start time
		 */
		SteadyTimeStamp getStartTime() const;

		/**
		 * @return number of frames per second
		 */
		float getFramerate() const										{ return mFramerate; }

		/**
		 * Find a service of a given type.
		 * @param type the type of service to get
		 * @return found service, nullptr if not found.
		 */
		Service* getService(const rtti::TypeInfo& type);

		/**
		 * Find a service of a given type.
		 * @param type the type of service to get
		 * @return found service, nullptr if not found.
		 */
		const Service* getService(const rtti::TypeInfo& type) const;

		/**
		 * Searches for a service based on given type name, names need to match exactly.
		 * @return an already registered service based on its type name, nullptr if not found
		 * @param type the type of the service as a string
		 */
		Service* getService(const std::string& type);

		/**
		 * Searches for a service of type T, returns a nullptr if not found.
		 * 
		 * ~~~~~{.cpp}
		 *	RenderService* renderer = getCore().getService<nap::RenderService>();
		 *	...
		 * ~~~~~
		 * @return a service of type T, returns nullptr if that service can't be found
		 */
		template <typename T>
		T* getService();

		/**
		 * Searches for a service of type T, returns a nullptr if not found.
		 * 
		 * ~~~~~{.cpp}
		 *	const RenderService* renderer = getCore().getService<nap::RenderService>();
		 *	...
		 * ~~~~~
		 * @return a service of type T, returns nullptr if that service can't be found
		 */
		template <typename T>
		const T* getService() const;

		/**
		 * Returns the extension associated with this instance of core as T. 
		 * Note that an extension is given explicitly to core on construction.
		 * When using the default constructor core has no interface associated with it!
		 * @return extension associated with core as type T
		 */
		template <typename T>
		const T& getExtension() const;

		/**
		 * @return if core has an extension of type T	
		 */
		template <typename T>
		bool hasExtension() const;

		/**
		 * Searches for a file next to the binary, and in case of non-packaged builds, searches through the project
		 * folders to find the file.
		 * @param filename File to search for.
		 * @param foundFilePath The full file path of where the file was found.
		 * @return true if the file was found, otherwise false.
		 */
		bool findProjectFilePath(const std::string& filename, std::string& foundFilePath) const;

		/**
		 * Returns loaded project information, only available after initialization, nullptr otherwise.
		 * @return The loaded project information, available after initialization, nullptr otherwise
		 */
		const nap::ProjectInfo* getProjectInfo() const;

        /**
         * Load path mapping file and replace any template vars with their respective values
         * @param projectInfo The current project info
         * @param err Contains the error if the path mapping operation failed
         * @return The path mapping that was loaded or nullptr if loading failed
         */
		bool loadPathMapping(nap::ProjectInfo& projectInfo, nap::utility::ErrorState& err);

        /**
          * Writes a 'config.json' file, that contains all currently loaded service configurations
          * @param path The path to the config file relative to the path of the project info json file
          * @param errorState Contains the error if the operation fails
          * @param linkToProjectInfo If set to true the new written file will be linked in the current project info, and the project info will be saved to the project.json file.
          * @return True on success
         */
        bool writeConfigFile(const std::string& path, utility::ErrorState& errorState, bool linkToProjectInfo = true);

		/**
		 * Returns all available and used service configurations.
		 * @return all available service configurations.
		 */
		std::vector<const ServiceConfiguration*> getServiceConfigs() const;

		/**
		 * Used on macOS to apply an environment variable for Vulkan.
		 */
		void setupPlatformSpecificEnvironment();

		/**
		 * Helper class. Automatically initializes all services on construction and
		 * shuts down all services on destruction. This object is returned as a handle
		 * by 'initializeServices()' and ensures services are always shut-down,
		 * in the right order, after initialization.
		 *
		 * ~~~~~{.cpp}
		 * // Initialize services & bail if handle is invalid.
		 * Core::ServicesHandle handle = mCore.initializeServices(error);
		 * if (handle == nullptr)
		 *		return false;
		 * ~~~~~
		 */
		class NAPAPI Services final
		{
		public:
			/**
			 * Shuts down all services
			 */
			~Services();

			/**
			 * @return if all services were initialized on construction
			 */
			bool initialized()	{ return mInitialized;  }

		private:
			friend class Core;
			/**
			 * Attempts to initialize all services.
			 * @param core core instance
			 * @param error contains the error if service initialization fails
			 */
			Services(Core& core, utility::ErrorState& error);
			nap::Core& mCore;
			bool mInitialized = false;
		};

	private:
		/**
		 * Locates the project info (project.json) file.
		 * @param foundFilePath The full file path of where the file was found.
		 * @return true if the file was found, otherwise false.
		 */
		bool findProjectInfoFile(std::string& foundFilePath) const;

		/**
		 * Helper function that creates all the services that are found in the various modules
		 * Note that a module does not need to define a service, only if it has been defined
		 * this call will try to create it.
		 * @param error contains the error if the services could not be added
		 * @return if the services are created successfully
		 */
		bool createServices(const nap::ProjectInfo& projectInfo, utility::ErrorState& errorState);

		/**
		* Adds a new service of type @type to @outServices
		* @param type the type of service to add
		* @param configuration The ServiceConfiguration that should be used to construct the service
		* @param outServices the list of services the service of @type will be added to
		* @param errorState in case of a duplicate, contains the error message if the service could not be added
		* @return if the service was added successfully
		*/
		bool addService(const rtti::TypeInfo& type, ServiceConfiguration* configuration, std::vector<Service*>& outServices, utility::ErrorState& errorState);

		/**
		 * Loads the service configurations from file.
		 * If the service configuration file does not exist a warning is issued and system defaults are used.
		 * Loading fails if the file can be deserialized but contains duplicate IDs or 
		 * objects not derived from nap::ServiceConfiguration.
		 * @param err contains the error if loading fails.
		 * @return if loading succeeded.
		 */
		bool loadServiceConfigurations(nap::utility::ErrorState& err);

		/**
		 * Load the service configuration file.
		 * If the service configuration file does not exist a warning is issued and system defaults are used.
		 * Loading fails if the file contains duplicate IDs or objects not derived from nap::ServiceConfiguration.
		 * @param filename The name of the file to read
		 * @param deserialize_result contains the result after reading the config file
		 * @param errorState contains the error if deserialization fails
		 * @return if service configuration reading succeeded or not
		 */
		bool loadServiceConfiguration(const std::string& filename, rtti::DeserializeResult& deserialize_result, utility::ErrorState& errorState);

		/**
		 * Occurs when resources are about to be loaded by the resource manager
		 * Forwards the call to all interested services.
		 * This can only be called when the services have been initialized
		 */
		void preResourcesLoaded();

		/**
		 * Occurs when resources have been successfully loaded by the resource manager
		 * Forwards the call to all interested services.
		 * This can only be called when the services have been initialized
		 */
		void postResourcesLoaded();

		/**
		 *	Calculates the framerate over time
		 */
		void calculateFramerate(double deltaTime);

		/**
		 * Setup our Python environment to find Python in thirdparty for NAP release or NAP source,
		 * or alongside our binary for a packaged project
		 */
		void setupPythonEnvironment();

		/**
		 * Explicitly load a project from file.
		 * Call this before initializeEngine() if custom project setup is required.
		 * @param projectFilename absolute path to the project file on disk.
         * @param context whether initializing for project or Napkin
		 * @param error contains the error if the file could not be loaded.
		 */
	    bool loadProjectInfo(std::string projectFilename, ProjectInfo::EContext context, nap::utility::ErrorState& error);

		/**
		 * Finds the configuration of a specific service.
		 * @param serviceType service to find configuration for.
		 * @return service configuration of specific type if present, nullptr otherwise
		 */
		nap::ServiceConfiguration* findServiceConfig(rtti::TypeInfo serviceType) const;

		/**
		 * Add a new service configuration to this project if not present already.
		 * Ownership is transferred.
		 * @param serviceConfig the service configuration to add.
		 * @return true when added, false if already present.
		 */
		bool addServiceConfig(std::unique_ptr<nap::ServiceConfiguration> serviceConfig);

		// Manages all the loaded modules
		std::unique_ptr<ModuleManager> mModuleManager = nullptr;

		// Manages all the objects in core
		std::unique_ptr<ResourceManager> mResourceManager = nullptr;

		// Holds on to the current project's configuration
		std::unique_ptr<nap::ProjectInfo> mProjectInfo = nullptr;

		// Sorted service nodes, set after init
		std::vector<std::unique_ptr<Service>> mServices;

		// All service configurations
		std::unordered_map<rtti::TypeInfo, std::unique_ptr<ServiceConfiguration>> mServiceConfigs;

		// Interface associated with this instance of core.
		std::unique_ptr<CoreExtension> mExtension = nullptr;

		// Frame reference point
		SteadyTimeStamp mTimeStamp;

		// Core timer
		SteadyTimer mTimer;

		// Current framerate
		float mFramerate = 0.0f;

		// Used to calculate framerate over time
		std::array<double, 20> mTicks;
		double mTicksum = 0;
		uint32 mTickIdx = 0;

		// If the engine is initialized
		bool mInitialized = false;

		// Called before resources are loaded
		nap::Slot<> mPreResourcesLoadedSlot		= { [&]() -> void { preResourcesLoaded();  } };

		// Called after resources are loaded
		nap::Slot<> mPostResourcesLoadedSlot	= { [&]() -> void { postResourcesLoaded(); } };
	};
}

//////////////////////////////////////////////////////////////////////////
// Template definitions
//////////////////////////////////////////////////////////////////////////

/**
 * Searches for a service of type T in the services and returns it, returns nullptr if none found
 */
template <typename T>
T* nap::Core::getService()
{
	return static_cast<T*>(getService(RTTI_OF(T)));
}


/**
 * Searches for a service of type T in the services and returns it, returns nullptr if none found
 */
template <typename T>
const T* nap::Core::getService() const
{
	return static_cast<const T*>(getService(RTTI_OF(T)));
}


/**
 * Returns the core extension as an extension of type T
 */
template <typename T>
const T& nap::Core::getExtension() const
{
	T* core_ext = rtti_cast<T>(mExtension.get());
	assert(core_ext != nullptr);
	return *core_ext;
}


/**
 * Returns if core has an extension of type T
 */
template <typename T>
bool nap::Core::hasExtension() const
{
	return rtti_cast<T>(mExtension.get()) != nullptr;
}

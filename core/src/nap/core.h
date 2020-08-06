#pragma once

// std includes
#include <unordered_map>
#include <vector>

// External Includes
#include <rtti/factory.h>
#include <rtti/rtti.h>
#include <rtti/deserializeresult.h>
#include <unordered_set>
#include <utility/dllexport.h>

// Core Includes
#include "modulemanager.h"
#include "resourcemanager.h"
#include "service.h"
#include "timer.h"
#include "coreextension.h"

// Name of the file that contains all the settings for the NAP services.
constexpr char SERVICE_CONFIG_FILENAME[] = "config.json";

namespace nap
{
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
	 * Call update inside your app loop to update all available services. When exiting the application
	 * invoke shutdown. This will close all operating services in the reverse order of their dependency tree
	 */
	class NAPAPI Core
	{
		RTTI_ENABLE()
	public:
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
		 * Loads all modules in to the core environment and creates all the associated services
		 * @param error contains the error code when initialization fails
		 * @param forcedDataPath optionally overwrite the project data detection, using specified path instead
		 * @param runningInNonProjectContext indicates if the engine is being run for a non-project use, eg. running Napkin
		 * @return if initialization succeeded
		 */
		bool initializeEngine(utility::ErrorState& error, const std::string& forcedDataPath={}, bool runningInNonProjectContext=false);
		
		/**
		 * Initializes all registered services
		 * Initialization occurs based on service dependencies, this means that if service B depends on Service A,
		 * Service A is initialized before service B etc.
		 * @param errorState contains the error message when initialization fails
		 * @return if initialization failed or succeeded
		 */
		bool initializeServices(utility::ErrorState& errorState);

		/**
		* Shuts down all registered services in the right order
		* Only call this when initializeServices has been called
		*/
		void shutdownServices();

		/**
		 * Initialize python interpreter so we can have components running python scripts
		 */
		bool initializePython(utility::ErrorState& error);

		/**
		 * Starts core, call this after initializing the engine, just before starting
		 * the application loop. This call will start the core timer
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
		* The resource manager holds all the entities and components currently loaded by Core
		* @return the resource manager
		*/
		ResourceManager* getResourceManager() { return mResourceManager.get(); }

		/**
		 * @return the ModuleManager for this core
		 */
		const ModuleManager& getModuleManager() const { return *mModuleManager; }

		/**
		 * @return number of elapsed time in milliseconds after invoking start
		 */
		uint32 getTicks() const;

		/**
		* @return number of elapsed seconds after invoking start
		*/
		double getElapsedTime() const;

		/**
		* @return start time point
		*/
		HighResTimeStamp getStartTime() const;

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
		 * Searches for a service based on given type name, names need to match exactly.
		 * @return an already registered service based on its type name, nullptr if not found
		 * @param type the type of the service as a string
		 */
		Service* getService(const std::string& type);

		/**
		 * Searches for a service of type T, returns a nullptr if not found.
		 * @return a service of type T, returns nullptr if that service can't be found
		 */
		template <typename T>
		T* getService();

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
 		 * Writes a configuration file consisting of all existing service configurations next to the binary executable.
 		 * @param errorState Serialization errors will be logged to errorState.
 		 * @return true on sucess.
		 */
		bool writeConfigFile(utility::ErrorState& errorState);
	
	private:
		/**
		* Helper function that creates all the services that are found in the various modules
		* Note that a module does not need to define a service, only if it has been defined
		* this call will try to create it.
		* @param error contains the error if the services could not be added
		* @return if the services are created successfully
		*/
		bool createServices(utility::ErrorState& errorState);

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
		 * If a service configuration file is provided which is used to initialize the various services
		 * @return if a service configuration file is provided
		 */
		bool hasServiceConfiguration();

		/**
		 * Load the service configuration file
		 * @param deserialize_result contains the result after reading the config file
		 * @param errorState contains the error if deserialization fails
		 * @return if service configuration reading succeeded or not
		 */
		bool loadServiceConfiguration(rtti::DeserializeResult& deserialize_result, utility::ErrorState& errorState);

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
		 * Determine and set our working directory based on where our project data is
		 * @param errorState if false is returned, contains error information
		 * @param forcedDataPath optionally overwrite the project data detection, using specified path instead
		 * @return if the project data was successfully found and working path set
		 */
		bool determineAndSetWorkingDirectory(utility::ErrorState& errorState, const std::string& forcedDataPath=std::string());
		
		/**
		 * Setup our Python environment to find Python in thirdparty for NAP release or NAP source,
		 * or alongside our binary for a packaged project
		 */
		void setupPythonEnvironment();
		
		// Typedef for a list of services
		using ServiceList = std::vector<std::unique_ptr<Service>>;

		// Manages all the loaded modules
		std::unique_ptr<ModuleManager> mModuleManager;

		// Manages all the objects in core
		std::unique_ptr<ResourceManager> mResourceManager;

		// Sorted service nodes, set after init
		ServiceList mServices;

		// Timer
		HighResolutionTimer mTimer;

		// Amount of milliseconds the app is running
		double mLastTimeStamp = 0;

		// Current framerate
		float mFramerate = 0.0f;

		// Used to calculate framerate over time
		std::array<double, 20> mTicks;
		double mTicksum = 0;
		uint32 mTickIdx = 0;

		// Interface associated with this instance of core.
		std::unique_ptr<CoreExtension> mExtension = nullptr;

		nap::Slot<> mPreResourcesLoadedSlot = { [&]() -> void { preResourcesLoaded(); }};
		nap::Slot<> mPostResourcesLoadedSlot = { [&]() -> void { postResourcesLoaded(); }};
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
	Service* new_service = getService(RTTI_OF(T));
	return new_service == nullptr ? nullptr : static_cast<T*>(new_service);
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

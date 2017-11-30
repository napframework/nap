#pragma once

// std includes
#include <unordered_map>
#include <vector>

// External Includes
#include <rtti/rtti.h>
#include <rtti/factory.h>
#include <unordered_set>

// Core Includes
#include "service.h"
#include "timer.h"
#include "modulemanager.h"
#include "utility/dllexport.h"
#include "resourcemanager.h"

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
		 * Constructor
		 */
		Core();

		/**
		 * Destructor
		 */
		virtual ~Core();

		/**
		 * Loads all modules in to the core environment and initializes them in the right order
         * @error contains the error code when initialization fails
		 */
		bool initializeEngine(utility::ErrorState& error);

		/**
		 * Starts core, call this after initializing the engine, just before starting
		 * the application loop. This call will start the core timer
		 */
		void start();

		/**
		 * Updates all services. This happens in 3 distinct steps.
		 * First the resource file is reloaded. After that all services are updated, the last step is the 
		 * the update of the entities and their respective components managed by the resource manager
		 * @param updateFunction application callback that is invoked after updating all the services but before render. Input parameter is deltaTime
		 * @return deltaTime between update calls in seconds
		 */
		double update(std::function<void(double)>& updateFunction);

		/**
		 * Shuts down all registered services in the right order
		 */
		void shutdown();

		/**
		* The resource manager holds all the entities and components currently loaded by Core
		* @return the resource manager
		*/
		ResourceManager* getResourceManager()								{ return mResourceManager.get(); }

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
		TimePoint getStartTime() const;

		/**
		* @return an already registered service of @type
		* @param type the type of service to get
		* @return the service if found, otherwise nullptr
		*/
		Service* getService(const rtti::TypeInfo& type, rtti::ETypeCheck typeCheck = rtti::ETypeCheck::EXACT_MATCH);

		/**
         * Searches for a service based on type name, searches for an exact match.
		 * @return an already registered service based on it's type name, nullptr if not found
		 * @param type the type of the service as a string
		 */
		Service* getService(const std::string& type);

		/**
		 *  @return a service of type T, returns nullptr if that service can't be found
		 */
		template <typename T>
		T* getService(rtti::ETypeCheck typeCheck = rtti::ETypeCheck::EXACT_MATCH);

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
		* @param outServices the list of services the service of @type will be added to
		* @param error in case of a duplicate, contains the error message if the service could not be added
		* @return if the service was added successfully
		*/
		bool addService(const rtti::TypeInfo& type, std::vector<Service*>& outServices, utility::ErrorState& errorState);

		/**
		* Initializes all registered services
		* Initialization occurs based on service dependencies, this means that if service B depends on Service A,
		* Service A is initialized before service B etc.
		* @param error contains the error message when initialization fails
		* @return if initialization failed or succeeded
		*/
		bool initializeServices(utility::ErrorState& errorState);

		/**
		* Occurs when a file has been successfully loaded by the resource manager
		* Forwards the call to all interested services
		* @param file the currently loaded resource file
		*/
		void resourceFileChanged(const std::string& file);

		// Typedef for a list of services
		using ServiceList = std::vector<std::unique_ptr<Service>>;

		// Manages all the loaded modules
		ModuleManager mModuleManager;

		// Manages all the objects in core
		std::unique_ptr<ResourceManager>	mResourceManager;

		// Sorted service nodes, set after init
		ServiceList mServices;

		// Timer
		SimpleTimer mTimer;
		
		// Last time stamp used for calculating delta time
		double mLastTimeStamp = 0.0;

		// Time it took to complete last cycle in seconds
		double mDeltaTime = 0.0;

		nap::Slot<const std::string&> mFileLoadedSlot = { [&](const std::string& inValue) -> void { resourceFileChanged(inValue); } };
	};


	//////////////////////////////////////////////////////////////////////////
	// Template definitions
	//////////////////////////////////////////////////////////////////////////


	// Searches for a service of type T in the services and returns it,
	// returns nullptr if none found
	template <typename T>
	T* Core::getService(rtti::ETypeCheck typeCheck)
	{
		Service* new_service = getService(RTTI_OF(T), typeCheck);
		return new_service == nullptr ? nullptr : static_cast<T*>(new_service);
	}
}


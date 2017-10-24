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
#include "serviceobjectgraphitem.h"
#include "objectgraph.h"
#include "timer.h"
#include "modulemanager.h"
#include "utility/dllexport.h"
#include "resourcemanager.h"

namespace nap
{
	/**
	 * Core is the class that manages entities and services.
	 * Only Core can make entities, as this objects manages entity relationships
	 * Use Core to register a service, start services and stop services.
	 * Allowing for the application to run.
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
		* @return number of elapsed time in milliseconds after
		* creation of core
		*/
		uint32 getTicks() const;

		/**
		* @return number of elapsed seconds after creation of core
		*/
		double getElapsedTime() const;

		/**
		 * @return start time point
		 */
		TimePoint getStartTime() const;

		/**
		 * @return a new or already existing service of @type, nullptr if service can't be created or found
		 * @param type the type name of service to get or create
		 */
		Service* getOrCreateService(const std::string& type);

		/**
		 * @return a new or already existing service of @type, nullptr if service can't be created or found
		 * @param type the type of service to get or create
		 */
        Service* getOrCreateService(const rtti::TypeInfo& type);

		/**
		 * @return a new or already existing service of type T
		 */
		template<typename T>
		T* getOrCreateService();

		/**
		 * Adds a new service of type @type
		 * @param type the type of service to add
		 * @return the newly added service
		 */
        Service& addService(const rtti::TypeInfo& type);

		/**
		 * @return an already registered service of @type
		 * @param type the type of service to get
		 */
		Service* getService(const rtti::TypeInfo& type);

		/**
		 *  @return a service of type T, returns nullptr if that service can't be found
		 */
		template <typename T>
		T* getService();

		/**
		 * Loads all modules in to the core environment 
         * @modulesDir is a path relative to the working directory containing the modules
		 */
		void initializeEngine();

		/**
		 * @return the resource manager
		 * The resource manager holds all the entities and components currently loaded by Core
		 */
		ResourceManager* getResourceManager()				{ return mResourceManager.get(); }

		/**
		 * Initializes all registered services
		 * Initialization occurs based on service dependencies, this means that if service B depends on Service A,
		 * Service A is initialized before service B etc.
		 * @param error contains the error message when initialization fails
		 * @return if initialization failed or succeeded
		 */
		bool initializeServices(utility::ErrorState& error);


	private:
		// Typedef for a list of services
		using ServiceList = std::vector<std::unique_ptr<Service>>;

		// Manages all the loaded modules
		ModuleManager mModuleManager;

		// Manages all the objects in core
		std::unique_ptr<ResourceManager>	mResourceManager;

		// All the services associated with core
		ServiceList mServices;

		// Graph used for prioritizing services
		ObjectGraph<ServiceObjectGraphItem> mServiceGraph;

		// Timer
		SimpleTimer mTimer;
	};


	//////////////////////////////////////////////////////////////////////////
	// Template definitions
	//////////////////////////////////////////////////////////////////////////


	// Searches for a service of type T in the services and returns it,
	// resturns nullptr if none found
	template <typename T>
	T* Core::getService()
	{
		Service* new_service = getService(RTTI_OF(T));
		return new_service == nullptr ? nullptr : static_cast<T*>(new_service);
	}


	// Returns an already existing service or creates a new one using @type
	template <typename T>
	T* Core::getOrCreateService()
	{
		nap::Service* service = getOrCreateService(RTTI_OF(T));
		return service == nullptr ? nullptr : static_cast<T*>(service);
	}
}


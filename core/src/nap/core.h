#pragma once

// std includes
#include <unordered_map>
#include <vector>

// rtti include
#include <rtti/rtti.h>

// Core Includes
#include "component.h"
#include "entity.h"
#include "modulemanager.h"
#include "operator.h"
#include "service.h"
#include "unordered_set"
#include "timer.h"

namespace nap
{
	class ModuleManager;
	/**
	 * Core is the class that manages entities and services.
	 * Only Core can make entities, as this objects manages entity relationships
	 * Use Core to register a service, start services and stop services.
	 * Allowing for the application to run.
	 */
	class Core
	{

	public:
		Core();

		virtual ~Core();

		/**
		 * Clears all children under the current root object
		 */
		void clear();

		/**
		 * Adds a new entity under root
		 */
		Entity& addEntity(const std::string& name);

		/**
		 * @return entity under the root with name @name
		 * @param name: name of the entity under root to get
		 */
		Entity* getEntity(const std::string& name);

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
		 * Add a custom service to the nap core, this has to be called by
		 * service plugin libraries
		 * @param args the arguments used for service construction
		 * @return the newly added service
		 */
		template <typename T, typename... Args>
		T& addService(Args&&... args);

		/**
		 * @return a new or already existing service of @type, nullptr if service can't be created or found
		 * @param type the type of service to get or create
		 */
        Service* getOrCreateService(const RTTI::TypeInfo& type);

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
        Service& addService(const RTTI::TypeInfo& type);

        /**
         * Start all registered services (TODO: DEPRECATED)
         */
		void start();

		/**
		 * Stop all running services
		 */
		void stop();

		/**
		 * If core is running (TODO: DEPRECATED)
		 */
		bool isRunning() const { return mIsRunning; }

        /**
         * Replace the root of this core
		 * @param entity the new root
         */
        void setRoot(Entity& entity);

		/**
		 *  @return the root entity of the system in which all entities are nested.
		 */
		Entity& getRoot() { return *mRoot; }

		/**
		 * @return an already registered service of @type
		 * @param type the type of service to get
		 */
		Service* getService(const RTTI::TypeInfo& type);

		/**
		 *  @return a service of type T, returns nullptr if that service can't be found
		 */
		template <typename T>
		T* getService();

		/**
		 * @return the service associated with type @inType
		 * @param inType the type to get associated service for
		 */
		Service* getServiceForType(const RTTI::TypeInfo& inType);

		/**
		 * @return the service associated with type T, nullptr if no service is associated
		 */
		template <typename T>
		Service* getServiceForType();

		/**
		 * Register a type to be associated with a specific service
		 * This call is performed by services that need to be registered with core
		 * @param inService the service to associated the type with
		 * @param typeInfo the type to associate with the service
		 */
		void registerType(const Service& inService, RTTI::TypeInfo typeInfo);

		/**
		 * @return a list of all available component types
		 */
		std::vector<RTTI::TypeInfo> getComponentTypes() const;

		/**
		 * @return the manager that manages all registered modules (aka plugins)
		 */
		ModuleManager& getModuleManager()							{ return mModuleManager; }

		/**
		 * Loads all modules in to the core environment 
		 */
        void initialize()											{
            mModuleManager.loadModules();
        }

	private:
		// Typedef for a list of services
		using ServiceList = std::vector<std::unique_ptr<Service>>;

		// Typedef for types associated with a service
		using ServiceTypeMap = std::unordered_map<std::string, std::unordered_set<RTTI::TypeInfo>>;

		// Type store with all registered components for every service
		ServiceTypeMap mTypes;

		// All the services associated with core
		ServiceList mServices;

		// If the world is running
		bool mIsRunning = false;

		// The single root entity
		std::unique_ptr<Entity> mRoot = nullptr;

		ModuleManager mModuleManager;

		// Timer
		SimpleTimer mTimer;
	};


	//////////////////////////////////////////////////////////////////////////
	// Template definitions
	//////////////////////////////////////////////////////////////////////////

	// Templated function to add a service to the core
	template <typename T, typename... Args>
	T& Core::addService(Args&&... inArguments)
	{
		// Check for existing service
		T* existing_service = getService<T>();
		if (existing_service != nullptr)
		{
			nap::Logger::warn("cant add service of type: %s, service already exists", RTTI_OF(T).getName().c_str());
			return *existing_service;
		}

		// Create new service
		auto unique_ptr = std::make_unique<T>(std::forward<Args>(inArguments)...);

		// set the core pointer of the service
		unique_ptr->mCore = this;

		// This is deprecated, use registerTypes instead!
		T::sRegisterTypes(*this, *unique_ptr);

		// Correct way of doing it
		unique_ptr->registerTypes(*this);
		
		// Get member
		T& return_v = *unique_ptr;

		// Add unique
		mServices.emplace_back(std::move(unique_ptr));

		// Return ref
		return return_v;
	}


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


	// Returns the service for type T
	template <typename T>
	Service* Core::getServiceForType()
	{
		return getServiceForType(RTTI_OF(T));
	}
}


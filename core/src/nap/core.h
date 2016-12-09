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

namespace nap
{
	class ModuleManager;
	//	Core is the class that manages entities and services.
	//	Only Core can make entities, as this objects manages entity relationships
	//	Use Core to register a service, start services and stop services.
	//	Allowing for the application to run.
	class Core
	{

	public:
		Core();

		virtual ~Core();

		void clear();

		Entity& addEntity(const std::string& name);

		Entity* getEntity(const std::string& name);

		// Add a custom service to the nap core, this has to be called by
		// service plugin libraries
		template <typename T, typename... Args>
		T& addService(Args&&... args);

        template<typename T>
        T& getOrCreateService() 
		{
			nap::Service& service = getOrCreateService(RTTI_OF(T));
			return static_cast<T&>(service);
        }

        Service& getOrCreateService(const RTTI::TypeInfo& type);

        Service& addService(const RTTI::TypeInfo& type);

        // Starts all registerd services.
		void start();

		// Stops all running services.
		void stop();

		// Tells wether the core (and all registered services) is running.
		bool isRunning() const { return mIsRunning; }

        // Replace the root of this core
        void setRoot(Entity& entity);

		// Returns the root entity of the system in which all entities are
		// nested.
		Entity& getRoot() { return *mRoot; }

		// Returns a service by type using RTTI, returns nullptr if the service
		// is not registered
		template <typename T>
		T* getService();

		// Find the service belonging to a specific type
		Service* getServiceForType(const RTTI::TypeInfo& inType);

		// Find the service for a type of type T
		template <typename T>
		Service* getServiceForType();

		// Register type from service
		void registerType(const Service& inService, RTTI::TypeInfo);

		// Retrieve all registered component types.
		std::vector<RTTI::TypeInfo> getComponentTypes() const;

		ModuleManager& getModuleManager() { return mModuleManager; }

        void initialize() { mModuleManager.loadModules(); }

	private:
		// Typedef for a list of services
		using ServiceList = std::vector<std::unique_ptr<Service>>;

		// Typedef for types associated with a service
		using ServiceTypeMap =
			std::unordered_map<std::string, std::vector<RTTI::TypeInfo>>;

		// Type store with all registered components for every service
		ServiceTypeMap mTypes;

		// All the services associated with core
		ServiceList mServices;

		// If the world is running
		bool mIsRunning = false;

		// The single root entity
		std::unique_ptr<Entity> mRoot = nullptr;

		ModuleManager mModuleManager;
	};



	// Templated function to add a service to the core
	template <typename T, typename... Args>
	T& Core::addService(Args&&... inArguments)
	{
		// Create new service
		auto unique_ptr =
			std::make_unique<T>(std::forward<Args>(inArguments)...);

		// set the core pointer of the service
		unique_ptr->mCore = this;

		// Register service types if added for first time
		if (!getService<T>()) T::sRegisterTypes(*this, *unique_ptr);

		// Get member
		T& return_v = *unique_ptr;

		// Add unique
		mServices.emplace_back(std::move(unique_ptr));

		// Return ref
		return return_v;
	}





	//! Searches for a service of type T in the services and returns it,
	//! resturns nullptr if none found
	template <typename T>
	T* Core::getService()
	{
		for (auto& service : mServices)
			if (service->getTypeInfo().isKindOf<T>())
				return dynamic_cast<T*>(service.get());
		return nullptr;
	}


	//! Returns the service for type T
	template <typename T>
	Service* Core::getServiceForType()
	{
		return getServiceForType(RTTI_OF(T));
	}
}


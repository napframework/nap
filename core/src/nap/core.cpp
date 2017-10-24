// Local Includes
#include "core.h"
#include "resourcemanager.h"
#include "logger.h"
#include "service.h"

// External Includes
#include <rtti/pythonmodule.h>
#include <iostream>

using namespace std;

RTTI_BEGIN_CLASS(nap::Core)
	RTTI_FUNCTION("getOrCreateService", (nap::Service* (nap::Core::*)(const std::string&))&nap::Core::getOrCreateService)
	RTTI_FUNCTION("getResourceManager", &nap::Core::getResourceManager)
RTTI_END_CLASS


namespace nap
{
	/**
	@brief Constructor

	Creates a default entity as the root
	**/
	Core::Core()
	{
		// Initialize timer
		mTimer.start();

		// Add resource manager service
		mResourceManager = std::make_unique<ResourceManager>(*this);
	}
	
    
	Core::~Core()
	{
		// In order to ensure a correct order of destruction we want our entities, components, etc. to be deleted before other services are deleted.
		// Because entities and components are managed and owned by the resource manager we explicitly delete this first.
		// Erase it
		mResourceManager = nullptr;
	}
	
    
	void Core::initializeEngine()
	{ 
		mModuleManager.loadModules();

		// Here we register a callback that is called when the nap python module is imported.
		// We register a 'core' attribute so that we can write nap.core.<function>() in python
		// to access core functionality as a 'global'.
		nap::rtti::PythonModule::get("nap").registerImportCallback([this](pybind11::module& module)
		{
			module.attr("core") = this;
		});
	}


	bool Core::initializeServices(utility::ErrorState& error)
	{
		std::vector<Service*> objects;
		for (const auto& service : mServices)
		{
			objects.emplace_back(service.get());
		}

		// Build service dependency graph
		bool success = mServiceGraph.build(objects, [=](Service* service) 
		{
			 return ServiceObjectGraphItem::create(service, this); 
		}, error);

		// Make sure the graph was successfully build
		if (!error.check(success, "unable to build service dependency graph"))
			return false;

		// Initialize all services
		for (auto& node : mServiceGraph.getSortedNodes())
		{
			nap::Service* service = node->mItem.mObject;
			if (!service->init(error))
				return false;
		}
		return true;
	}


	// Returns service that matches @type
	Service* Core::getService(const rtti::TypeInfo& type)
	{
		// Find service of type 
		const auto& found_service = std::find_if(mServices.begin(), mServices.end(), [&type](const auto& service)
		{
			return service->get_type().is_derived_from(type.get_raw_type());
		});

		// Check if found
		return found_service == mServices.end() ? nullptr : (*found_service).get();
	}


	// Add a new service
	Service& Core::addService(const rtti::TypeInfo& type)
	{
        assert(type.is_valid());
		assert(type.can_create_instance());
		assert(type.is_derived_from<Service>());

		// Check if service doesn't already exist
		nap::Service* existing_service = getService(type);
		if (existing_service != nullptr)
		{
			nap::Logger::warn("can't add service of type: %s, service already exists", type.get_name().data());
			return *existing_service;
		}

		// Add service
		Service* service = type.create<Service>();
		service->mCore = this;
		service->registerObjectCreators(mResourceManager->getFactory());

		// Add service
		mServices.emplace_back(std::unique_ptr<Service>(service));
		service->created();
		return *service;
	}


	Service* Core::getOrCreateService(const std::string& type)
	{
		return getOrCreateService(rtti::TypeInfo::get_by_name(type));
	}


	// Creates a new service of type @type if doesn't exist yet
	Service* Core::getOrCreateService(const rtti::TypeInfo& type)
	{
		// Otherwise add
		if (!type.is_derived_from(RTTI_OF(nap::Service)))
		{
			nap::Logger::fatal("can't add service, service not of type: %s", RTTI_OF(Service).get_name().data());
			return nullptr;
		}

		// Find registered service
		Service* found_service = getService(type);
		if (found_service != nullptr)
		{
			return found_service;
		}

		// Add new service of type
        return &addService(type);
	}


	// return number of elapsed ticks
	uint32 Core::getTicks() const
	{
		return mTimer.getTicks();
	}


	// Return elapsed time
	double Core::getElapsedTime() const
	{
		return mTimer.getElapsedTime();
	}


	// Returns start time of core module as point in time
	TimePoint Core::getStartTime() const
	{
		return mTimer.getStartTime();
	}
}

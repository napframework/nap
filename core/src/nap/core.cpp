// Local Includes
#include "core.h"

// External Includes
#include <algorithm>

using namespace std;

namespace nap
{

	/**
	@brief Stops all associated services and clears root
	**/
	Core::~Core()
	{
		if (mIsRunning) stop();

		// make sure the entities are deleted before the services are destructed, otherwise entities can try
		// deregistering themselves with destructed services!
		mRoot = nullptr;
	}


	/**
	@brief Constructor

	Creates a default entity as the root
	**/
	Core::Core()
	{
        getModuleManager().loadCoreModule();

		// the root entity has no parent: nullptr
		mRoot = std::unique_ptr<Entity>(new Entity(*this));
		mRoot->mName = "root";

		// Initialize timer
		mTimer.start();
	}


	/**
	@brief Start core
	**/
	void Core::start()
	{
		mIsRunning = true;
		for (auto& service : mServices)
		{
			service->start();
		}
	}


	/**
	@brief Stop core
	**/
	void Core::stop()
	{
		mIsRunning = false;
		for (auto& service : mServices)
			service->stop();
	}


	std::vector<rtti::TypeInfo> Core::getComponentTypes() const
	{
		rtti::TypeInfo componentType = rtti::TypeInfo::get<Component>();
		return componentType.get_raw_derived_classes();
	}


	/**
	@brief Find the service associated with the RTTI of type

	TODO: This method is only here for serviceable component and can go
	Every serviceable component should list its own services of interest
	**/
	Service* Core::getServiceForType(const rtti::TypeInfo& inType)
	{
		// Get raw type to search for
		rtti::TypeInfo raw_search_type = inType.get_raw_type();
		if (!raw_search_type.is_valid()) 
		{
			Logger::warn("Unable to determine object type, can't retrieve service");
			return nullptr;
		}

		// Service name
		std::string found_service;
		for (auto& v : mTypes) 
		{
			// Find type in service
			const auto& match_type = std::find_if(v.second.begin(), v.second.end(), [&](const rtti::TypeInfo& info) 
			{
				return raw_search_type.is_derived_from(info.get_raw_type());
			});

			// Check if the type was found in the service list
			if (match_type != v.second.end()) 
			{
				found_service = v.first;
				break;
			}
		}

		// if no service if found, return null
		if (found_service.empty()) return nullptr;

		// Return the service if found
		const auto& v = std::find_if(mServices.begin(), mServices.end(), [&](const auto& service) 
		{ 
			return service->getTypeName() == found_service; 
		});
		return v == mServices.end() ? nullptr : (*v).get();
	}


	/**
	@brief Register a type associated with a service
	 TODO: Move registration into actual service
	**/
	void Core::registerType(const Service& inService, rtti::TypeInfo inTypeInfo)
	{
		// Find and add
		auto it = mTypes.find(inService.getTypeName());
		if (it != mTypes.end()) 
		{
			if (it->second.find(inTypeInfo) != it->second.end())
			{
				nap::Logger::warn("type: %s already registered with service: %s", inTypeInfo.get_name().data(), inService.get_type().get_name().data());
			}
			it->second.emplace(inTypeInfo);
			return;
		}

		// Add new key with type info if new
		mTypes[inService.getTypeName()] = {inTypeInfo};
	}


	Entity& Core::addEntity(const std::string& name) 
	{ 
		return mRoot->addEntity(name); 
	}


	Entity* Core::getEntity(const std::string& name) 
	{ 
		return mRoot->getEntity(name); 
	}

	
	void Core::setRoot(Entity& entity) 
	{ 
		mRoot = std::unique_ptr<Entity>(&entity); 
	}


	void Core::clear() 
	{ 
		mRoot->clearChildren(); 
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
		service->registerTypes(*this);

		// Add service
		mServices.emplace_back(std::unique_ptr<Service>(service));
		service->initialized();
		return *service;
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

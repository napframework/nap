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
		// the root entity has no parent: nullptr
		mRoot = std::unique_ptr<Entity>(new Entity(*this));
		mRoot->mName = "root";
	}



	/**
	@brief Start core
	**/
	void Core::start()
	{
		mIsRunning = true;
		for (auto& service : mServices)
			service->start();
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


	std::vector<RTTI::TypeInfo> Core::getComponentTypes() const
	{
		return RTTI::TypeInfo::getRawTypes(RTTI::TypeInfo::get<Component>());
	}


	/**
	@brief Find the service associated with the RTTI of type

	TODO: This method is only here for serviceable component and can go
	Every serviceable component should list its own services of interest
	**/
	Service* Core::getServiceForType(const RTTI::TypeInfo& inType)
	{
		// Get raw type to search for
		RTTI::TypeInfo raw_search_type = inType.getRawType();
		if (!raw_search_type.isValid()) {
			Logger::warn("Unable to determine object type, can't retrieve service");
			return nullptr;
		}

		// Service name
		std::string found_service;
		for (auto& v : mTypes) {
			// Find type in service
			const auto& match_type = std::find_if(v.second.begin(), v.second.end(), [&](const RTTI::TypeInfo& info) {
				return raw_search_type.isKindOf(info.getRawType());
			});

			// Check if the type was found in the service list
			if (match_type != v.second.end()) {
				found_service = v.first;
				break;
			}
		}

		// if no service if found, return null
		if (found_service.empty()) return nullptr;

		// Return the service if found
		const auto& v = std::find_if(mServices.begin(), mServices.end(),
									 [&](const auto& service) { return service->getTypeName() == found_service; });
		return v == mServices.end() ? nullptr : (*v).get();
	}



	/**
	@brief Register a type associated with a service
	**/
	void Core::registerType(const Service& inService, RTTI::TypeInfo inTypeInfo)
	{
		// Find and add
		auto it = mTypes.find(inService.getTypeName());
		if (it != mTypes.end()) {
			it->second.emplace_back(inTypeInfo);
			return;
		}

		// Add new key with type info if new
		mTypes[inService.getTypeName()] = {inTypeInfo};
	}

	Entity& Core::addEntity(const std::string& name) { return mRoot->addEntity(name); }

	Entity* Core::getEntity(const std::string& name) { return mRoot->getEntity(name); }

	void Core::setRoot(Entity& entity) { mRoot = std::unique_ptr<Entity>(&entity); }

	void Core::clear() { mRoot->clearChildren(); }


	Service& Core::addService(const RTTI::TypeInfo& type)
	{
        assert(type.isValid());
		assert(type.canCreateInstance());
		assert(type.isKindOf<Service>());
		Service* service = static_cast<Service*>(type.createInstance());
		service->mCore = this;
		mServices.emplace_back(std::unique_ptr<Service>(service));
		return *service;
	}


	Service& Core::getOrCreateService(const RTTI::TypeInfo& type)
	{
		Service* srv = getServiceForType(type);
        if (srv)
		{
			return *srv;
		}

		if (!type.isKindOf(RTTI_OF(nap::Service)))
		{
			nap::Logger::fatal("can't add service, service not of type: %s", RTTI_OF(Service).getName().c_str());
		}

        return addService(type);
	}
}
// Local Includes
#include "logger.h"
#include "service.h"
#include "configure.h"
#include "serviceablecomponent.h"

RTTI_DEFINE(nap::Service)

namespace nap
{
	/**
	@brief Destructor
	**/
	Service::~Service()
	{
		if (mIsRunning) stop();
	}


	/**
	@brief Returns the core associated with this service
	**/
	Core& Service::getCore()
	{
		assert(mCore);
		return *mCore;
	}


	/**
	@brief Starts the service
	**/
	void Service::start()
	{
		mIsRunning = true;
		onStart();
	}


	/**
	@brief Stops the service
	**/
	void Service::stop()
	{
		mIsRunning = false;
		onStop();
	}

	
	/**
	 * This function is called when the service is created
	 * Override to register all types that are important to the service
	 * to function properly.
	 */
	void Service::registerTypes(nap::Core& core)
	{
		nap::Logger::info("type registration not implemented by service: %s", this->getTypeName().c_str());
		return;
	}


	/**
	@brief Registers a new component associated with the service
	**/
	void Service::registerObject(Object& object)
	{ 
		// Get or create component using emplace
		auto it = mObjects.emplace(std::make_pair(object.getTypeInfo().getRawType(), ObjectList()));
		(it.first)->second.emplace_back(&object);

		// Make sure we know if it's removal
		object.removed.connect(mRemoved);

		// Let derived service handle possible changes
		objectRegistered(object);
	}


	/**
	@brief De-registers an existing component associated with the service
	**/
	void Service::removeObject(Object& object)
	{
		// Find component container
		RTTI::TypeInfo info = object.getTypeInfo().getRawType();
		std::string name = info.getName();

		auto it = mObjects.find(info);
		if (it == mObjects.end())
			return;

		// Fetch component list associated with type
		ObjectList& ass_obj = (*it).second;

		// Find position
		auto cit = std::find_if(ass_obj.begin(), ass_obj.end(),
			[&](const auto& c_object) { return c_object == &object; });

		// Remove
		if (cit == ass_obj.end()) {
			Logger::warn("Unable to remove object: " + object.getName() + ", not registered with service: " +
						 this->getTypeName());
			return;
		}

		// Remove component at found position
		ass_obj.erase(cit);

		// Let derived services handle removal
		objectRemoved(object);
	}


	// Called just before a registered component is removed
	void Service::removed(nap::Object& object)
	{
		// Remove from list
		removeObject(object);
	}


	/**
	@brief Returns all valid components matching the @inInfo criteria

	Note that this function is fast, it uses the RTTI Map to figure out if components registered with the service are of type @inType
	It returns the total amount of compatible components + a list of components lists vector<vector<Component*>>
	**/
	size_t Service::getTypeFilteredObjects(const RTTI::TypeInfo& inInfo, std::vector<ObjectList*>& outObjects)
	{
		// Get raw compare type
		outObjects.clear();
		RTTI::TypeInfo search_type = inInfo.getRawType();
		if (!search_type.isKindOf<nap::Object>())
		{
			Logger::warn("not of type: " + search_type.getName());
			return 0;
		}

		// Figure out size
		size_t size(0);
		for (auto& i : mObjects)
		{
			if (i.first.isKindOf(search_type))
			{
				size += i.second.size();
				outObjects.emplace_back(&(i.second));
			}
		}
		return size;
	}


	/**
	@brief Get all components associated with RTTI inTypeInfo
	**/
	void Service::getObjects(const RTTI::TypeInfo& inTypeInfo, std::vector<Object*> outObjects)
	{
		// Clear
		outObjects.clear();

		// Figure out size
		std::vector<ObjectList*> valid_objects;
		size_t size = getTypeFilteredObjects(inTypeInfo, valid_objects);

		// Return if empty
		if (size == 0) return;

		// Otherwise reserve and insert
		outObjects.reserve(size);
		
		// Add all valid components
		for (auto& c : valid_objects)
		{
			outObjects.insert(std::end(outObjects), std::begin(*c), std::end(*c));
		}
	}
}
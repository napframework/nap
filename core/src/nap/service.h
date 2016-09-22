#pragma once

#include <nap/component.h>
#include <nap/operator.h>
#include <rtti/rtti.h>
#include <set>
#include <nap/configure.h>
#include <nap/signalslot.h>

namespace nap
{
	// Forward Declares
	class Core;

	/**
	 @brief Service
	 A Service is a process within the core that cooperates with certain components in the system, this is the base
	 class for all services
	 **/

	class Service
	{
		RTTI_ENABLE()
		friend class Core;
		friend class ServiceableComponent;
        friend class ServiceableOperator;

	public:
        // Virtual destructor because of virtual methods!
		virtual ~Service();

		// tells if the service has been started by the nap Core
		bool isRunning() const { return mIsRunning; }

		// Returns the nap Core this service belongs to
		Core& getCore();

		// Service type name
		const std::string getTypeName() const { return RTTI::TypeInfo::get(*this).getName(); };

		// Component search filter function
        using ObjectFilterFunction = std::function<bool(Object&, Core&)>;

		// Get all components and operators associated with type T
		template <typename T>
		void getObjects(std::vector<T*>& outObjects, ObjectFilterFunction inFilter = nullptr);

		// Get all components associated with RTTI inTypeInfo
		void getObjects(const RTTI::TypeInfo& inTypeInfo, std::vector<Object*> outObjects);

		// Register a newly added object
		virtual void registerObject(Object& object);

		// De-register an added object
		virtual void removeObject(Object& object);

	protected:
		// This method is being called when the service is started by the core.
		virtual void onStart(){};

		// This method is being called when the service is stopped by the core.
		virtual void onStop(){};

		// Starts the service
		void start();

		// Stops the servicef
		void stop();

		// Created components
		using ObjectList = std::vector<Object*>;
		using ObjectMap = std::unordered_map<RTTI::TypeInfo, ObjectList>;
		

		// all the registered (currently available) objects
		ObjectMap mObjects;

		NSLOT(mRemoved, nap::Object&, removed)
		void removed(nap::Object& obj);

		// Virtuals called when components are added / removed
		virtual void objectRegistered(Object& inObject)		{ };
		virtual void objectRemoved(Object& inObject)		{ };

	private:
		// if the service is currently active
		bool mIsRunning = false;

		// this variable will be set by the core when the service is added
		Core* mCore = nullptr;

		// Returns all valid components as a list based on @inInfo
		size_t getTypeFilteredObjects(const RTTI::TypeInfo& inInfo, std::vector<ObjectList*>& outObjects);
	};


	//////////////////////////////////////////////////////////////////////////
	// Template Definitions
	//////////////////////////////////////////////////////////////////////////

	// Get all components associated with type T
	template <typename T>
	void Service::getObjects(std::vector<T*>& outObjects, ObjectFilterFunction inFilter)
	{
		// Clear
		outObjects.clear();

		// Figure out size
		std::vector<ObjectList*> valid_objects;
		size_t size = getTypeFilteredObjects(RTTI_OF(T), valid_objects);

		// Return if empty
		if (size == 0) return;

		// Otherwise reserve and insert
		outObjects.reserve(size);

		// Add all valid components
		for (auto& list : valid_objects)
		{
			for (auto& object : *list)
			{
				if(inFilter && !inFilter(*object, getCore()))
					continue;

				outObjects.emplace_back(static_cast<T*>(object));
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////

#define NAP_DECLARE_SERVICE() \
	\
friend class nap::Core;            \
	\
protected:                    \
	static void sRegisterTypes(nap::Core& inCore, const nap::Service& inService);

//////////////////////////////////////////////////////////////////////////


RTTI_DECLARE_BASE(nap::Service)

#pragma once

/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "entity.h"
#include <rtti/objectptr.h>

namespace nap
{
	class EntityPtr
	{
	public:
		EntityPtr() = default;

		EntityPtr(Entity* entity) :
			mResource(entity)
		{
		}

		const std::string& getInstancePath() const { return mPath; }

		/**
		 * Convert the pointer to a string for serialization
		 * @return The string representation of this object
		 */
		std::string toString() const
		{
			return mPath;
		}

		/**
		 * Convert the full target ID as specified to an ID that can be resolved to an object
		 *
		 * @param targetID The target ID to translate
		 * @return The translated ID
		 */
		static std::string translateTargetID(const std::string& targetID);

		/**
		 * Assign the target ID & object to this pointer. Used for pointer resolving by the ResourceManager, should not be called manually (is only public so that we can register it in RTTI)
		 * @param targetID The ID of the target
		 * @param targetObject The pointer to be assigned
		 */
		void assign(const std::string& targetID, rtti::Object& targetObject)
		{
			mPath = targetID;
			mResource = rtti_cast<Entity>(&targetObject);
		}

		const Entity& operator*() const
		{
			assert(mResource != nullptr);
			return *mResource;
		}

		Entity& operator*()
		{
			assert(mResource != nullptr);
			return *mResource;
		}

		const Entity* operator->() const
		{
			assert(mResource != nullptr);
			return mResource.get();
		}

		Entity* operator->()
		{
			assert(mResource != nullptr);
			return mResource.get();
		}

		bool operator==(const EntityPtr& other) const
		{
			return mResource == other.mResource;
		}

		bool operator==(std::nullptr_t) const
		{
			return mResource == nullptr;
		}

		bool operator!=(const EntityPtr& other) const
		{
			return mResource != other.mResource;
		}

		bool operator!=(std::nullptr_t) const
		{
			return mResource != nullptr;
		}

		bool operator<(const EntityPtr& other) const
		{
			return mResource < other.mResource;
		}

		bool operator>(const EntityPtr& other) const
		{
			return mResource > other.mResource;
		}

		bool operator<=(const EntityPtr& other) const
		{
			return mResource <= other.mResource;
		}

		bool operator>=(const EntityPtr& other) const
		{
			return mResource >= other.mResource;
		}

		Entity* get() const
		{
			return mResource.get();
		}

		Entity* get()
		{
			return mResource.get();
		}

	private:
		rtti::ObjectPtr<Entity>	mResource;		///< Pointer to the target resource
		std::string				mPath;			///< Path in the entity hierarchy, either relative or absolute
	};


	/**
	* The EntityInstancePtrInitProxy is a proxy object that is only used to pass arguments to the correct constructor of EntityInstancePtr
	* It's only here so that we can use initEntityInstancePtr to initialize both regular EntityInstancePtrs, as well as vectors of EntityInstancePtr.
	*/
	template<typename SourceComponentType>
	struct EntityInstancePtrInitProxy
	{
		ComponentInstance*					mSourceComponentInstance;						///< The ComponentInstance that the EntityInstancePtr is located in
		EntityPtr							SourceComponentType::*mEntityMemberPointer;		///< Member pointer to the EntityPtr located in the Component
	};

	class EntityInstancePtr
	{
	public:
		EntityInstancePtr() = default;

		template<class SourceComponentType>
		EntityInstancePtr(ComponentInstance* sourceComponentInstance, EntityPtr(SourceComponentType::*entityMemberPointer))
		{
			SourceComponentType* resource = sourceComponentInstance->getComponent<SourceComponentType>();
			EntityPtr& target_entity_resource = resource->*entityMemberPointer;

			sourceComponentInstance->addToEntityLinkMap(target_entity_resource.get(), target_entity_resource.getInstancePath(), &mInstance);
		}

		/**
		* Construct a EntityInstancePtr from a EntityInstancePtrInitProxy, which can be retrieved through initEntityInstancePtr.
		*/
		template<class SourceComponentType>
		EntityInstancePtr(const EntityInstancePtrInitProxy<SourceComponentType>& proxy) :
			EntityInstancePtr(proxy.mSourceComponentInstance, proxy.mEntityMemberPointer)
		{
		}

		const EntityInstance& operator*() const
		{
			assert(mInstance != nullptr);
			return *mInstance;
		}

		EntityInstance& operator*()
		{
			assert(mInstance != nullptr);
			return *mInstance;
		}

		EntityInstance* operator->() const
		{
			assert(mInstance != nullptr);
			return mInstance;
		}

		EntityInstance* operator->()
		{
			assert(mInstance != nullptr);
			return mInstance;
		}

		bool operator==(const EntityInstancePtr& other) const
		{
			return mInstance == other.mInstance;
		}

		bool operator==(std::nullptr_t) const
		{
			return mInstance == nullptr;
		}

		bool operator!=(const EntityInstancePtr& other) const
		{
			return mInstance != other.mInstance;
		}

		bool operator!=(std::nullptr_t) const
		{
			return mInstance != nullptr;
		}

		bool operator<(const EntityInstancePtr& other) const
		{
			return mInstance < other.mInstance;
		}

		bool operator>(const EntityInstancePtr& other) const
		{
			return mInstance > other.mInstance;
		}

		bool operator<=(const EntityInstancePtr& other) const
		{
			return mInstance <= other.mInstance;
		}

		bool operator>=(const EntityInstancePtr& other) const
		{
			return mInstance >= other.mInstance;
		}

		EntityInstance* get() const
		{
			return mInstance;
		}

		EntityInstance* get()
		{
			return mInstance;
		}

	private:
		template<typename SourceComponentType_>
		friend std::vector<EntityInstancePtr> initEntityInstancePtr(ComponentInstance* sourceComponentInstance, std::vector<EntityPtr>(SourceComponentType_::*entityMemberPointer));

		EntityInstance* mInstance = nullptr;
	};

	/**
	* Init a regular EntityInstancePtr. Returns a EntityInstancePtrInitProxy which is a simple wrapper around the function arguments.
	* The return value is passed directly to the corresponding constructor on EntityInstancePtr, which does the actual initialization.
	*
	* @param sourceComponentInstance The ComponentInstance that the EntityInstancePtr being initialized is a member of
	* @param entityMemberPointer Member pointer to the EntityPtr member of the Component
	*
	* @return A EntityInstancePtrInitProxy which can be passed to the EntityInstancePtr constructor
	*/
	template<typename SourceComponentType>
	EntityInstancePtrInitProxy<SourceComponentType> initEntityInstancePtr(ComponentInstance* sourceComponentInstance, EntityPtr(SourceComponentType::*entityMemberPointer));

	/**
	* Init a std::vector of EntityInstancePtrs. Returns a std::vector which is then used to initialize the target std::vector of EntityInstancePtrs
	*
	* @param sourceComponentInstance The ComponentInstance that the EntityInstancePtr being initialized is a member of
	* @param entityMemberPointer Member pointer to the EntityPtr member of the Component
	*
	* @return std::vector of initialized EntityInstancePtrs
	*/
	template<typename SourceComponentType>
	std::vector<EntityInstancePtr> initEntityInstancePtr(ComponentInstance* sourceComponentInstance, std::vector<EntityPtr>(SourceComponentType::*entityMemberPointer));
}


//////////////////////////////////////////////////////////////////////////
// The following construct is required to support EntityPtr in RTTR as a regular pointer.
//////////////////////////////////////////////////////////////////////////
namespace rttr
{
	template<>
	struct wrapper_mapper<nap::EntityPtr>
	{
		using wrapped_type = nap::Entity*;
		using type = nap::EntityPtr;

		inline static wrapped_type get(const type& obj)
		{
			return obj.get();
		}

		inline static type create(const wrapped_type& value)
		{
			return nap::EntityPtr(value);
		}
	};
}


//////////////////////////////////////////////////////////////////////////
// Template Definitions
//////////////////////////////////////////////////////////////////////////
namespace nap
{
	template<typename SourceComponentType>
	nap::EntityInstancePtrInitProxy<SourceComponentType>
		initEntityInstancePtr(ComponentInstance* sourceComponentInstance, EntityPtr(SourceComponentType::*entityMemberPointer))
	{
		return{ sourceComponentInstance, entityMemberPointer };
	}


	template<typename SourceComponentType>
	std::vector<nap::EntityInstancePtr>
		initEntityInstancePtr(ComponentInstance* sourceComponentInstance, std::vector<EntityPtr>(SourceComponentType::*entityMemberPointer))
	{
		SourceComponentType* resource = sourceComponentInstance->getComponent<SourceComponentType>();
		std::vector<EntityPtr>& target_entity_resource = resource->*entityMemberPointer;

		std::vector<EntityInstancePtr> result;
		result.resize(target_entity_resource.size());

		for (int i = 0; i != result.size(); ++i)
			sourceComponentInstance->addToEntityLinkMap(target_entity_resource[i].get(), target_entity_resource[i].getInstancePath(), &result[i].mInstance);

		return result;
	}

}

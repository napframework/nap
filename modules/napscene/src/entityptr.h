#pragma once

#include "rtti/objectptr.h"
#include "entity.h"

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
		void assign(const std::string& targetID, rtti::RTTIObject& targetObject)
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
		std::string			mPath;			///< Path in the entity hierarchy, either relative or absolute
	};


	class EntityInstancePtr
	{
	public:
		template<class SourceComponentType>
		EntityInstancePtr(ComponentInstance* sourceComponentInstance, EntityPtr(SourceComponentType::*entityMemberPointer))
		{
			SourceComponentType* resource = sourceComponentInstance->getComponent<SourceComponentType>();
			EntityPtr& target_entity_resource = resource->*entityMemberPointer;

			sourceComponentInstance->addToEntityLinkMap(target_entity_resource.get(), target_entity_resource.getInstancePath(), &mInstance);
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
		EntityInstance* mInstance = nullptr;
	};
}

/**
* The following construct is required to support EntityPtr in RTTR as a regular pointer.
*/
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
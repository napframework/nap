#pragma once

#include "objectptr.h"

namespace nap
{
	class Entity;
	class EntityInstance;

	class Component;
	class ComponentInstance;

	/**
	 * EntityPtr is used to access EntityInstance object. Because EntityInstances are spawned at runtime, the file
	 * objects have no knowledge of instances. To make sure that we can point to EntityInstances, EntityPtr
	 * wraps both a pointer to an Entity and to an EntityInstance. From an RTTI perspective, EntityPtr acts
	 * like a pointer to another Entity, while at runtime, the pointer acts like a pointer to an EntityInstance.
	 */
	template<typename RESOURCE_TYPE, typename INSTANCE_TYPE>
	class InstancePtr
	{
	public:
		using InstancePtrType = InstancePtr<RESOURCE_TYPE, INSTANCE_TYPE>;

		InstancePtr() = default;

		// Regular ptr Ctor
		InstancePtr(RESOURCE_TYPE* ptr) :
			mResource(ptr)
		{
		}

		// Copy ctor
		InstancePtr(const InstancePtrType& other)
		{
			Assign(other);
		}

		// Move ctor
		InstancePtr(InstancePtrType&& other)
		{
			Assign(other);
			other.mResource = nullptr;
			other.mInstance = nullptr;
		}

		// Assignment operator
		InstancePtrType& operator=(const InstancePtrType& other)
		{
			Assign(other);
			return *this;
		}

		// Move assignment operator
		InstancePtrType& operator=(InstancePtrType&& other)
		{
			Assign(other);
			other.mResource = nullptr;
			other.mInstance = nullptr;
			return *this;
		}

		//////////////////////////////////////////////////////////////////////////

		const INSTANCE_TYPE& operator*() const
		{
			assert(mInstance != nullptr);
			return *mInstance;
		}

		INSTANCE_TYPE& operator*()
		{
			assert(mInstance != nullptr);
			return *mInstance;
		}

		INSTANCE_TYPE* operator->() const
		{
			assert(mInstance != nullptr);
			return mInstance;
		}

		INSTANCE_TYPE* operator->()
		{
			assert(mInstance != nullptr);
			return mInstance;
		}

		bool operator==(const InstancePtrType& other) const
		{
			return mResource == other.mResource && mInstance == other.mInstance;
		}

		bool operator==(const INSTANCE_TYPE* entityInstance) const
		{
			return mInstance == entityInstance;
		}

		bool operator!=(const INSTANCE_TYPE* entityInstance) const
		{
			return mInstance != entityInstance;
		}

		bool operator==(const RESOURCE_TYPE* Entity) const
		{
			return mResource == Entity;
		}

		bool operator!=(const RESOURCE_TYPE* Entity) const
		{
			return mResource != Entity;
		}

		bool operator<(const InstancePtrType& other) const
		{
			return mInstance < other.mInstance;
		}

		bool operator>(const InstancePtrType& other) const
		{
			return mInstance > other.mInstance;
		}

		bool operator<=(const InstancePtrType& other) const
		{
			return mInstance <= other.mInstance;
		}

		bool operator>=(const InstancePtrType& other) const
		{
			return mInstance >= other.mInstance;
		}

		RESOURCE_TYPE* getResource()
		{
			return mResource.get();
		}

		RESOURCE_TYPE* getResource() const
		{
			return mResource.get();
		}

		INSTANCE_TYPE* get() const
		{
			return mInstance;
		}

		INSTANCE_TYPE* get()
		{
			return mInstance;
		}

	private:
		
		void Assign(const InstancePtrType& other)
		{
			mResource = other.mResource;
			mInstance = other.mInstance;
		}

	private:
		friend class ResourceManagerService;
		ObjectPtr<RESOURCE_TYPE> mResource;
		INSTANCE_TYPE* mInstance = nullptr;
	};

	using EntityPtr = InstancePtr<Entity, EntityInstance>;
	using ComponentPtr = InstancePtr<Component, ComponentInstance>;
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
			return obj.getResource();
		}

		inline static type create(const wrapped_type& value)
		{
			return nap::EntityPtr(value);
		}		
	};

	template<>
	struct wrapper_mapper<nap::ComponentPtr>
	{
		using wrapped_type = nap::Component*;
		using type = nap::ComponentPtr;

		inline static wrapped_type get(const type& obj)
		{
			return obj.getResource();
		}

		inline static type create(const wrapped_type& value)
		{
			return nap::ComponentPtr(value);
		}
	};
}

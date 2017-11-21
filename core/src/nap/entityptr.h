#pragma once

namespace nap
{
	class Entity;
	class EntityInstance;

	/**
	* EntityPtr is used to access EntityInstance object. Because EntityInstances are spawned at runtime, the file
	* objects have no knowledge of instances. To make sure that we can point to EntityInstances, EntityPtr
	* wraps both a pointer to an Entity and to an EntityInstance. From an RTTI perspective, EntityPtr acts
	* like a pointer to another Entity, while at runtime, the pointer acts like a pointer to an EntityInstance.
	*/
	class EntityPtr
	{
	public:
		EntityPtr() = default;

		// Regular ptr Ctor
		EntityPtr(Entity* ptr) :
			mResource(ptr)
		{
		}

		// Copy ctor
		EntityPtr(const EntityPtr& other)
		{
			Assign(other);
		}

		// Move ctor
		EntityPtr(EntityPtr&& other)
		{
			Assign(other);
			other.mResource = nullptr;
			other.mInstance = nullptr;
		}

		// Assignment operator
		EntityPtr& operator=(const EntityPtr& other)
		{
			Assign(other);
			return *this;
		}

		// Move assignment operator
		EntityPtr& operator=(EntityPtr&& other)
		{
			Assign(other);
			other.mResource = nullptr;
			other.mInstance = nullptr;
			return *this;
		}

		//////////////////////////////////////////////////////////////////////////

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

		bool operator==(const EntityPtr& other) const
		{
			return mResource == other.mResource && mInstance == other.mInstance;
		}

		bool operator==(const EntityInstance* entityInstance) const
		{
			return mInstance == entityInstance;
		}

		bool operator!=(const EntityInstance* entityInstance) const
		{
			return mInstance != entityInstance;
		}

		bool operator==(const Entity* Entity) const
		{
			return mResource == Entity;
		}

		bool operator!=(const Entity* Entity) const
		{
			return mResource != Entity;
		}

		bool operator<(const EntityPtr& other) const
		{
			return mInstance < other.mInstance;
		}

		bool operator>(const EntityPtr& other) const
		{
			return mInstance > other.mInstance;
		}

		bool operator<=(const EntityPtr& other) const
		{
			return mInstance <= other.mInstance;
		}

		bool operator>=(const EntityPtr& other) const
		{
			return mInstance >= other.mInstance;
		}

		Entity* getResource()
		{
			return mResource.get();
		}

		Entity* getResource() const
		{
			return mResource.get();
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

		void Assign(const EntityPtr& other)
		{
			mResource = other.mResource;
			mInstance = other.mInstance;
		}

	private:
		friend class Scene;
		ObjectPtr<Entity> mResource;
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
			return obj.getResource();
		}

		inline static type create(const wrapped_type& value)
		{
			return nap::EntityPtr(value);
		}
	};
}

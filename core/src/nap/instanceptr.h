#pragma once

#include "objectptr.h"

namespace nap
{
	/**
	 * InstancePtr is used to access instances in the Entity system through their resource. Because instances are spawned at runtime, the file
	 * objects (resources) have no knowledge of instances. To make sure that we can point to instances, InstancePtr
	 * wraps both a pointer to a Resource and to an Instance. From an RTTI perspective, InstancePtr acts
	 * like a pointer to another Resource, while at runtime, the pointer acts like a pointer to the corresponding Instance.
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
}

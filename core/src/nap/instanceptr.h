#pragma once

#include "objectptr.h"

namespace nap
{
	/**
	* InstancePtr is used to access instances in the Entity system through their resource. Because instances are spawned at runtime, the file
	* objects (resources) have no knowledge of instances. To make sure that we can point to instances, InstancePtr
	* wraps both a pointer to a Resource and to an Instance. From an RTTI perspective, InstancePtr acts
	* like a pointer to another Resource, while at runtime, the pointer acts like a pointer to the corresponding Instance. The instance pointer
	* is filled in by the ResourceManager.
	*
	* InstancePtr is the untyped base that can be used generically to resolve pointers, derived classes are convenience classes
	* to support typed versions the resource and instance pointers. 
	*/
	class NAPAPI InstancePtrBase
	{
		RTTI_ENABLE()

	public:
		/**
		* @return RTTIObject pointer.
		*/
		ObjectPtr<rtti::RTTIObject>& getResource()
		{
			return mResource;
		}

		/**
		* @return RTTIObject pointer.
		*/
		const ObjectPtr<rtti::RTTIObject>& getResource() const
		{
			return mResource;
		}
		
	protected:
		InstancePtrBase();

		/**
		* ctor taking direct pointer.
		*/
		InstancePtrBase(rtti::RTTIObject* resource) :
			mResource(resource),
			mInstance(nullptr)
		{
		}
	
		/**
		* @return RTTIObject pointer.
		*/
		rtti::RTTIObject* get()
		{
			return mInstance;
		}

		/**
		* @return RTTIObject pointer.
		*/
		const rtti::RTTIObject* get() const
		{
			return mInstance;
		}

	private:
		template<typename RESOURCE_TYPE, typename INSTANCE_TYPE, typename BASE> friend class InstancePtr;
		friend class ResourceManagerService;

		ObjectPtr<rtti::RTTIObject> mResource;
		rtti::RTTIObject*			mInstance;
	};

	/**
	 * Strongly typed version of InstancePtrBase.
	 * As this is a template class we cannot provide RTTI in a generic way. To make sure we can still filter out for a specific kind of pointer, 
	 * another base can be used as a 'mixin', also known as the Curiously Returning Template Pattern (CRTP). The base can then be used to check for 
	 * the kind of pointer type.
	 */
	template<typename RESOURCE_TYPE, typename INSTANCE_TYPE, typename BASE>
	class InstancePtr : public BASE
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		// Note: these functions are normally defined by RTTI_ENABLE. However, because template arguments cannot be passed to macros (we derive from a template parameter), 
		// we cannot use that macro here. To still be able to do is_derived_from checks on InstancePtrs, we've manually expanded the macro here.
		virtual RTTR_INLINE rttr::type get_type() const { return rttr::detail::get_type_from_instance(this); }
		virtual RTTR_INLINE void* get_ptr() { return reinterpret_cast<void*>(this); }
		virtual RTTR_INLINE rttr::detail::derived_info get_derived_info() { return{ reinterpret_cast<void*>(this), rttr::detail::get_type_from_instance(this) }; }
		using base_class_list = rttr::detail::type_list<BASE>;
		//////////////////////////////////////////////////////////////////////////

		using InstancePtrType = InstancePtr<RESOURCE_TYPE, INSTANCE_TYPE, BASE>;

		InstancePtr() = default;

		// Regular ptr Ctor
		InstancePtr(RESOURCE_TYPE* resource) :
			BASE(resource)
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
			assert(InstancePtrBase::mInstance != nullptr);
			return *static_cast<const INSTANCE_TYPE*>(InstancePtrBase::mInstance);
		}

		INSTANCE_TYPE& operator*()
		{
			assert(InstancePtrBase::mInstance != nullptr);
			return *static_cast<INSTANCE_TYPE*>(InstancePtrBase::mInstance);
		}

		INSTANCE_TYPE* operator->() const
		{
			assert(InstancePtrBase::mInstance != nullptr);
			return static_cast<INSTANCE_TYPE*>(InstancePtrBase::mInstance);
		}

		INSTANCE_TYPE* operator->()
		{
			assert(InstancePtrBase::mInstance != nullptr);
			return static_cast<INSTANCE_TYPE*>(InstancePtrBase::mInstance);
		}

		bool operator==(const InstancePtrType& other) const
		{
			return InstancePtrBase::mResource == other.mResource && InstancePtrBase::mInstance == other.mInstance;
		}

		bool operator==(const INSTANCE_TYPE* entityInstance) const
		{
			return InstancePtrBase::mInstance == entityInstance;
		}

		bool operator!=(const INSTANCE_TYPE* entityInstance) const
		{
			return InstancePtrBase::mInstance != entityInstance;
		}

		bool operator==(const RESOURCE_TYPE* entity) const
		{
			return InstancePtrBase::mResource == entity;
		}

		bool operator!=(const RESOURCE_TYPE* entity) const
		{
			return InstancePtrBase::mResource != entity;
		}

		bool operator<(const InstancePtrType& other) const
		{
			return InstancePtrBase::mInstance < other.mInstance;
		}

		bool operator>(const InstancePtrType& other) const
		{
			return InstancePtrBase::mInstance > other.mInstance;
		}

		bool operator<=(const InstancePtrType& other) const
		{
			return InstancePtrBase::mInstance <= other.mInstance;
		}

		bool operator>=(const InstancePtrType& other) const
		{
			return InstancePtrBase::mInstance >= other.mInstance;
		}

		ObjectPtr<RESOURCE_TYPE> getResource()
		{
			return ObjectPtr<RESOURCE_TYPE>(static_cast<RESOURCE_TYPE*>(mResource.get()));
		}

		const ObjectPtr<RESOURCE_TYPE> getResource() const
		{
			return ObjectPtr<RESOURCE_TYPE>(static_cast<RESOURCE_TYPE*>(mResource.get()));
		}

		INSTANCE_TYPE* get() const
		{
			return static_cast<INSTANCE_TYPE*>(mInstance);
		}

		INSTANCE_TYPE* get()
		{
			return static_cast<INSTANCE_TYPE*>(mInstance);
		}

	private:		
		void Assign(const InstancePtrType& other) 
		{
			InstancePtrBase::mResource = other.mResource;
			InstancePtrBase::mInstance = other.mInstance;
		}
	};
}

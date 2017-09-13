#pragma once

#include "instanceptr.h"

namespace nap
{
	class Component;
	class ComponentInstance;

	/**
	 * Placeholder class with RTTI that enables us to check the type of the pointer. Also see comments in InstancePtr.
	 */
	class NAPAPI ComponentPtrBase : public InstancePtrBase
	{
		RTTI_ENABLE(InstancePtrBase)
	public:
		ComponentPtrBase() = default;

	protected:
		ComponentPtrBase(Component* resource);
	};

	/**
	 * Provides strongly typed interface for components. This class is derived from InstancePtr for two reasons:
	 * 1) To mixin ComponentPtrBase as a generic base that we can filter on.
	 * 2) To avoid the need for users to supply the InstanceType manually. The instance type is retrieved from the resource through
	 *      the InstanceType typedef. This typedef is automatically generated when declaring a component using DECLARE_COMPONENT.
	 *
	 * As a summary, this is the entire hierarchy for ComponentPtr the class roles behind it:
	 *
	 *		InstancePtrBase											(generic ontyped instance/resource ptr)
	 *		   	  ^
	 *		      |
	 *		ComponentPtrBase										(used for filtering component ptrs using rtti)
	 *			  ^
	 *		      |
	 *		InstancePtr<Resource, Instance, ComponentPtrBase>		(Typed resource/instance interface)
	 *			  ^
	 *		      |
	 *		ComponentPtr<Resource>									(Convenience to avoid instance type in ComponentPtr declarations)
	 */
	template<typename RESOURCE_TYPE>
	class ComponentPtr : public InstancePtr<RESOURCE_TYPE, typename RESOURCE_TYPE::InstanceType, ComponentPtrBase>
	{
		using InstancePtrBase = InstancePtr<RESOURCE_TYPE, typename RESOURCE_TYPE::InstanceType, ComponentPtrBase>;
		RTTI_ENABLE(InstancePtrBase)

	public:
		ComponentPtr() = default;

		// Regular ptr Ctor
		ComponentPtr(RESOURCE_TYPE* resource) :
			InstancePtrBase(resource)
		{
		}
	};
}


/**
 * The following construct is required to support ComponentPtr in RTTR as a regular pointer.
 */
namespace rttr
{
	template<typename T>
	struct wrapper_mapper<nap::ComponentPtr<T>>
	{
		using wrapped_type = T*;
		using type = nap::ComponentPtr<T>;

		inline static wrapped_type get(const type& obj)
		{
			return obj.getResource().get();
		}

		inline static type create(const wrapped_type& value)
		{
			return type(value);
		}
	};
}
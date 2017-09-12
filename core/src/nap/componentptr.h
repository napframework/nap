#pragma once

#include "instanceptr.h"

namespace nap
{
	class Component;
	class ComponentInstance;

	class ComponentPtrBase : public InstancePtrBase
	{
		RTTI_ENABLE(InstancePtrBase)
	public:
		ComponentPtrBase() = default;

	protected:
		ComponentPtrBase(Component* resource);
	};

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
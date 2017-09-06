#pragma once

#include "instanceptr.h"

namespace nap
{
	class Component;
	class ComponentInstance;

	using ComponentPtr = InstancePtr<Component, ComponentInstance>;
}

/**
 * The following construct is required to support ComponentPtr in RTTR as a regular pointer.
 */
namespace rttr
{
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

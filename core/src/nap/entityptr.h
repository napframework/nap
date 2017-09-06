#pragma once

#include "instanceptr.h"

namespace nap
{
	class Entity;
	class EntityInstance;

	using EntityPtr = InstancePtr<Entity, EntityInstance>;
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

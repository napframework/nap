#pragma once

// External includes
#include <nap/serviceablecomponent.h>
#include <nap/attribute.h>

namespace nap
{
	/**
	 * Instance of a shader that lives in your object tree
	 * Holds a pointer to the actual shader resource program
	 */
	class MaterialComponent : public ServiceableComponent
	{
		RTTI_ENABLE_DERIVED_FROM(ServiceableComponent)
	public:
		// Default constructor
		MaterialComponent() = default;
	};
}

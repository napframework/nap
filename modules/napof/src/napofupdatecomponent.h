#pragma once

// RTTI Includes
#include <rtti/rtti.h>

// Nap Includes
#include <nap/coremodule.h>
#include <nap/serviceablecomponent.h>
#include <nap/attribute.h>

namespace nap
{
	/**
	@brief Updatable NAP component
	**/
	class OFUpdatableComponent : public ServiceableComponent
	{
		RTTI_ENABLE_DERIVED_FROM(ServiceableComponent)
	public:
		// Default Constructor
		OFUpdatableComponent()		{ }

		// Update Attributes
		Attribute<bool>		mEnableUpdates	{ this, "Update", true };

		// Update Call
		void				update();
		virtual void		onUpdate() = 0;
	};
}

RTTI_DECLARE_BASE(nap::OFUpdatableComponent)

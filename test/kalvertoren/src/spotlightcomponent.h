#pragma once

#include <nap/component.h>

namespace nap
{
	class SpotlightComponentInstance;

	/**
	 *	spotlightcomponent
	 */
	class SpotlightComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(SpotlightComponent, SpotlightComponentInstance)
	public:
	};


	/**
	 * spotlightcomponentInstance	
	 */
	class SpotlightComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		SpotlightComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }
	};
}

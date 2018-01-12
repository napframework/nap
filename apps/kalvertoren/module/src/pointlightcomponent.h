#pragma once

#include <component.h>

namespace nap
{
	class PointlightComponentInstance;

	/**
	 *	spotlightcomponent
	 */
	class NAPAPI PointlightComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(PointlightComponent, PointlightComponentInstance)
	public:
		float mIntensity = 1.0f;		// property: intensity of the point light
		float mAttenuation = 0.0001f;	// property: attenuation of the light, higher values make the light intensity drop faster over distance
	};


	/**
	 * spotlightcomponentInstance	
	 */
	class NAPAPI PointlightComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		PointlightComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)										{ }

		/**
		 *	Initialize this component based on the resource
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		float mIntensity = 1.0f;
		float mAttenuation = 0.0001f;
	};
}

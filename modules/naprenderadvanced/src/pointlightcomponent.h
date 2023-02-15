#pragma once

// Local includes
#include "lightcomponent.h"

// External includes
#include <perspcameracomponent.h>

namespace nap
{
	// Forward declares
	class PointLightComponentInstance;

	/**
	 *	PointLightComponent
	 */
	class NAPAPI PointLightComponent : public LightComponent
	{
		RTTI_ENABLE(LightComponent)
		DECLARE_COMPONENT(PointLightComponent, PointLightComponentInstance)
	public:
		ResourcePtr<ParameterFloat> mAttenuation;				///< Property: 'Attenuation'
		ComponentPtr<PerspCameraComponent> mShadowCamera;		///< Property: 'ShadowCamera' Camera that produces the depth texture for a directional light
	};


	/**
	 * PointLightComponentInstance
	 */
	class NAPAPI PointLightComponentInstance : public LightComponentInstance
	{
		RTTI_ENABLE(LightComponentInstance)
	public:
		PointLightComponentInstance(EntityInstance& entity, Component& resource) :
			LightComponentInstance(entity, resource) { }

		/**
		 * Initialize LightComponentInstance based on the LightComponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the LightComponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * @return the shadow camera if available, else nullptr
		 */
		virtual CameraComponentInstance* getShadowCamera()					{ return (mShadowCamera != nullptr) ? &(*mShadowCamera) : nullptr; }

		/**
		 * @return the light equation
		 */
		virtual ELightEquation getLightEquation() const						{ return ELightEquation::Point; }

		/**
		 * @return the light type
		 */
		virtual ELightType getLightType() const								{ return ELightType::Point; }

	private:
		// Shadow camera
		ComponentInstancePtr<PerspCameraComponent> mShadowCamera = { this, &PointLightComponent::mShadowCamera };
	};
}

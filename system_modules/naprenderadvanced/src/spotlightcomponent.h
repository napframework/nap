#pragma once

// Local includes
#include "lightcomponent.h"

// External includes
#include <perspcameracomponent.h>

namespace nap
{
	// Forward declares
	class SpotLightComponentInstance;

	/**
	 *	SpotLightComponent
	 */
	class NAPAPI SpotLightComponent : public LightComponent
	{
		RTTI_ENABLE(LightComponent)
		DECLARE_COMPONENT(SpotLightComponent, SpotLightComponentInstance)
	public:
		ResourcePtr<ParameterEntryFloat> mAttenuation;			///< Property: 'Attenuation'
		ResourcePtr<ParameterEntryFloat> mAngle;				///< Property: 'Angle'
		ResourcePtr<ParameterEntryFloat> mFallOff;				///< Property: 'FallOff'
		ComponentPtr<PerspCameraComponent> mShadowCamera;		///< Property: 'ShadowCamera' Camera that produces the depth texture for a directional light
		uint mShadowMapSize = 1024U;							///< Property: 'ShadowMapSize'
	};


	/**
	 * SpotLightComponentInstance
	 */
	class NAPAPI SpotLightComponentInstance : public LightComponentInstance
	{
		RTTI_ENABLE(LightComponentInstance)
	public:
		SpotLightComponentInstance(EntityInstance& entity, Component& resource) :
			LightComponentInstance(entity, resource) { }

		// Destructor
		virtual ~SpotLightComponentInstance() override { LightComponentInstance::removeLightComponent(); }

		/**
		 * Initialize LightComponentInstance based on the LightComponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the LightComponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * @return whether this light component supports shadows
		 */
		virtual bool isShadowSupported() const override						{ return true; }

		/**
		 * @return the shadow camera if available, else nullptr
		 */
		virtual CameraComponentInstance* getShadowCamera()					{ return (mShadowCamera != nullptr) ? &(*mShadowCamera) : nullptr; }

		/**
		 * @return the light type
		 */
		virtual ELightType getLightType() const	override					{ return ELightType::Spot; }

		/**
		 * @return the shadow map type
		 */
		virtual EShadowMapType getShadowMapType() const override			{ return EShadowMapType::Quad; }

	private:
		// Shadow camera
		ComponentInstancePtr<PerspCameraComponent> mShadowCamera = { this, &SpotLightComponent::mShadowCamera };
	};
}

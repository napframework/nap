#pragma once

// Local includes
#include "lightcomponent.h"

namespace nap
{
	// Forward declares
	class DirectionalLightComponentInstance;

	/**
	 *	DirectionalLightComponent
	 */
	class NAPAPI DirectionalLightComponent : public LightComponent
	{
		RTTI_ENABLE(LightComponent)
		DECLARE_COMPONENT(DirectionalLightComponent, DirectionalLightComponentInstance)
	public:
		ComponentPtr<OrthoCameraComponent> mShadowCamera;		///< Property: 'ShadowCamera' Camera that produces the depth texture for a directional light
	};


	/**
	 * DirectionalLightComponentInstance
	 */
	class NAPAPI DirectionalLightComponentInstance : public LightComponentInstance
	{
		RTTI_ENABLE(LightComponentInstance)
	public:
		DirectionalLightComponentInstance(EntityInstance& entity, Component& resource) :
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

		RGBColorFloat mColor = { 1.0f, 1.0f, 1.0f };
		float mIntensity = 1.0f;

	private:
		// Shadow camera
		ComponentInstancePtr<OrthoCameraComponent> mShadowCamera = { this, &DirectionalLightComponent::mShadowCamera };
	};
}

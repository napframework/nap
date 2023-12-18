#pragma once

// Local includes
#include "lightcomponent.h"

// External includes
#include <orthocameracomponent.h>

namespace nap
{
	// Forward declares
	class DirectionalLightComponentInstance;

	/**
	 * Directional light component for NAP RenderAdvanced's light system.
	 *
	 * Unidirectional light that emits from its origin to a specified direction. The Render Advanced service creates and
	 * manages a `nap::DepthRenderTarget` and `nap::DepthRenderTexture2D` for rendering this light's shadow maps.
	 */
	class NAPAPI DirectionalLightComponent : public LightComponent
	{
		RTTI_ENABLE(LightComponent)
		DECLARE_COMPONENT(DirectionalLightComponent, DirectionalLightComponentInstance)
	public:
		ComponentPtr<OrthoCameraComponent> mShadowCamera;		///< Property: 'ShadowCamera' Camera that produces the depth texture for a directional light
		uint mShadowMapSize = 1024U;							///< Property: 'ShadowMapSize'
	};


	/**
	 * Directional light component instance for NAP RenderAdvanced's light system.
	 *
	 * Unidirectional light that emits from its origin to a specified direction. The Render Advanced service creates and
	 * manages a `nap::DepthRenderTarget` and `nap::DepthRenderTexture2D` for rendering this light's shadow maps.
	 */
	class NAPAPI DirectionalLightComponentInstance : public LightComponentInstance
	{
		RTTI_ENABLE(LightComponentInstance)
	public:
		DirectionalLightComponentInstance(EntityInstance& entity, Component& resource) :
			LightComponentInstance(entity, resource) { }

		// Destructor
		virtual ~DirectionalLightComponentInstance() override { LightComponentInstance::removeLightComponent(); }

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
		virtual CameraComponentInstance* getShadowCamera() override			{ return mShadowCamera.get(); }

		/**
		 * @return the light type
		 */
		virtual ELightType getLightType() const	override					{ return ELightType::Directional; }

		/**
		 * @return the shadow map type
		 */
		virtual EShadowMapType getShadowMapType() const override			{ return EShadowMapType::Quad; }

	private:
		// Shadow camera
		ComponentInstancePtr<OrthoCameraComponent> mShadowCamera = { this, &DirectionalLightComponent::mShadowCamera };
	};
}

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
	 * Spot light component for NAP RenderAdvanced's light system.
	 *
	 * Omnidirectional light that emits from its origin to a specified direction with an angle of view (i.e. cone light).
	 * The shadow map for this light is a 2D depth texture; therefore, the reach of the light can exceed the extent beyond
	 * that of the depth map. The Render Advanced service creates and manages a `nap::DepthRenderTarget` and
	 * `nap::DepthRenderTexture2D` for rendering this light's shadow maps.
	 */
	class NAPAPI SpotLightComponent : public LightComponent
	{
		RTTI_ENABLE(LightComponent)
		DECLARE_COMPONENT(SpotLightComponent, SpotLightComponentInstance)
	public:
		float mAttenuation = 0.1f;								///< Property: 'Attenuation' The rate at which light intensity is lost over distance from the origin
		float mAngle = 90.0f;									///< Property: 'Angle' The light's angle of view (focus)
		float mFallOff = 0.5f;									///< Property: 'FallOff' The falloff, where 0.0 cuts off at the edge, and 1.0 results in a linear gradient.
		ComponentPtr<PerspCameraComponent> mShadowCamera;		///< Property: 'ShadowCamera' Camera that produces the depth texture for a directional light
		uint mShadowMapSize = 1024;								///< Property: 'ShadowMapSize' The horizontal and vertical dimension of the shadow map for this light
	};


	/**
	 * Spot light component instance for NAP RenderAdvanced's light system.
	 *
	 * Omnidirectional light that emits from its origin to a specified direction with an angle of view (i.e. cone light).
	 * The shadow map for this light is a 2D depth texture; therefore, the reach of the light can exceed the extent beyond
	 * that of the depth map. The Render Advanced service creates and manages a `nap::DepthRenderTarget` and
	 * `nap::DepthRenderTexture2D` for rendering this light's shadow maps.
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

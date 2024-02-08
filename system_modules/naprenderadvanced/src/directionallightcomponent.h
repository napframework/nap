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
		ComponentPtr<OrthoCameraComponent> mShadowCamera;					///< Property: 'ShadowCamera' Camera that produces the depth texture for a directional light
		float mProjectionSize  = 20.0f;										///< Property: 'ProjectionSize' The shadow camera scene projection size
		glm::vec2 mClippingPlanes = { 1.0f, 1000.0f };						///< Property: 'ClippingPlanes' The near and far shadow clipping distance of this light
		uint mShadowMapSize = 1024;											///< Property: 'ShadowMapSize' The horizontal and vertical dimension of the shadow map for this light
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

		/**
		 * Initialize LightComponentInstance based on the LightComponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the LightComponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * @return the light type
		 */
		virtual ELightType getLightType() const	override					{ return ELightType::Directional; }

		/**
		 * @return the shadow map type
		 */
		virtual EShadowMapType getShadowMapType() const override			{ return EShadowMapType::Quad; }

	private:

		// Shadow camera entity resource
		std::unique_ptr<Entity> mShadowCamEntity = nullptr;
		std::unique_ptr<OrthoCameraComponent> mShadowCamComponent = nullptr;
		std::unique_ptr<TransformComponent> mShadowCamXformComponent = nullptr;
	};
}

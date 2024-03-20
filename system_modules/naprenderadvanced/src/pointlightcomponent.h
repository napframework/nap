#pragma once

// Local includes
#include "lightcomponent.h"

// External includes
#include <perspcameracomponent.h>
#include <rendergnomoncomponent.h>

namespace nap
{
	// Forward declares
	class PointLightComponentInstance;

	/**
	 * Point light component for NAP RenderAdvanced's light system.
	 * 
	 * Omnidirectional light that emits from its origin. Therefore, ignores the `direction` uniform. The Render Advanced
	 * service creates and manages a `nap::CubeDepthRenderTarget` and `nap::DepthRenderTextureCube` for rendering this
	 * light's shadow maps.
	 */
	class NAPAPI PointLightComponent : public LightComponent
	{
		RTTI_ENABLE(LightComponent)
		DECLARE_COMPONENT(PointLightComponent, PointLightComponentInstance)
	public:
		float mAttenuation = 0.1f;								///< Property: 'Attenuation' The rate at which light intensity is lost over distance from the origin
		glm::vec2 mClippingPlanes = { 1.0f, 1000.0f };			///< Property: 'ClippingPlanes' The near and far shadow clipping distance of this light
		uint mShadowMapSize = 512;								///< Property: 'ShadowMapSize' The horizontal and vertical dimension of the shadow map for this light
	};


	/**
	 * Point light component instance for NAP RenderAdvanced's light system.
	 *
	 * Omnidirectional light that emits from its origin. Therefore, ignores the `direction` uniform. The Render Advanced
	 * service creates and manages a `nap::CubeDepthRenderTarget` and `nap::DepthRenderTextureCube` for rendering this
	 * light's shadow maps.
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
		 * @return the light type
		 */
		virtual ELightType getLightType() const	override					{ return ELightType::Point; }

		/**
		 * @return the shadow map type
		 */
		virtual EShadowMapType getShadowMapType() const override			{ return EShadowMapType::Cube; }

		/**
		 * The rate at which light intensity is lost over distance from the origin
		 * @return light attenuation
		 */
		float getAttenuation() const										{ return mAttenuation; }

		/**
		 * Set the rate at which light intensity is lost over distance from the origin
		 * @param attenuation light attenuation
		 */
		void setAttenuation(float attenuation)								{ mAttenuation = attenuation; }

		float mAttenuation = 0.1f;

	private:
		// Shadow camera entity resource
		std::unique_ptr<Entity> mShadowCamEntity = nullptr;
		std::unique_ptr<PerspCameraComponent> mShadowCamComponent = nullptr;
		std::unique_ptr<TransformComponent> mShadowCamXformComponent = nullptr;
		std::unique_ptr<RenderGnomonComponent> mShadowOriginComponent = nullptr;
		std::unique_ptr<GnomonMesh> mGnomonMesh = nullptr;
	};
}

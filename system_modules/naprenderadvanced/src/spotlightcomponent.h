/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local includes
#include "lightcomponent.h"

// External includes
#include <perspcameracomponent.h>
#include <renderfrustumcomponent.h>
#include <rendergnomoncomponent.h>

namespace nap
{
	// Forward declares
	class SpotLightComponentInstance;
	class SceneService;

	/**
	 * Spot light component for NAP RenderAdvanced's light system.
	 *
	 * Source that emits light from its origin to a specified direction with an angle of view (i.e. cone light).
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
		float mFOVClip = 1.0f;									///< Property: 'FOVClip' The light shadow camera view clip value, where 1.0 is equals the light angle of view.
		float mFalloff = 0.5f;									///< Property: 'Falloff' The falloff, where 0.0 cuts off at the edge, and 1.0 results in a linear gradient.
		glm::vec2 mClippingPlanes = { 1.0f, 1000.0f };			///< Property: 'ClippingPlanes' The near and far shadow clipping distance of this light
		uint mShadowMapSize = 1024;								///< Property: 'ShadowMapSize' The horizontal and vertical dimension of the shadow map for this light
	};


	/**
	 * Spot light component instance for NAP RenderAdvanced's light system.
	 *
	 * Source that emits light from its origin to a specified direction with an angle of view (i.e. cone light).
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
		virtual ELightType getLightType() const	override					{ return ELightType::Spot; }

		/**
		 * @return the shadow map type
		 */
		virtual EShadowMapType getShadowMapType() const override			{ return EShadowMapType::Quad; }

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

		/**
		 * Light angle of view in degrees
		 * @return lamp angle of view in degrees
		 */	
		float getAngle() const												{ return mAngle; }

		/**
		 * Set light and camera angle of view in degrees
		 * @param angle angle of view in degrees
		 */
		void setAngle(float angle);

		/**
		 * Light shadow camera view clip value, where 1.0 is equals the light angle of view.
		 * @return shadow camera view clip value
		 */
		float getFOVClip() const											{ return mFOVClip; }

		/**
		 * Set light shadow camera view clip value, where 1.0 is equals the light angle of view.
		 * @param clip shadow camera view clip value in the range 0.0 - 1.0
		 */
		void setFOVClip(float clip);

		/**
		 * Light falloff. A value of 0.0 results in a hard edge, a value of 1.0 results in a linear gradient. 
		 * @return lamp falloff
		 */	
		float getFalloff() const											{ return mFalloff; }

		/**
		 * Set the spotlight falloff
		 * @param angle spotlight angle
		 */
		void setFalloff(float falloff)										{ mFalloff = falloff; }

		float mAttenuation = 0.1f;
		float mAngle = 90.0f;
		float mFOVClip = 1.0f;
		float mFalloff = 0.5f;

	private:
		// Shadow camera entity resource
		std::unique_ptr<Entity> mShadowCamEntity = nullptr;
		std::unique_ptr<PerspCameraComponent> mShadowCamComponent = nullptr;
		std::unique_ptr<RenderFrustumComponent> mShadowFrustrumComponent = nullptr;
		std::unique_ptr<RenderGnomonComponent> mShadowOriginComponent = nullptr;
		std::unique_ptr<TransformComponent> mShadowCamXformComponent = nullptr;
		std::unique_ptr<GnomonMesh> mGnomonMesh = nullptr;
	};
}

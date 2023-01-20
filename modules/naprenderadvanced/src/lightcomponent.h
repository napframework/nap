#pragma once

// External includes
#include <component.h>
#include <componentptr.h>
#include <parameternumeric.h>
#include <parametervec.h>
#include <parametercolor.h>
#include <cameracomponent.h>
#include <perspcameracomponent.h>
#include <orthocameracomponent.h>
#include <transformcomponent.h>
#include <renderablemeshcomponent.h>
#include <depthrendertarget.h>

// Local includes
#include "light.h"

namespace nap
{
	class LightComponentInstance;
	class RenderableMeshComponentInstance;
	class RenderService;

	/**
	 *	LightComponent
	 */
	class NAPAPI LightComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(LightComponent, LightComponentInstance)
	public:

		/**
		* Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		* @param components the components this object depends on
		*/
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		ResourcePtr<Light>						mLight;										///< Property: "Light" The light resource
		std::vector<ComponentPtr<RenderableMeshComponent>> mRenderComponents;				///< Property: "RenderComponents"

		ComponentPtr<PerspCameraComponent>		mEyeCamera;									///< Property: "EyeCamera" Camera that represents the eye/view frustrum
		ComponentPtr<PerspCameraComponent>		mShadowCameraPerspective;					///< Property: "ShadowCameraPerspective" Camera that produces the depth texture for a point light
		ComponentPtr<OrthoCameraComponent>		mShadowCameraOrthographic;					///< Property: "ShadowCameraOrthographic" Camera that produces the depth texture for a directional light
		ECameraType								mCameraType = ECameraType::Perspective;		///< Property: "CameraType" The camera type to use

		ResourcePtr<DepthRenderTarget>			mDepthRenderTarget;
	};


	/**
	 * LightComponentInstance	
	 */
	class NAPAPI LightComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		LightComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		/**
		 * Initialize LightComponentInstance based on the LightComponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the LightComponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * update LightComponentInstance. This is called by NAP core automatically
		 * @param deltaTime time in between frames in seconds
		 */
		virtual void update(double deltaTime) override;

		/**
		 * 
		 */
		bool isCameraEnabled() const											{ return mCameraEnabled; }

		/**
		 *
		 */
		bool isShadowEnabled() const											{ return mShadowEnabled; }

		/**
		 *
		 */
		CameraComponentInstance* getShadowCamera() const;

		/**
		 * Returns the depth render target for the shadow map. Asserts when shadow is not enabled.
		 * @return the shadow depth render target.
		 */
		DepthRenderTarget& getShadowTarget() const;

		/**
		 *
		 */
		const std::vector<RenderableComponentInstance*>& getRenderableComponents() const { return mResolvedRenderComponents; }

		/**
		 * 
		 */
		bool beginShadowPass();

		/**
		 * 
		 */
		void endShadowPass();

	private:
		LightComponent* mResource = nullptr;
		RenderService* mRenderService = nullptr;
		TransformComponentInstance* mTransform = nullptr;

		glm::mat4 mLightView;
		glm::mat4 mLightViewProjection;

		bool mCameraEnabled = false;
		bool mShadowEnabled = false;

		ComponentInstancePtr<PerspCameraComponent>	mEyeCameraComponent = { this, &LightComponent::mEyeCamera };
		ComponentInstancePtr<PerspCameraComponent>	mShadowCameraPerspective = { this, &LightComponent::mShadowCameraPerspective };
		ComponentInstancePtr<OrthoCameraComponent>	mShadowCameraOrthographic = { this, &LightComponent::mShadowCameraOrthographic };

		std::vector<ComponentInstancePtr<RenderableMeshComponent>> mRenderComponents = initComponentInstancePtr(this, &LightComponent::mRenderComponents);
		std::vector<RenderableComponentInstance*> mResolvedRenderComponents;
	};
}

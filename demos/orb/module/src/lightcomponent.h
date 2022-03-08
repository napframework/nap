#pragma once

#include <component.h>
#include <componentptr.h>
#include <parameternumeric.h>
#include <parametervec.h>
#include <parametercolor.h>
#include <cameracomponent.h>
#include <perspcameracomponent.h>
#include <orthocameracomponent.h>
#include <transformcomponent.h>

namespace nap
{
	class LightComponentInstance;
	class RenderableMeshComponentInstance;

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

		ResourcePtr<ParameterRGBColorFloat>		mLightColorParam;							///< Property: "LightColor"
		ResourcePtr<ParameterVec3>				mLightPositionParam;						///< Property: "LightPosition"
		ResourcePtr<ParameterVec3>				mLightDirectionParam;						///< Property: "LightDirection"
		ResourcePtr<ParameterFloat>				mLightIntensityParam;						///< Property: "LightIntensity"

		ComponentPtr<TransformComponent>		mTargetTransform;							///< Property: "TargetTransform" Light target transform

		ComponentPtr<PerspCameraComponent>		mEyeCamera;									///< Property: "EyeCamera" Camera that represents the eye and look
		ComponentPtr<PerspCameraComponent>		mShadowCameraPerspective;					///< Property: "ShadowCameraPerspective" Camera that produces the depth texture for a point light
		ComponentPtr<OrthoCameraComponent>		mShadowCameraOrthographic;					///< Property: "ShadowCameraOrthographic" Camera that produces the depth texture for a directional light
		ECameraType								mCameraType = ECameraType::Perspective;		///< Property: "CameraType" The camera type to use
		bool									mEnableShadow = false;						///< Property: "EnableShadow" Enables shadow rendering
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
		CameraComponentInstance* getShadowCamera() const;

	private:
		LightComponent* mResource = nullptr;
		TransformComponentInstance* mTransform = nullptr;

		std::vector<RenderableMeshComponentInstance*> mCachedRenderComponents;

		glm::mat4 mLightView;
		glm::mat4 mLightViewProjection;

		bool mCameraEnabled = false;

		ComponentInstancePtr<TransformComponent>	mTargetTransformComponent = { this, &LightComponent::mTargetTransform };
		ComponentInstancePtr<PerspCameraComponent>	mEyeCameraComponent = { this, &LightComponent::mEyeCamera };

		ComponentInstancePtr<PerspCameraComponent>	mShadowCameraPerspective = { this, &LightComponent::mShadowCameraPerspective };
		ComponentInstancePtr<OrthoCameraComponent>	mShadowCameraOrthographic = { this, &LightComponent::mShadowCameraOrthographic };
	};
}

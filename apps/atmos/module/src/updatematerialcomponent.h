#pragma once

// Local Includes
#include "selectimagecomponent.h"

#include <component.h>
#include <componentptr.h>
#include <renderablemeshcomponent.h>
#include <transformcomponent.h>
#include <color.h>
#include "parametersimple.h"

namespace nap
{
	class UpdateMaterialComponentInstance;

	/**
	 *	updatematerialcomponent
	 */
	class NAPAPI UpdateMaterialComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(UpdateMaterialComponent, UpdateMaterialComponentInstance)
	public:

		/**
		* Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		* @param components the components this object depends on
		*/
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		ComponentPtr<RenderableMeshComponent> mScanMeshComponent = nullptr;				///< Property: 'ScanMeshComponent'
		ComponentPtr<RenderableMeshComponent> mNormalMeshComponent = nullptr;			///< Property: 'NormalMeshComponent'
		ComponentPtr<SelectImageComponent> mTileableImageSelectComponent = nullptr;		///< Property: "TileableImageSelectComponent'
		ComponentPtr<SelectImageComponent> mSingleImageSelectComponent = nullptr;		///< Property: "SingleImageSelectComponent
		ComponentPtr<TransformComponent> mCameraTransformComponent = nullptr;			///< Property: "CameraTransformComponent"

		ResourcePtr<ParameterFloat>				mPremultValue;
		ResourcePtr<ParameterFloat>				mColorTexMix;
		ResourcePtr<ParameterFloat>				mDiffuseColorMix;
		ResourcePtr<ParameterFloat>				mVideoMaskValue;
		ResourcePtr<ParameterFloat>				mVideoContrastValue;
		ResourcePtr<ParameterFloat>				mVideoTexMix;
		ResourcePtr<ParameterRGBColorFloat>		mDiffuseColor;
		ResourcePtr<ParameterRGBColorFloat>		mMaskColor;
		ResourcePtr<ParameterFloat>				mColorTexScaleOne;
		ResourcePtr<ParameterFloat>				mColorTexScaleTwo;
		ResourcePtr<ParameterVec2>				mTextureSpeed;
		ResourcePtr<ParameterFloat>				mVideoTexScaleOne;
		ResourcePtr<ParameterVec2>				mVideoTextureSpeed;
		ResourcePtr<ParameterVec3>				mLightPos;
		ResourcePtr<ParameterFloat>				mLightIntensity;
		ResourcePtr<ParameterFloat>				mAmbientIntensity;
		ResourcePtr<ParameterFloat>				mDiffuseIntensity;
		ResourcePtr<ParameterRGBColorFloat>		mNormalSpecColor;
		ResourcePtr<ParameterFloat>				mNormalSpecIntens;
		ResourcePtr<ParameterFloat>				mNormalSpecShine;
		ResourcePtr<ParameterFloat>				mNormalScale;
		ResourcePtr<ParameterFloat>				mNormalRandom;
		ResourcePtr<ParameterFloat>				mDiffuseSpecInfl;
		ResourcePtr<ParameterFloat>				mNormalRotValue;
		ResourcePtr<ParameterRGBColorFloat>		mScanSpecColor;
		ResourcePtr<ParameterFloat>				mScanSpecIntens;
		ResourcePtr<ParameterFloat>				mScanSpecShine;
		ResourcePtr<ParameterFloat>				mScanRotValue;
		ResourcePtr<ParameterFloat>				mWindSpeed;
		ResourcePtr<ParameterFloat>				mWindScale;
		ResourcePtr<ParameterFloat>				mWindFreq;
		ResourcePtr<ParameterFloat>				mWindRandom;

		ResourcePtr<ParameterFloat>				mFogMin;
		ResourcePtr<ParameterFloat>				mFogMax;
		ResourcePtr<ParameterFloat>				mFogPower;
		ResourcePtr<ParameterFloat>				mFogInfluence;
	};


	/**
	 * updatematerialcomponentInstance	
	 */
	class NAPAPI UpdateMaterialComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		UpdateMaterialComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource),
			mUpdateMaterialResource(rtti_cast<UpdateMaterialComponent>(&resource))
		{
		}

		/**
		 * Initialize updatematerialcomponentInstance based on the updatematerialcomponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the updatematerialcomponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * update updatematerialcomponentInstance. This is called by NAP core automatically
		 * @param deltaTime time in between frames in seconds
		 */
		virtual void update(double deltaTime) override;

		ComponentInstancePtr<RenderableMeshComponent> mScanMeshComponent =				{ this, &UpdateMaterialComponent::mScanMeshComponent };
		ComponentInstancePtr<RenderableMeshComponent> mNormalsMeshComponent =			{ this, &UpdateMaterialComponent::mNormalMeshComponent };
		ComponentInstancePtr<SelectImageComponent> mTileableImageSelectComponent =		{ this, &UpdateMaterialComponent::mTileableImageSelectComponent };
		ComponentInstancePtr<SelectImageComponent> mSingleImageSelectComponent =		{ this, &UpdateMaterialComponent::mSingleImageSelectComponent };
		ComponentInstancePtr<TransformComponent> mCameraTransform =						{ this, &UpdateMaterialComponent::mCameraTransformComponent };

		glm::vec3		mNormalRotAngle		= { 0.0f, 1.0f, 0.0f };
		glm::vec3		mScanRotAngle		= { 0.0f, 1.0f, 0.0f };

		RGBColorFloat	mFogColor			= { 0.0f, 0.0f, 0.0f };

	private:
		UpdateMaterialComponent* mUpdateMaterialResource;

		double mWindTime = 0.0;
		double mTexTimeU = 0.0;
		double mTexTimeV = 0.0;
		double mVidTimeU = 0.0;
		double mVidTimeV = 0.0;
	};
}

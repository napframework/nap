#pragma once

// Local Includes
#include "selectimagecomponent.h"

#include <component.h>
#include <componentptr.h>
#include <renderablemeshcomponent.h>
#include <transformcomponent.h>
#include <color.h>

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
	};


	/**
	 * updatematerialcomponentInstance	
	 */
	class NAPAPI UpdateMaterialComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		UpdateMaterialComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

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

		// Properties
		float			mColorTexScaleOne	= 10.0f;
		float			mColorTexScaleTwo	= 1.0f;
		float			mColorTexMix		= 0.0f;
		float			mDiffuseColorMix	= 0.0f;
		RGBColorFloat	mDiffuseColor		= { 0.0f, 0.0f, 0.0f };
		float			mPremultValue		= { 0.0f };
		glm::vec2		mTextureSpeed		= { 0.0f, 0.0f };

		// Light
		glm::vec3		mLightPos			= { 0, 100.0f, 100.0f };
		float			mLightIntensity		= 1.0f;
		float			mAmbientIntensity	= 0.5f;
		float			mDiffuseIntensity	= 1.0f;

		float			mNormalSpecIntens	= 0.5f;
		float			mScanSpecIntens		= 0.25f;
		RGBColorFloat	mNormalSpecColor	= { 1.0f, 1.0f, 1.0f };
		RGBColorFloat	mScanSpecColor		= { 1.0f, 1.0f, 1.0f };
		float			mNormalSpecShine	= { 20.0f };
		float			mScanSpecShine		= { 10.0f };
		float			mNormalRotValue		= { 0.0f };
		float			mScanRotValue		= { 0.0f };
		glm::vec3		mNormalRotAngle		= { 0.0f, 1.0f, 0.0f };
		glm::vec3		mScanRotAngle		= { 0.0f, 1.0f, 0.0f };

		float			mWindSpeed			= 0.25f;
		float			mWindScale			= 0.6f;
		float			mWindFreq			= 10.0f;
		float			mWindRandom			= 0.15f;
		float			mNormalRandom		= 0.5f;
		float			mNormalScale		= 1.0f;
		float			mDiffuseSpecInfl	= 0.0f;			//< Scales specular highlights based on diffuse information

	private:
		double mWindTime = 0.0;
		double mTexTimeU = 0.0;
		double mTexTimeV = 0.0;
	};
}

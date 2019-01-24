#pragma once

// External Includes
#include <component.h>
#include <componentptr.h>
#include <entityptr.h>
#include <orbitcomponent.h>
#include <transformcomponent.h>
#include <renderablemeshcomponent.h>

namespace nap
{
	class UpdateMaterialComponentInstance;

	/**
	 * UpdateMaterialComponent
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

		ComponentPtr<TransformComponent>		mCameraTransformComponent;	///< Property: 'CameraTransformComponent' link to the camera transform component
		ComponentPtr<RenderableMeshComponent>	mCombinationMeshComponent;	///< Property: 'CombinationMeshComponent' link to the combination mesh component
		ComponentPtr<RenderableMeshComponent>	mSunCloudsMeshComponent;	///< Property: 'SunCloudsMeshComponent' link to the sun clouds mesh component
		ComponentPtr<RenderableMeshComponent>	mSunGlareMeshComponent;		///< Property: 'SunGlareMeshComponent' link to the sun glare mesh component
		ComponentPtr<RenderableMeshComponent>	mStaticMeshComponent;		///< Property: 'StaticMeshComponent' link to the static mesh component
		ComponentPtr<RenderableMeshComponent>	mPartyMeshComponent;		///< Property: 'PartyMeshComponent' link to the party mesh component
		ComponentPtr<OrbitComponent>			mOrbitComponent;			///< Property: 'OrbitComponent' link to the orbit component
		EntityPtr								mLightRigEntity;			///< Property: 'LightRigEntity' link to the light rig entity
	};


	/**
	 * UpdateMaterialComponentInstance	
	 */
	class NAPAPI UpdateMaterialComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		UpdateMaterialComponentInstance(EntityInstance& entity, Component& resource) : ComponentInstance(entity, resource) { }

		/**
		 * Initialize UpdateMaterialComponentInstance based on the UpdateMaterialComponent resource
		 * @param errorState should hold the error message when initialization fails
		 * @return if the UpdateMaterialComponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * update UpdateMaterialComponentInstance. This is called by NAP core automatically
		 * @param deltaTime time in between frames in seconds
		 */
		virtual void update(double deltaTime) override;
		void updateCameraLocation();
		void updateParty(double deltaTime);
		void updatePartyCenter();
		void updateSunClouds(double deltaTime);
		void updateSunGlareOrbit();

		// Pointers to the run time Component Instances, set during de-serialization
		ComponentInstancePtr<TransformComponent>		mCameraTransformComponent = { this, &UpdateMaterialComponent::mCameraTransformComponent };
		ComponentInstancePtr<RenderableMeshComponent>	mCombinationMeshComponent = { this, &UpdateMaterialComponent::mCombinationMeshComponent };
		ComponentInstancePtr<RenderableMeshComponent>	mSunCloudsMeshComponent = { this, &UpdateMaterialComponent::mSunCloudsMeshComponent };
		ComponentInstancePtr<RenderableMeshComponent>	mSunGlareMeshComponent = { this, &UpdateMaterialComponent::mSunGlareMeshComponent };
		ComponentInstancePtr<RenderableMeshComponent>	mStaticMeshComponent = { this, &UpdateMaterialComponent::mStaticMeshComponent };
		ComponentInstancePtr<RenderableMeshComponent>	mPartyMeshComponent = { this, &UpdateMaterialComponent::mPartyMeshComponent };
		ComponentInstancePtr<OrbitComponent>			mOrbitComponent = { this, &UpdateMaterialComponent::mOrbitComponent };
		EntityInstancePtr								mLightRigEntity = { this, &UpdateMaterialComponent::mLightRigEntity };

		// Exposed properties for GUI
		void		setCombinationTextures(nap::Texture2D& textureOne, nap::Texture2D& textureTwo);
		float*		getCombinationBlendValuePtr();

		float*		getSunCloudsRotationPtr();
		float*		getSunCloudsContrastPtr();
		float*		getSunCloudsScalePtr();
		float*		getSunCloudsTemperaturePtr();
		float*		getSunCloudsCloudRatioPtr();
		float*		getSunCloudsCloudFillPtr();
		float*		getSunCloudsLightFillPtr();
		
		float*		getSunGlareOuterSizePtr();
		float*		getSunGlareInnerSizePtr();
		float*		getSunGlareStretchPtr();
		float*		getSunGlareTemperaturePtr();
		
		float*		getStaticTemperaturePtr();
		
		int*		getPartyWaveCountPtr();
		float*		getPartyWaveLengthPtr();
		float*		getPartyWaveFalloffStartPtr();
		float*		getPartyWaveFalloffEndPtr();
		float*		getPartyWaveNoiseScalePtr();
		float*		getPartyWaveNoiseInfluencePtr();
		float*		getPartyWaveCenterPtr();
		float*		getPartyWaveHighlightLengthPtr();
		float*		getPartyWaveHighlightIntensityPtr();

		float		mSunCloudsNoiseSpeed = 0.01f;
		const float	mSunCloudsNoiseSpeedMax = 0.25f;

		float		mSunCloudsWindSpeed = 0.05f;
		const float	mSunCloudsWindSpeedMax = 0.5f;

		const float mSunCloudsScaleMin = 0.1f;
		const float mSunCloudsScaleMax = 2.0f;

		const float	mSunGlareSizeMin = 0.05f;
		const float	mSunGlareSizeMax = 0.25f;

		const float	mSunGlareStretchMin = 1.0f;
		const float	mSunGlareStretchMax = 10.0f;

		float		mPartyCenter[2] = { 0.2f, 0.8f };
		int			mPartyBPM = 20;
		int			mPartyBPMMin = 0;
		int			mPartyBPMMax = 240;
		int			mPartyWaveCountMax = 12;
		float		mPartyWaveLengthMin = 0.1f;
		float		mPartyWaveFalloffMin = 0.1f;
		float		mPartyWaveFalloffMax = 1.5f;
		float		mPartyWaveNoiseSpeed = 0.1f;
		float		mPartyWaveNoiseSpeedMax = 1.0f;
		float		mPartyWaveNoiseScaleMin = 0.1f;
		float		mPartyWaveNoiseScaleMax = 1.0f;
		float		mPartyWaveNoiseInfluenceMax = 0.5f;
	};
}

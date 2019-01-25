#include "updatematerialcomponent.h"

// External Includes
#include <entity.h>
#include <mathutils.h>

// nap::UpdateMaterialComponent run time class definition 
RTTI_BEGIN_CLASS(nap::UpdateMaterialComponent)
	RTTI_PROPERTY("CameraTransformComponent", &nap::UpdateMaterialComponent::mCameraTransformComponent, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("CombinationMeshComponent", &nap::UpdateMaterialComponent::mCombinationMeshComponent, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("SunCloudsMeshComponent", &nap::UpdateMaterialComponent::mSunCloudsMeshComponent, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("SunGlareMeshComponent", &nap::UpdateMaterialComponent::mSunGlareMeshComponent, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("StaticMeshComponent", &nap::UpdateMaterialComponent::mStaticMeshComponent, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("PartyMeshComponent", &nap::UpdateMaterialComponent::mPartyMeshComponent, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("SoundMeshComponent", &nap::UpdateMaterialComponent::mSoundMeshComponent, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("OrbitComponent", &nap::UpdateMaterialComponent::mOrbitComponent, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("LightRigEntity", &nap::UpdateMaterialComponent::mLightRigEntity, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::UpdateMaterialComponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::UpdateMaterialComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void UpdateMaterialComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{

	}


	bool UpdateMaterialComponentInstance::init(utility::ErrorState& errorState)
	{
		// Call an intial update to apply properties
		updateSunGlareOrbit();
		updatePartyCenter();

		if (mPartyPresetsActive)
			startPartyPresetTransition(PARTY_PRESET_CALM);

		return true;
	}


	void UpdateMaterialComponentInstance::update(double deltaTime)
	{
		// Update party
		updateParty(deltaTime);

		// Update clouds shader
		updateSunClouds(deltaTime);

		// Update camera location
		updateCameraLocation();
	}


	void UpdateMaterialComponentInstance::startPartyGlitch()
	{
		mPartyGlitchOn = true;
		mPartyGlitchStopTimer = 0.0f;
		mPartyMeshComponent->getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uGlitchSize").mValue = mPartyGlitchSize;
		glm::vec3* glitchPosition = &mPartyMeshComponent->getMaterialInstance().getOrCreateUniform<nap::UniformVec3>("uGlitchPosition").mValue;
		glitchPosition->x = nap::math::random(0.0f, 1.0f);
		glitchPosition->y = nap::math::random(0.0f, 1.0f);
	}


	void UpdateMaterialComponentInstance::stopPartyGlitch()
	{
		mPartyGlitchOn = false;
		mPartyMeshComponent->getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uGlitchSize").mValue = 0.0f;
	}


	void UpdateMaterialComponentInstance::startPartyPresetTransition(PartyPresetTypes type)
	{
		mPartyPresetsActive = type != PARTY_PRESET_NONE;
		mPartyPresetTransitionActive = mPartyPresetsActive;
		mPartyPresetTransitionBpmVelocity = 0.0f;
		mPartyPresetTransitionNoiseSpeedVelocity = 0.0f;
		mPartyPresetTransitionNoiseInfluenceVelocity = 0.0f;
		mPartyPresetBeats = 0.0f;
		mPartyPresetType = type;
		switch (type)
		{
		case PARTY_PRESET_CALM:
			mPartyPreset = &mPartyPresetsCalm[nap::math::random(0, static_cast<int>(sizeof(mPartyPresetsCalm) / sizeof(mPartyPresetsCalm[0])) - 1)];
			break;
		case PARTY_PRESET_INTENSE:
			mPartyPreset = &mPartyPresetsIntense[nap::math::random(0, static_cast<int>(sizeof(mPartyPresetsIntense) / sizeof(mPartyPresetsIntense[0])) - 1)];
			break;
		}
		mPartyPresetTransitionBpmIncrement = mPartyPreset->bpm > mPartyBPM;
		mPartyPresetTransitionNoiseSpeedIncrement = mPartyPreset->noiseSpeed > mPartyWaveNoiseSpeed;
		mPartyPresetTransitionNoiseInfluenceIncrement = mPartyPreset->noiseInfluence > *getPartyWaveNoiseInfluencePtr();
	}


	void UpdateMaterialComponentInstance::updatePartyPresetTransition(double deltaTime)
	{
		// Smoothly transition the party preset values
		nap::math::smooth(mPartyBPM, mPartyPreset->bpm, mPartyPresetTransitionBpmVelocity, static_cast<float>(deltaTime), mPartyPresetTransitionDuration, 1000000.0f);
		nap::math::smooth(mPartyWaveNoiseSpeed, mPartyPreset->noiseSpeed, mPartyPresetTransitionNoiseSpeedVelocity, static_cast<float>(deltaTime), mPartyPresetTransitionDuration, 1000000.0f);
		nap::math::smooth(*getPartyWaveNoiseInfluencePtr(), mPartyPreset->noiseInfluence, mPartyPresetTransitionNoiseInfluenceVelocity, static_cast<float>(deltaTime), mPartyPresetTransitionDuration, 1000000.0f);

		// Stop the updating when the transition is complete
		if
		(
			(mPartyPresetTransitionBpmIncrement && mPartyBPM + mPartyPresetTransitionTolerance >= mPartyPreset->bpm ||
			!mPartyPresetTransitionBpmIncrement && mPartyBPM - mPartyPresetTransitionTolerance <= mPartyPreset->bpm) &&
			(mPartyPresetTransitionNoiseSpeedIncrement && mPartyWaveNoiseSpeed + mPartyPresetTransitionTolerance >= mPartyPreset->noiseSpeed ||
			!mPartyPresetTransitionNoiseSpeedIncrement && mPartyWaveNoiseSpeed - mPartyPresetTransitionTolerance <= mPartyPreset->noiseSpeed) &&
			(mPartyPresetTransitionNoiseInfluenceIncrement && *getPartyWaveNoiseInfluencePtr() + mPartyPresetTransitionTolerance >= mPartyPreset->noiseInfluence ||
			!mPartyPresetTransitionNoiseInfluenceIncrement && *getPartyWaveNoiseInfluencePtr() - mPartyPresetTransitionTolerance <= mPartyPreset->noiseInfluence)
		)
		{
			mPartyBPM = mPartyPreset->bpm;
			mPartyPresetTransitionActive = false;
		}
	}


	void UpdateMaterialComponentInstance::updateParty(double deltaTime)
	{
		float beatIncrement = (mPartyBPM / 60.0f) * static_cast<float>(deltaTime);

		// update the preset transition if active
		if (mPartyPresetTransitionActive)
		{
			updatePartyPresetTransition(deltaTime);
		}
		// otherwise, start counting the preset duration if active
		else if (mPartyPresetsActive)
		{
			mPartyPresetBeats += abs(beatIncrement);
			if (mPartyPresetBeats >= mPartyPreset->durationBeats)
				startPartyPresetTransition(mPartyPresetType == PARTY_PRESET_CALM ? PARTY_PRESET_INTENSE : PARTY_PRESET_CALM);
		}

		// update beat / noise
		float* uBeat = &mPartyMeshComponent->getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uBeat").mValue;
		float* uWaveNoiseZ = &mPartyMeshComponent->getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uWaveNoiseZ").mValue;
		*uBeat = fmod(*uBeat + beatIncrement, 1.0f);
		*uWaveNoiseZ += mPartyWaveNoiseSpeed * static_cast<float>(deltaTime);

		// update glitch
		if (mPartyGlitchOn)
		{
			mPartyGlitchStopTimer += static_cast<float>(deltaTime);
			if (mPartyGlitchStopTimer >= mPartyGlitchDuration)
				stopPartyGlitch();
		}
		else
		{
			mPartyGlitchCheckTimer += static_cast<float>(deltaTime);
			if (mPartyGlitchCheckTimer >= mPartyGlitchCheckInterval)
			{
				if (mPartyGlitchIntensity >= nap::math::random(0.0f, 1.0f))
					startPartyGlitch();
				else
					mPartyGlitchCheckTimer = 0.0f;
			}
		}
	}


	void UpdateMaterialComponentInstance::updatePartyCenter()
	{
		glm::vec3* uCenter = &mPartyMeshComponent->getMaterialInstance().getOrCreateUniform<nap::UniformVec3>("uCenter").mValue;
		uCenter->x = mPartyCenter[0];
		uCenter->y = mPartyCenter[1];
	}


	void UpdateMaterialComponentInstance::updateCameraLocation()
	{
		// Retrieve camera location
		glm::vec3 cam_pos = math::extractPosition(mCameraTransformComponent->getGlobalTransform());

		// Get all renderable meshes under the light rig and set camera location uniform (they should all have one)
		std::vector<RenderableMeshComponentInstance*> render_meshes;
		mLightRigEntity->getComponentsOfTypeRecursive<RenderableMeshComponentInstance>(render_meshes);
		for (auto& rmesh : render_meshes)
		{
			nap::MaterialInstance& material = rmesh->getMaterialInstance();
			material.getOrCreateUniform<nap::UniformVec3>("cameraLocation").setValue(cam_pos);
		}
	}


	void UpdateMaterialComponentInstance::updateSunClouds(double deltaTime)
	{
		// Apply wind and noise forces to cloud shader
		glm::vec3* sunCloudsOffset = &mSunCloudsMeshComponent->getMaterialInstance().getOrCreateUniform<nap::UniformVec3>("uOffset").mValue;
		float sunCloudsRotation = mSunCloudsMeshComponent->getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uRotation").mValue;
		float windDirectionRad = nap::math::radians(sunCloudsRotation);
		float windDistance = mSunCloudsWindSpeed * static_cast<float>(deltaTime);
		sunCloudsOffset->x += cos(windDirectionRad) * windDistance;
		sunCloudsOffset->y += sin(windDirectionRad) * windDistance;
		sunCloudsOffset->z += mSunCloudsNoiseSpeed * static_cast<float>(deltaTime);
	}


	void UpdateMaterialComponentInstance::updateSunGlareOrbit()
	{
		mSunGlareMeshComponent->getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uOrbitAngle").mValue = mOrbitComponent->getAngle();
		mSunGlareMeshComponent->getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uOrbitRadius").mValue = mOrbitComponent->mRadius;
		glm::vec3* sunGlareOrbitCenter = &mSunGlareMeshComponent->getMaterialInstance().getOrCreateUniform<nap::UniformVec3>("uOrbitCenter").mValue;
		sunGlareOrbitCenter->x = mOrbitComponent->mCenter[0];
		sunGlareOrbitCenter->y = mOrbitComponent->mCenter[1];
	}


	void UpdateMaterialComponentInstance::setCombinationTextures(nap::Texture2D& textureOne, nap::Texture2D& textureTwo)
	{
		mCombinationMeshComponent->getMaterialInstance().getOrCreateUniform<nap::UniformTexture2D>("uTextureOne").setTexture(textureOne);
		mCombinationMeshComponent->getMaterialInstance().getOrCreateUniform<nap::UniformTexture2D>("uTextureTwo").setTexture(textureTwo);
	}


	float* UpdateMaterialComponentInstance::getCombinationBlendValuePtr()
	{
		return &mCombinationMeshComponent->getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uBlendValue").mValue;
	}


	float* UpdateMaterialComponentInstance::getSunCloudsRotationPtr()
	{
		return &mSunCloudsMeshComponent->getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uRotation").mValue;
	}


	float* UpdateMaterialComponentInstance::getSunCloudsContrastPtr()
	{
		return &mSunCloudsMeshComponent->getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uContrast").mValue;
	}


	float* UpdateMaterialComponentInstance::getSunCloudsScalePtr()
	{
		return &mSunCloudsMeshComponent->getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uScale").mValue;
	}


	float* UpdateMaterialComponentInstance::getSunCloudsTemperaturePtr()
	{
		return &mSunCloudsMeshComponent->getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uTemperature").mValue;
	}


	float* UpdateMaterialComponentInstance::getSunCloudsCloudRatioPtr()
	{
		return &mSunCloudsMeshComponent->getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uCloudRatio").mValue;
	}


	float* UpdateMaterialComponentInstance::getSunCloudsCloudFillPtr()
	{
		return &mSunCloudsMeshComponent->getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uCloudFill").mValue;
	}


	float* UpdateMaterialComponentInstance::getSunCloudsLightFillPtr()
	{
		return &mSunCloudsMeshComponent->getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uLightFill").mValue;
	}


	float* UpdateMaterialComponentInstance::getSunGlareOuterSizePtr()
	{
		return &mSunGlareMeshComponent->getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uOuterSize").mValue;
	}


	float* UpdateMaterialComponentInstance::getSunGlareInnerSizePtr()
	{
		return &mSunGlareMeshComponent->getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uInnerSize").mValue;
	}


	float* UpdateMaterialComponentInstance::getSunGlareStretchPtr()
	{
		return &mSunGlareMeshComponent->getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uStretch").mValue;
	}


	float* UpdateMaterialComponentInstance::getSunGlareTemperaturePtr()
	{
		return &mSunGlareMeshComponent->getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uTemperature").mValue;
	}


	float* UpdateMaterialComponentInstance::getStaticTemperaturePtr()
	{
		return &mStaticMeshComponent->getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uTemperature").mValue;
	}


	int* UpdateMaterialComponentInstance::getPartyWaveCountPtr()
	{
		return &mPartyMeshComponent->getMaterialInstance().getOrCreateUniform<nap::UniformInt>("uWaveCount").mValue;
	}


	float* UpdateMaterialComponentInstance::getPartyWaveLengthPtr()
	{
		return &mPartyMeshComponent->getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uWaveLength").mValue;
	}


	float* UpdateMaterialComponentInstance::getPartyWaveFalloffStartPtr()
	{
		return &mPartyMeshComponent->getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uWaveFalloffStart").mValue;
	}


	float* UpdateMaterialComponentInstance::getPartyWaveFalloffEndPtr()
	{
		return &mPartyMeshComponent->getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uWaveFalloffEnd").mValue;
	}


	float* UpdateMaterialComponentInstance::getPartyWaveNoiseScalePtr()
	{
		return &mPartyMeshComponent->getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uWaveNoiseScale").mValue;
	}


	float* UpdateMaterialComponentInstance::getPartyWaveNoiseInfluencePtr()
	{
		return &mPartyMeshComponent->getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uWaveNoiseInfluence").mValue;
	}


	float* UpdateMaterialComponentInstance::getPartyWaveCenterPtr()
	{
		return &mPartyMeshComponent->getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uWaveCenter").mValue;
	}


	float* UpdateMaterialComponentInstance::getPartyWaveHighlightLengthPtr()
	{
		return &mPartyMeshComponent->getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uWaveHighlightLength").mValue;
	}


	float* UpdateMaterialComponentInstance::getPartyWaveHighlightIntensityPtr()
	{
		return &mPartyMeshComponent->getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uWaveHighlightIntensity").mValue;
	}


	float* UpdateMaterialComponentInstance::getSoundTemperaturePtr()
	{
		return &mSoundMeshComponent->getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uTemperature").mValue;
	}
}

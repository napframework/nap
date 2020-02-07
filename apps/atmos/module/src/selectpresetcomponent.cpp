#include "SelectPresetComponent.h"
#include "updatematerialcomponent.h"

#include <nap/logger.h>
#include <entity.h>
#include <nap/core.h>
#include <algorithm>
#include <nap/resourcemanager.h>
#include <scene.h>

// nap::SelectPresetComponent run time class definition 
RTTI_BEGIN_CLASS(nap::SelectPresetComponent)
	// Put additional properties here
	RTTI_PROPERTY("PresetParameterGroup", &nap::SelectPresetComponent::mPresetParameterGroup, nap::rtti::EPropertyMetaData::Required)
	
	//now controlling update material directly..
	RTTI_PROPERTY("FogColor", &nap::SelectPresetComponent::mFogColor, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("BackgroundColor", &nap::SelectPresetComponent::mBackgroundColor, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("PresetIndex",	&nap::SelectPresetComponent::mPresetIndex,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Presets",		&nap::SelectPresetComponent::mPresets,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("FadeColor", &nap::SelectPresetComponent::mFadeColor, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("AnimationDuration", &nap::SelectPresetComponent::mAnimationDuration, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::SelectPresetComponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SelectPresetComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void SelectPresetComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{

	}


	bool SelectPresetComponentInstance::init(utility::ErrorState& errorState)
	{
		nap::SelectPresetComponent* resource = getComponent<SelectPresetComponent>();
		
		ResourceManager* resourceManager = this->getEntityInstance()->getCore()->getResourceManager();
	
		mPresetGroup = resource->mPresetParameterGroup.get();
		mParameterService = this->getEntityInstance()->getCore()->getService<nap::ParameterService>();
		std::vector<std::string> availablePresets = mParameterService->getPresets(*mPresetGroup);
	
		mFadeColor = resource->mFadeColor;
		mAnimationDuration = resource->mAnimationDuration;

		if (!errorState.check(availablePresets.size() > 0, "No presets available"))
			return false;

		if (!errorState.check(resource->mPresets.size() > 0, "No presets defined"))
			return false;

		for (const auto& preset : resource->mPresets)
		{
			if (!errorState.check(std::find(availablePresets.begin(), availablePresets.end(), preset) != availablePresets.end(), "preset %s does not exist", preset))
				return false;

			mPresets.push_back(preset);
		}

		return true;
	}

	std::string SelectPresetComponentInstance::getCurrentPreset() 
	{
		return mPresets[mPresetIndex];
	}

	void SelectPresetComponentInstance::selectPreset(int index)
	{
		if (index >= 0 && index < mPresets.size())
		{
			std::string preset = mPresets[index];
			selectPreset(preset);
		}
		else {
			//ERROR here?
		}
	}

	void SelectPresetComponentInstance::selectPreset(const std::string& presetPath)
	{
		mNextPreset = presetPath;

		ResourceManager* resourceManager = this->getEntityInstance()->getCore()->getResourceManager();
		nap::rtti::ObjectPtr<Scene> scene = resourceManager->findObject<Scene>("Scene");
		mScanEntity = scene->findEntity("ScanEntity");
		nap::UpdateMaterialComponentInstance& up_mat_comp = mScanEntity->getComponent<UpdateMaterialComponentInstance>();
		
		mCurrentColor = up_mat_comp.mFogColor;
		mFogSettingsStart = up_mat_comp.getFogSettings();
		mFogSettingsEnd = glm::vec4(0, 1, 0, 1); 
		mAnimationTime = 0;
		mPresetSwitchAnimationState = FADE_OUT_CURRENT;
	}

	void SelectPresetComponentInstance::loadPreset(const std::string& presetPath)
	{
		nap::Logger::debug("loading preset: %s...", presetPath);

		utility::ErrorState presetLoadError;
		if (!mParameterService->loadPreset(*mPresetGroup, presetPath, presetLoadError))
		{
			nap::Logger::fatal("error: %s", presetLoadError.toString().c_str());
		}
	}

	void SelectPresetComponentInstance::update(double deltaTime)
	{
		updatePresetSwitchAnimation(deltaTime);
	}


	void SelectPresetComponentInstance::updatePresetSwitchAnimation(double deltaTime) 
	{
		float lerpProgress = 0;
		nap::SelectPresetComponent* resource = getComponent<SelectPresetComponent>();

		switch (mPresetSwitchAnimationState)
		{
		case nap::NONE:
			break;
		case nap::FADE_OUT_CURRENT:
			mAnimationTime = mAnimationTime + deltaTime;
			lerpProgress = mAnimationTime / mAnimationDuration;
			fadeToFadeColor(lerpProgress);
			if (mAnimationTime >= mAnimationDuration)
				mPresetSwitchAnimationState = LOAD_NEXT;
			break;
		case nap::LOAD_NEXT:
			loadPreset(mNextPreset);
			mPresetSwitchAnimationState = NONE;


			//TODO where to test for new scene loaded?
			//startRevealAnimation();
			break;
		case nap::REVEAL_NEXT:
			mAnimationTime = mAnimationTime + deltaTime;
			lerpProgress = mAnimationTime / mAnimationDuration;
			lerpProgress = 1.0 - lerpProgress;
			fadeToFadeColor(lerpProgress);

			if (mAnimationTime >= mAnimationDuration)
				mPresetSwitchAnimationState = NONE;
			break;
		default:
			break;
		}
	}

	void SelectPresetComponentInstance::startRevealAnimation() 
	{
		// did this in init before but entity wasn't available yet...
		ResourceManager* resourceManager = this->getEntityInstance()->getCore()->getResourceManager();
		nap::rtti::ObjectPtr<Scene> scene = resourceManager->findObject<Scene>("Scene");
		mScanEntity = scene->findEntity("ScanEntity");
		nap::UpdateMaterialComponentInstance& up_mat_comp = mScanEntity->getComponent<UpdateMaterialComponentInstance>();


		//changing the current fog color to that of the new scene...no idea if this works
		mCurrentColor = up_mat_comp.mFogColor;
		fadeToFadeColor(1);
		mPresetSwitchAnimationState = REVEAL_NEXT;
		mAnimationTime = 0;
		mFogSettingsStart = up_mat_comp.getFogSettings();
		mFogSettingsEnd = glm::vec4(0, 1, 0, 1); //have to find correct values
	}

	void SelectPresetComponentInstance::fadeToFadeColor(float fadeProgress) 
	{
			RGBColorFloat color = lerpColors(mCurrentColor, mFadeColor, fadeProgress);

			//this is how I did it before:
			//nap::SelectPresetComponent* resource = getComponent<SelectPresetComponent>();
			//resource->mFogColor->setValue(color);
			//resource->mBackgroundColor->setValue(color);

			//this more direct way is how I do it now:
			updateFogColor(color);
			updateFogSettings(fadeProgress);
	}

	void SelectPresetComponentInstance::updateFogColor(RGBColorFloat& color)
	{
		nap::UpdateMaterialComponentInstance& up_mat_comp = mScanEntity->getComponent<UpdateMaterialComponentInstance>();
		up_mat_comp.mFogColor = color;
	}

	void SelectPresetComponentInstance::updateFogSettings(float lerpValue)
	{
		nap::UpdateMaterialComponentInstance& up_mat_comp = mScanEntity->getComponent<UpdateMaterialComponentInstance>();
		up_mat_comp.fogFade(mFogSettingsStart, mFogSettingsEnd, lerpValue);
	}

	RGBColorFloat SelectPresetComponentInstance::lerpColors(RGBColorFloat& color1, RGBColorFloat& color2, float lerpValue)
	{
		glm::vec3 colorResultVec = nap::math::lerp<glm::vec3>(color1.toVec3(), color2.toVec3(), lerpValue);
		return RGBColorFloat(colorResultVec.r, colorResultVec.g, colorResultVec.b);;
	}

}
#include "SelectPresetComponent.h"

#include <nap/logger.h>

// External Includes
#include <entity.h>
#include <nap/core.h>
//#include <utility.h>
#include <algorithm>

// nap::SelectPresetComponent run time class definition 
RTTI_BEGIN_CLASS(nap::SelectPresetComponent)
	// Put additional properties here
	RTTI_PROPERTY("PresetParameterGroup", &nap::SelectPresetComponent::mPresetParameterGroup, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("FogColor", &nap::SelectPresetComponent::mFogColor, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("BackgroundColor", &nap::SelectPresetComponent::mBackgroundColor, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("PresetIndex",	&nap::SelectPresetComponent::mPresetIndex,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Presets",		&nap::SelectPresetComponent::mPresets,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("FadeColor", &nap::SelectPresetComponent::mFadeColor, nap::rtti::EPropertyMetaData::Required)
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

		mParameterService = this->getEntityInstance()->getCore()->getService<nap::ParameterService>();
		ParameterGroup* presetGroup = resource->mPresetParameterGroup.get();
		std::vector<std::string> availablePresets = mParameterService->getPresets(*presetGroup);
	
		mFadeColor = resource->mFadeColor;

		if (!errorState.check(availablePresets.size() > 0, "No presets available"))
			return false;

		if (!errorState.check(resource->mPresets.size() > 0, "No presets defined"))
			return false;

		for (const auto& presetPath : resource->mPresets)
		{
			//if (!errorState.check(utility::fileExists(presetPath), "preset %s does not exist", presetPath))
			//	return false;

			//std::string presetName = presetPath.split("/")[3];
			//if (!errorState.check(std::find(availablePresets.begin(), availablePresets.end(), preset) != availablePresets.end(), "preset %s does not exist", preset))
			//	return false;

			mPresets.push_back(presetPath);
		}

		return true;
	}

	std::string SelectPresetComponentInstance::getCurrentPreset() 
	{
		return mPresets[mPresetIndex];
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
			mAnimationTime += deltaTime;
			lerpProgress = mAnimationTime / mAnimationDuration;
			FadeToFadeColor(lerpProgress);

			if (lerpProgress >= 1)
				mPresetSwitchAnimationState = LOAD_NEXT;

			break;
		case nap::LOAD_NEXT:
			loadPreset(mNextPreset);
		
			mCurrentColor = resource->mFogColor->mValue;
			FadeToFadeColor(1);
			mPresetSwitchAnimationState = REVEAL_NEXT;
			mAnimationTime = 0;
			break;
		case nap::REVEAL_NEXT:
			mAnimationTime += deltaTime;
			lerpProgress = mAnimationTime / mAnimationDuration;
			lerpProgress = 1.0 - lerpProgress;
			FadeToFadeColor(lerpProgress);

			if (lerpProgress < 0.0)
				mPresetSwitchAnimationState = NONE;

			break;
		default:
			break;
		}
	}

	void SelectPresetComponentInstance::FadeToFadeColor(float fadeProgress) 
	{
			RGBColorFloat color = LerpColors(mCurrentColor, mFadeColor, fadeProgress);

			nap::SelectPresetComponent* resource = getComponent<SelectPresetComponent>();
			resource->mFogColor->setValue(color);
			resource->mBackgroundColor->setValue(color);
	}

	RGBColorFloat SelectPresetComponentInstance::LerpColors(RGBColorFloat color1, RGBColorFloat color2, float lerpValue)
	{
		//im pretty sure i can do this in a better way....

		glm::vec3 color1Vec = color1.toVec3();
		glm::vec3 color2Vec = color2.toVec3();
		glm::vec3 colorResultVec = nap::math::lerp<glm::vec3>(color1Vec, color2Vec, lerpValue);
		RGBColorFloat colorResult = RGBColorFloat(colorResultVec.r, colorResultVec.g, colorResultVec.b);
	
		return colorResult;
	}


	void SelectPresetComponentInstance::selectPreset(int index)
	{
		//nap::Logger::debug("preset select: %s...", index);

		//if (index >= 0 && index < mPresets.size())
		//{

			std::string preset = mPresets[index];

			selectPreset(preset);
//		}
	}

	void SelectPresetComponentInstance::selectPreset(const std::string& presetPath)
	{
		mNextPreset = presetPath;
		mPresetSwitchAnimationState = FADE_OUT_CURRENT;
		mAnimationTime = 0;
		nap::SelectPresetComponent* resource = getComponent<SelectPresetComponent>();
		mCurrentColor = resource->mFogColor->mValue;
	}

	void SelectPresetComponentInstance::loadPreset(const std::string& presetPath)
	{
		nap::Logger::debug("loading preset: %s...", presetPath);

		utility::ErrorState presetLoadError;
		ParameterGroup& parameterGroup = mParameterService->getRootGroup();

		if (!mParameterService->loadPreset(parameterGroup, presetPath, presetLoadError))
		{
			nap::Logger::fatal("error: %s", presetLoadError.toString().c_str());
		}
	}



}
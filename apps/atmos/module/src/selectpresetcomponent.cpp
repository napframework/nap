#include "SelectPresetComponent.h"


#include <nap/logger.h>
#include <entity.h>
#include <nap/core.h>
#include <algorithm>
#include <nap/resourcemanager.h>
#include <scene.h>

// nap::SelectPresetComponent run time class definition 
RTTI_BEGIN_CLASS(nap::SelectPresetComponent)
	RTTI_PROPERTY("PresetParameterGroup", &nap::SelectPresetComponent::mPresetParameterGroup, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("FogParameterGroup", &nap::SelectPresetComponent::mFogParameterGroup, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("FogColor", &nap::SelectPresetComponent::mFogColor, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("BackgroundColor", &nap::SelectPresetComponent::mBackgroundColor, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("PresetIndex", &nap::SelectPresetComponent::mPresetIndex, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Presets", &nap::SelectPresetComponent::mPresets, nap::rtti::EPropertyMetaData::Required)
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
	
		mPresetGroup = resource->mPresetParameterGroup;

		mFogColor = resource->mFogColor;
		mFogGroup = resource->mFogParameterGroup;
		mPresetIndex = -1;

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

	void SelectPresetComponentInstance::selectPresetByIndex(int index)
	{
		if (index >= 0 && index < mPresets.size())
		{
			nap::SelectPresetComponent* resource = getComponent<SelectPresetComponent>();
			resource->mPresetIndex = index;
		}
		else {
			//ERROR here?
			assert(0);
		}
	}

	void SelectPresetComponentInstance::transitionToPreset(const std::string& presetPath)
	{
		mNextPreset = presetPath;
		startTransition(PresetSwitchTransitionState::FADE_OUT_CURRENT);
	}

	void SelectPresetComponentInstance::loadPreset(const std::string& presetPath)
	{
		nap::Logger::debug("loading preset: %s...", presetPath);
		mCurrentPreset = presetPath;
		mParameterService->presetLoaded.connect(mPresetLoaded);
	
		utility::ErrorState presetLoadError;
		if (!mParameterService->loadPreset(*mPresetGroup, presetPath, presetLoadError))
		{
			nap::Logger::fatal("error: %s", presetLoadError.toString().c_str());
		}
	}

	void SelectPresetComponentInstance::update(double deltaTime)
	{
		if (mPresetIndex != getComponent<SelectPresetComponent>()->mPresetIndex)
		{
			mPresetIndex = getComponent<SelectPresetComponent>()->mPresetIndex;
			std::string presetName = mPresets[mPresetIndex];
			transitionToPreset(presetName);
		}

		updatePresetSwitchAnimation(deltaTime);
	}

	void SelectPresetComponentInstance::updatePresetSwitchAnimation(double deltaTime) 
	{
		float lerpProgress = 0;
		switch (mPresetSwitchAnimationState)
		{
		case nap::NONE:
			break;
		case nap::FADE_OUT_CURRENT:
			mAnimationTime = mAnimationTime + deltaTime;
			lerpProgress = mAnimationTime / mAnimationDuration;
			updateFogFade(lerpProgress);

			if (mAnimationTime >= mAnimationDuration)
				mPresetSwitchAnimationState = LOAD_NEXT;
			break;
		case nap::LOAD_NEXT:
			mPresetSwitchAnimationState = WAIT_FOR_LOAD;
			loadPreset(mNextPreset);
			break;
		case  nap::WAIT_FOR_LOAD:
			//waiting on preset loaded to come back. load is blocking so we never get here...
			break;
		case nap::REVEAL_NEXT:
			mAnimationTime = mAnimationTime + deltaTime;
			lerpProgress = mAnimationTime / mAnimationDuration;
			lerpProgress = 1.0 - lerpProgress;
			updateFogFade(lerpProgress);

			if (mAnimationTime >= mAnimationDuration)
				mPresetSwitchAnimationState = NONE;
			break;
		default:
			break;
		}
	}

	void SelectPresetComponentInstance::startTransition(const PresetSwitchTransitionState& presetSwitchTransitionState) {
		
		if (mPresetSwitchAnimationState == presetSwitchTransitionState)
			return;

		mPresetSwitchAnimationState = presetSwitchTransitionState;
		mCurrentColor = getFogColor();
		mFogSettingsStart = getFogSettings();
		mFogSettingsEnd = glm::vec4(0, 1, 0, 1);
		mAnimationTime = 0;
	}

	void SelectPresetComponentInstance::updateFogFade(double fadeProgress) 
	{
		fadeProgress = nap::math::clamp(fadeProgress, 0.0, 1.0);

		glm::vec3 colorVec = nap::math::lerp<glm::vec3>(mCurrentColor.toVec3(), mFadeColor.toVec3(), fadeProgress);
		setFogColor(RGBColorFloat(colorVec.r, colorVec.g, colorVec.b));

		glm::vec4 fogSettings = nap::math::lerp(mFogSettingsStart, mFogSettingsEnd, fadeProgress);
		setFogSettings(fogSettings);
	}

	RGBColorFloat& SelectPresetComponentInstance::getFogColor()
	{
		return mFogColor->mValue;
	}

	void SelectPresetComponentInstance::setFogColor(RGBColorFloat& color)
	{	
		mFogColor->setValue(color);
	}

	void SelectPresetComponentInstance::setFogSettings(glm::vec4& fogSettings)
	{
		((ParameterFloat*)mFogGroup->mParameters[3]->get_ptr())->setValue(fogSettings[0]); //fog min
		((ParameterFloat*)mFogGroup->mParameters[4]->get_ptr())->setValue(fogSettings[1]); //fog max
		((ParameterFloat*)mFogGroup->mParameters[5]->get_ptr())->setValue(fogSettings[2]); //fog influence
		((ParameterFloat*)mFogGroup->mParameters[6]->get_ptr())->setValue(fogSettings[3]); //fog power
	}

	glm::vec4 SelectPresetComponentInstance::getFogSettings()
	{
		return glm::vec4(((ParameterFloat*)mFogGroup->mParameters[3]->get_ptr())->mValue,   //fog min
			((ParameterFloat*)mFogGroup->mParameters[4]->get_ptr())->mValue,				//fog max
			((ParameterFloat*)mFogGroup->mParameters[5]->get_ptr())->mValue,				//fog influence
			((ParameterFloat*)mFogGroup->mParameters[6]->get_ptr())->mValue);				//fog power
	}

	void SelectPresetComponentInstance::onPresetLoaded(std::string& presetFile)
	{
		//reveal the next preset:
		startTransition(PresetSwitchTransitionState::REVEAL_NEXT);
	}
}
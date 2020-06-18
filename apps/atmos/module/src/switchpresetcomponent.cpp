#include "switchpresetcomponent.h"

#include <nap/logger.h>
#include <entity.h>
#include <nap/core.h>
#include <algorithm>
#include <nap/resourcemanager.h>
#include <scene.h>

// nap::SwitchPresetComponent run time class definition
RTTI_BEGIN_CLASS(nap::SwitchPresetComponent)
	RTTI_PROPERTY("PresetParameterGroup", &nap::SwitchPresetComponent::mPresetParameterGroup, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("FogParameterGroup", &nap::SwitchPresetComponent::mFogParameterGroup, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("PresetIndex", &nap::SwitchPresetComponent::mPresetIndex, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("FadeColor", &nap::SwitchPresetComponent::mFadeColor, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::SwitchPresetComponentInstance run time class definition
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SwitchPresetComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void SwitchPresetComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{

	}


	bool SwitchPresetComponentInstance::init(utility::ErrorState& errorState)
	{
		nap::SwitchPresetComponent* resource = getComponent<SwitchPresetComponent>();
		
		ResourceManager* resourceManager = this->getEntityInstance()->getCore()->getResourceManager();
	
		mPresetGroup = resource->mPresetParameterGroup;
		mFogGroup = resource->mFogParameterGroup;
		mPresetIndex = -1;

		mParameterService = this->getEntityInstance()->getCore()->getService<nap::ParameterService>();
		mPresets = mParameterService->getPresets(*mPresetGroup);

		mFadeColor = resource->mFadeColor;

		if (!errorState.check(mPresets.size() > 0, "No presets available"))
			return false;

		selectPresetByIndex(resource->mPresetIndex, 0); // Force a transition to the index specified in the resource on initialization

		return true;
	}

	void SwitchPresetComponentInstance::selectPresetByIndex(unsigned int index, float animationDuration)
	{
	    assert(index < mPresets.size());

	    if (mPresetIndex != index)
        {
	        mPresetIndex = index;
            nap::Logger::debug("preset index: %i...", mPresetIndex);
            std::string presetName = mPresets[mPresetIndex];
            transitionToPreset(presetName, animationDuration);
        }
	}

	void SwitchPresetComponentInstance::transitionToPreset(const std::string& presetPath, float animationDuration)
	{
		mNextPreset = presetPath;
		mAnimationDuration = animationDuration;
		startTransition(PresetSwitchTransitionState::FADE_OUT_CURRENT);
	}

	void SwitchPresetComponentInstance::loadPreset(const std::string& presetPath)
	{
		nap::Logger::debug("loading preset: %s...", presetPath.c_str());
		mParameterService->presetLoaded.connect(mPresetLoaded);
	
		utility::ErrorState presetLoadError;
		if (!mParameterService->loadPreset(*mPresetGroup, presetPath, presetLoadError))
		{
			nap::Logger::fatal("error: %s", presetLoadError.toString().c_str());
		}
	}

	void SwitchPresetComponentInstance::update(double deltaTime)
	{
        updatePresetTransition(deltaTime);
	}

	void SwitchPresetComponentInstance::updatePresetTransition(double deltaTime)
	{
		float lerpProgress = 0;
		switch (mPresetSwitchAnimationState)
		{
		case NONE:
			break;
		case PresetSwitchTransitionState::FADE_OUT_CURRENT:
			if (mAnimationStart)
			{
				mAnimationTime = 0.0f;
				mAnimationStart = false;
			}
			else
			{
				mAnimationTime = mAnimationTime + deltaTime;
			}
			
			lerpProgress = mAnimationTime / mAnimationDuration;
			updateFogFade(lerpProgress);

			if (mAnimationTime >= mAnimationDuration)
			{
				mAnimationStart = true;
				mPresetSwitchAnimationState = LOAD_NEXT;
			}
			break;
		case PresetSwitchTransitionState::LOAD_NEXT:
			mPresetSwitchAnimationState = WAIT_FOR_LOAD;
			loadPreset(mNextPreset);
			break;
		case  PresetSwitchTransitionState::WAIT_FOR_LOAD:
			//waiting on preset loaded to come back. load is blocking so we never get here...
			break;
		case PresetSwitchTransitionState::REVEAL_NEXT:
			if (mAnimationStart)
			{
				mAnimationTime = 0.0f;
				mAnimationStart = false;
			}
			else
			{
				mAnimationTime = mAnimationTime + deltaTime;
			}
			lerpProgress = mAnimationTime / mAnimationDuration;
			lerpProgress = 1.0 - lerpProgress;
			updateFogFade(lerpProgress);

			if (mAnimationTime >= mAnimationDuration)
			{
				mPresetSwitchAnimationState = NONE;
				mAnimationStart = true;
			}
				
			break;
		default:
			break;
		}
	}

	void SwitchPresetComponentInstance::startTransition(const PresetSwitchTransitionState& presetSwitchTransitionState) {
		
		if (mPresetSwitchAnimationState == presetSwitchTransitionState)
			return;

		mPresetSwitchAnimationState = presetSwitchTransitionState;
		mCurrentColor = getFogColor();
		mFogSettingsStart = getFogSettings();
		mFogSettingsEnd = glm::vec4(0, 1, 0, 1);
		mAnimationTime = 0;

		switch (mPresetSwitchAnimationState) {
			case FADE_OUT_CURRENT:
				updateFogFade(0);
				fadeOutTransitionStarted(mAnimationDuration);
				break;
			case REVEAL_NEXT:
				updateFogFade(1);
				revealTransitionStarted(mAnimationDuration);
				break;
			default:
				nap::Logger::error("startTransition - Only FADE_OUT_CURRENT and START_REVEAL states are supported");
				break;
		}
	}

	void SwitchPresetComponentInstance::updateFogFade(double fadeProgress)
	{
		fadeProgress = nap::math::clamp(fadeProgress, 0.0, 1.0);

		glm::vec3 colorVec = nap::math::lerp<glm::vec3>(mCurrentColor.toVec3(), mFadeColor.toVec3(), fadeProgress);
		RGBColorFloat color = RGBColorFloat(colorVec.r, colorVec.g, colorVec.b);
		setFogColor(color);

		glm::vec4 fogSettings = nap::math::lerp(mFogSettingsStart, mFogSettingsEnd, fadeProgress);
		setFogSettings(fogSettings);
	}

	RGBColorFloat& SwitchPresetComponentInstance::getFogColor()
	{
		//improvement: could reference individual parameters instead of group for clarity and error sensitivity..
		return ((ParameterRGBColorFloat*)mFogGroup->mParameters[1]->get_ptr())->mValue;
	}

	void SwitchPresetComponentInstance::setFogColor(RGBColorFloat& color)
	{	
		//improvement: could reference individual parameters instead of group for clarity and error sensitivity..
		((ParameterRGBColorFloat*)mFogGroup->mParameters[1]->get_ptr())->setValue(color);
	}

	void SwitchPresetComponentInstance::setFogSettings(glm::vec4& fogSettings)
	{
		//improvement: could reference individual parameters instead of group for clarity and error sensitivity..

		((ParameterFloat*)mFogGroup->mParameters[3]->get_ptr())->setValue(fogSettings[0]); //fog min
		((ParameterFloat*)mFogGroup->mParameters[4]->get_ptr())->setValue(fogSettings[1]); //fog max
		((ParameterFloat*)mFogGroup->mParameters[5]->get_ptr())->setValue(fogSettings[2]); //fog influence
		((ParameterFloat*)mFogGroup->mParameters[6]->get_ptr())->setValue(fogSettings[3]); //fog power
	}

	glm::vec4 SwitchPresetComponentInstance::getFogSettings()
	{	
		//improvement: could reference individual parameters instead of group for clarity and error sensitivity
		
		return glm::vec4(((ParameterFloat*)mFogGroup->mParameters[3]->get_ptr())->mValue,   //fog min
			((ParameterFloat*)mFogGroup->mParameters[4]->get_ptr())->mValue,				//fog max
			((ParameterFloat*)mFogGroup->mParameters[5]->get_ptr())->mValue,				//fog influence
			((ParameterFloat*)mFogGroup->mParameters[6]->get_ptr())->mValue);				//fog power
	}

	void SwitchPresetComponentInstance::onPresetLoaded(const std::string& presetFile)
	{
		startTransition(PresetSwitchTransitionState::REVEAL_NEXT);
	}
}
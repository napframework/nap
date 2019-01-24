#include "lightingmodecomponent.h"

// External Includes
#include <mathutils.h>

// nap::LightingModeComponent run time class definition 
RTTI_BEGIN_CLASS(nap::LightingModeComponent)
	RTTI_PROPERTY("UpdateMaterialComponent", &nap::LightingModeComponent::mUpdateMaterialComponent, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("SelectVideoComponent", &nap::LightingModeComponent::mSelectVideoComponent, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("OffColorTexture", &nap::LightingModeComponent::mOffColorTexture, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("SunColorTexture", &nap::LightingModeComponent::mSunColorTexture, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("VideoColorTexture", &nap::LightingModeComponent::mVideoColorTexture, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("StaticColorTexture", &nap::LightingModeComponent::mStaticColorTexture, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("PartyColorTexture", &nap::LightingModeComponent::mPartyColorTexture, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("SoundColorTexture", &nap::LightingModeComponent::mSoundColorTexture, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::LightingModeComponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::LightingModeComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void LightingModeComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{

	}


	bool LightingModeComponentInstance::init(utility::ErrorState& errorState)
	{
		// Get references to the lighting mode textures
		mOffColorTexture = getComponent<LightingModeComponent>()->mOffColorTexture.get();
		mSunColorTexture = getComponent<LightingModeComponent>()->mSunColorTexture.get();
		mVideoColorTexture = getComponent<LightingModeComponent>()->mVideoColorTexture.get();
		mStaticColorTexture = getComponent<LightingModeComponent>()->mStaticColorTexture.get();
		mPartyColorTexture = getComponent<LightingModeComponent>()->mPartyColorTexture.get();
		mSoundColorTexture = getComponent<LightingModeComponent>()->mSoundColorTexture.get();

		// Transition from black to sun mode when the app starts
		mLightingModeInt = static_cast<int>(LightingModes::Sun);
		startLightingModeTransition();

		return true;
	}


	void LightingModeComponentInstance::update(double deltaTime)
	{
		// Update lighting mode transition if active
		if (mLightingModeTransitionActive)
			updateLightingModeTransition(deltaTime);	
	}


	void LightingModeComponentInstance::startLightingModeTransition()
	{
		// do nothing if the lighting mode didn't actually change
		LightingModes newLightingModeEnum = static_cast<LightingModes>(mLightingModeInt);
		if (mLightingModeEnum == newLightingModeEnum) return;

		// Store the previous and current lighing mode
		mOldLightingModeEnum = mLightingModeEnum;
		mLightingModeEnum = newLightingModeEnum;

		// Set lighting mode transition properties
		mLightingModeTransitionActive = true;
		mLightingModeTransitionVelocity = 0.0f;

		// Set combination plane uniform values at transition start
		mUpdateMaterialComponent->setCombinationTextures(*getTextureForLightingMode(mOldLightingModeEnum), *getTextureForLightingMode(mLightingModeEnum));
		*mUpdateMaterialComponent->getCombinationBlendValuePtr() = 0.0f;

		// Start / stop the current video depending on the lighting mode
		if (mOldLightingModeEnum == LightingModes::Video)
			mSelectVideoComponent->stopCurrentVideo();
		
		if (mLightingModeEnum == LightingModes::Video)
			mSelectVideoComponent->playCurrentVideo();
	}


	void LightingModeComponentInstance::updateLightingModeTransition(double deltaTime)
	{
		// Smoothly increment the blend uniform value in the combination plane
		float* blendValue = mUpdateMaterialComponent->getCombinationBlendValuePtr();
		nap::math::smooth(*blendValue, 1.0f, mLightingModeTransitionVelocity, static_cast<float>(deltaTime), 0.5f, 1000.0f);

		// Stop the updating when the transition is complete
		if (*blendValue >= 1.0f)
		{
			// reset the old lighting mode so we don't render
			// the old mode unnecessarily after the crossfade
			mOldLightingModeEnum = LightingModes::Off;
			mLightingModeTransitionActive = false;
		}
	}


	bool LightingModeComponentInstance::isLightingModeSelected(LightingModes lightingMode)
	{
		return mLightingModeEnum == lightingMode;
	}


	bool LightingModeComponentInstance::isLightingModeRendered(LightingModes lightingMode)
	{
		return mLightingModeEnum == lightingMode || mOldLightingModeEnum == lightingMode;
	}


	nap::Texture2D* LightingModeComponentInstance::getTextureForLightingMode(LightingModes& lightingMode)
	{
		switch (lightingMode)
		{
		case LightingModes::Sun:
			return mSunColorTexture;
		case LightingModes::Video:
			return mVideoColorTexture;
		case LightingModes::Static:
			return mStaticColorTexture;
		case LightingModes::Party:
			return mPartyColorTexture;
		case LightingModes::Sound:
			return mSoundColorTexture;
		default:
			return mOffColorTexture;
		}
	}
}

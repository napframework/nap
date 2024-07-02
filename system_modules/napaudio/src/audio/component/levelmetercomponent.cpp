/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "levelmetercomponent.h"

// Nap includes
#include <entity.h>
#include <nap/core.h>

// Audio includes
#include <audio/service/audioservice.h>

// RTTI
RTTI_BEGIN_CLASS(nap::audio::LevelMeterComponent, "Measures the amplitude level of the audio signal")
		RTTI_PROPERTY("Input", &nap::audio::LevelMeterComponent::mInput, nap::rtti::EPropertyMetaData::Required, "Audio component to analyze")
		RTTI_PROPERTY("AnalysisWindowSize", &nap::audio::LevelMeterComponent::mAnalysisWindowSize, nap::rtti::EPropertyMetaData::Default, "Size of the analysis window in ms")
		RTTI_PROPERTY("MeterType", &nap::audio::LevelMeterComponent::mMeterType, nap::rtti::EPropertyMetaData::Default, "Used analysis method: Root mean square (RMS) or peak amplitude (PEAK)")
		RTTI_PROPERTY("FilterInput", &nap::audio::LevelMeterComponent::mFilterInput, nap::rtti::EPropertyMetaData::Default, "Filter input before analysis")
		RTTI_PROPERTY("CenterFrequency", &nap::audio::LevelMeterComponent::mCenterFrequency, nap::rtti::EPropertyMetaData::Default, "Filter frequency band (hz)")
		RTTI_PROPERTY("BandWidth", &nap::audio::LevelMeterComponent::mBandWidth, nap::rtti::EPropertyMetaData::Default, "Filter frequency band width (hz)")
		RTTI_PROPERTY("Channel", &nap::audio::LevelMeterComponent::mChannel, nap::rtti::EPropertyMetaData::Default, "Input channel to analyze")
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::LevelMeterComponentInstance)
		RTTI_CONSTRUCTOR(nap::EntityInstance &, nap::Component &)
		RTTI_FUNCTION("getLevel", &nap::audio::LevelMeterComponentInstance::getLevel)
RTTI_END_CLASS

namespace nap
{
	
	namespace audio
	{
		
		bool LevelMeterComponentInstance::init(utility::ErrorState& errorState)
		{
			mResource = getComponent<LevelMeterComponent>();
			mAudioService = getEntityInstance()->getCore()->getService<AudioService>();
			auto& nodeManager = mAudioService->getNodeManager();
			
			mCenterFrequency = mResource->mCenterFrequency;
			mBandWidth = mResource->mBandWidth;
			mFilterGain = mResource->mFilterGain;
			
			if (!errorState.check(mResource->mChannel < mInput->getChannelCount(),
			                      "%s: Channel exceeds number of input channels", mResource->mID.c_str()))
				return false;
			
			mMeter = nodeManager.makeSafe<LevelMeterNode>(nodeManager);
			
			if (mResource->mFilterInput)
			{
				mFilter = nodeManager.makeSafe<FilterNode>(nodeManager);
				mFilter->setMode(FilterNode::EMode::BandPass);
				mFilter->setFrequency(mResource->mCenterFrequency);
				mFilter->setGain(mResource->mFilterGain);
				mFilter->setBand(mBandWidth);
				mFilter->audioInput.connect(*mInput->getOutputForChannel(mResource->mChannel));
				mMeter->input.connect(mFilter->audioOutput);
			}
			else {
				mMeter->input.connect(*mInput->getOutputForChannel(mResource->mChannel));
			}
			
			return true;
		}
		
		
		ControllerValue LevelMeterComponentInstance::getLevel()
		{
			return mMeter->getLevel();
		}
		
		
		void LevelMeterComponentInstance::setCenterFrequency(ControllerValue centerFrequency)
		{
			mCenterFrequency = centerFrequency;
			mFilter->setFrequency(mCenterFrequency);
		}
		
		
		void LevelMeterComponentInstance::setBandWidth(ControllerValue bandWidth)
		{
			mBandWidth = bandWidth;
			mFilter->setBand(mBandWidth);
		}
		
		
		void LevelMeterComponentInstance::setFilterGain(ControllerValue gain)
		{
			mFilterGain = gain;
			mFilter->setGain(mFilterGain);
		}
		
		
		ControllerValue LevelMeterComponentInstance::getCenterFrequency() const
		{
			return mCenterFrequency;
		}
		
		
		ControllerValue LevelMeterComponentInstance::getBandWidth() const
		{
			return mBandWidth;
		}
		
		
		ControllerValue LevelMeterComponentInstance::getFilterGain() const
		{
			return mFilterGain;
		}
		
		
		void LevelMeterComponentInstance::setInput(AudioComponentBaseInstance& input)
		{
			auto inputPtr = &input;
			if (mResource->mFilterInput)
				mAudioService->enqueueTask([&, inputPtr]() {
					mFilter->audioInput.connect(*inputPtr->getOutputForChannel(mResource->mChannel));
				});
			else
				mAudioService->enqueueTask([&, inputPtr]() {
					mMeter->input.connect(*inputPtr->getOutputForChannel(mResource->mChannel));
				});
		}
		
	}
	
}

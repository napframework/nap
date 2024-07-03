/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "levelmeternode.h"

#include <audio/core/audionodemanager.h>
#include <cmath>

RTTI_BEGIN_ENUM(nap::audio::LevelMeterNode::Type)
	RTTI_ENUM_VALUE(nap::audio::LevelMeterNode::Type::RMS, "RMS"),
	RTTI_ENUM_VALUE(nap::audio::LevelMeterNode::Type::PEAK, "Peak")
RTTI_END_ENUM

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::LevelMeterNode)
	RTTI_PROPERTY("input", &nap::audio::LevelMeterNode::input, nap::rtti::EPropertyMetaData::Embedded)
	RTTI_FUNCTION("getLevel", &nap::audio::LevelMeterNode::getLevel)
RTTI_END_CLASS


namespace nap
{
	namespace audio
	{

		LevelMeterNode::LevelMeterNode(NodeManager& nodeManager, TimeValue analysisWindowSize, bool rootProcess) : Node(nodeManager), mRootProcess(rootProcess), mAnalysisWindowSize(analysisWindowSize)
		{
			mWindowSizeInSamples = getNodeManager().getSamplesPerMillisecond() * mAnalysisWindowSize;
			mSquaredBuffer.resize(mWindowSizeInSamples);
            
			if (rootProcess)
				getNodeManager().registerRootProcess(*this);
		}


		LevelMeterNode::~LevelMeterNode()
		{
			if (mRootProcess)
				getNodeManager().unregisterRootProcess(*this);
		}


		float LevelMeterNode::getLevel()
		{
			return mValue.load();
		}


		void LevelMeterNode::process()
		{
			auto inputBuffer = input.pull();

			if (inputBuffer == nullptr)
				return;
            
			switch (mType) {
				case PEAK:

					for (auto& sample : *inputBuffer) {
						
						// keep track of peak, sample by sample
						float newValue = fabs(sample);
						if (newValue > mPeakTemp)
							mPeakTemp = newValue;
						
						// reset temp and update the output value once every 'window'
						mIndex++;
						if(mIndex == mWindowSizeInSamples)
						{
							mPeak = mPeakTemp;
							mPeakTemp = 0.f;
							mIndex = 0;
						}
						
					}
					
					mValue.store(mPeak);

					break;

				default: // RMS

					for (auto& sample : *inputBuffer) {
						
						// updated squared sum
						mSquaredSum -= mSquaredBuffer[mIndex];
						mSquaredBuffer[mIndex] = sample * sample;
						mSquaredSum += mSquaredBuffer[mIndex];
						
						// increment index
						mIndex++;
						if (mIndex == mSquaredBuffer.size())
							mIndex = 0;
						
					}
					
					mValue.store(mSquaredSum / (float)mSquaredBuffer.size());
					
					break;
					
			}

		}

		void LevelMeterNode::sampleRateChanged(float sampleRate)
		{
			mWindowSizeInSamples = getNodeManager().getSamplesPerMillisecond() * mAnalysisWindowSize;
			mSquaredBuffer.resize(mWindowSizeInSamples);
		}

	}
}

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
			mBuffer.resize(getNodeManager().getSamplesPerMillisecond() * mAnalysisWindowSize);
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
			if (mDirty.check()) {
				switch (mType) {
					case PEAK:
						mValue = calculatePeak();
						break;

					default:
						mValue = calculateRms();
						break;
				}
			}
			return mValue;
		}


		void LevelMeterNode::process()
		{
			auto inputBuffer = input.pull();

			if (inputBuffer == nullptr)
				return;

			for (auto& sample : *inputBuffer) {
				mBuffer[mIndex++] = sample;
				if (mIndex == mBuffer.size()) {
					mIndex = 0;
					mDirty.set();
				}
			}
		}
		

		void LevelMeterNode::sampleRateChanged(float sampleRate)
		{
			mBuffer.resize(getNodeManager().getSamplesPerMillisecond() * mAnalysisWindowSize);
		}


		float LevelMeterNode::calculateRms()
		{
			float x = 0;
			for (auto i = 0; i < mBuffer.size(); ++i)
				x += mBuffer[i] * mBuffer[i];
			x /= float(mBuffer.size());

			return sqrt(x);
		}


		float LevelMeterNode::calculatePeak()
		{
			float x = 0;
			for (auto i = 0; i < mBuffer.size(); ++i) {
				float newValue = fabs(mBuffer[i]);
				if (newValue > x)
					x = newValue;
			}
			return x;
		}

	}
}

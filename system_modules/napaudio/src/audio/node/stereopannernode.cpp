/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "stereopannernode.h"
#include <audio/utility/audiofunctions.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::StereoPannerNode)
	RTTI_PROPERTY("leftInput", &nap::audio::StereoPannerNode::leftInput, nap::rtti::EPropertyMetaData::Embedded)
	RTTI_PROPERTY("rightInput", &nap::audio::StereoPannerNode::rightInput, nap::rtti::EPropertyMetaData::Embedded)
	RTTI_PROPERTY("leftOutput", &nap::audio::StereoPannerNode::leftOutput, nap::rtti::EPropertyMetaData::Embedded)
	RTTI_PROPERTY("rightOutput", &nap::audio::StereoPannerNode::rightOutput, nap::rtti::EPropertyMetaData::Embedded)
	RTTI_FUNCTION("setPanning", &nap::audio::StereoPannerNode::setPanning)
RTTI_END_CLASS

namespace nap
{
	namespace audio
	{
		
		StereoPannerNode::StereoPannerNode(NodeManager& manager) : Node(manager)
		{
			equalPowerPan<ControllerValue>(mPanning, mLeftGain, mRightGain);
		}
		
		
		void StereoPannerNode::setPanning(ControllerValue value)
		{
			mNewPanning = value;
		}
		
		
		void StereoPannerNode::process()
		{
			auto newPanning = mNewPanning.load();
			if (newPanning != mPanning) {
				mPanning = newPanning;
				equalPowerPan<ControllerValue>(mPanning, mLeftGain, mRightGain);
			}
			
			auto& leftInputBuffer = *leftInput.pull();
			auto& rightInputBuffer = *rightInput.pull();
			auto& leftOutputBuffer = getOutputBuffer(leftOutput);
			auto& rightOutputBuffer = getOutputBuffer(rightOutput);
			
			for (auto i = 0; i < leftOutputBuffer.size(); ++i)
				leftOutputBuffer[i] = leftInputBuffer[i] * mLeftGain;
			
			for (auto i = 0; i < rightOutputBuffer.size(); ++i)
				rightOutputBuffer[i] = rightInputBuffer[i] * mRightGain;
		}
		
	}
}

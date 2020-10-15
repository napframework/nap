/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "gainnode.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::GainNode)
	RTTI_PROPERTY("input", &nap::audio::GainNode::audioInput, nap::rtti::EPropertyMetaData::Embedded)
	RTTI_PROPERTY("audioOutput", &nap::audio::GainNode::audioOutput, nap::rtti::EPropertyMetaData::Embedded)
	RTTI_FUNCTION("setGain", &nap::audio::GainNode::setGain)
	RTTI_FUNCTION("getGain", &nap::audio::GainNode::getGain)
RTTI_END_CLASS


namespace nap
{
	namespace audio
	{
		
		void GainNode::process()
		{
			auto& outputBuffer = getOutputBuffer(audioOutput);
			auto inputBuffer = audioInput.pull();
			
			if (inputBuffer == nullptr) {
				for (auto i = 0; i < outputBuffer.size(); ++i)
					outputBuffer[i] = 0;
				return;
			}
			
			for (auto i = 0; i < outputBuffer.size(); ++i)
				outputBuffer[i] = (*inputBuffer)[i] * mGain.getNextValue();
		}
		
		
		void GainNode::setGain(ControllerValue gain, TimeValue smoothTime)
		{
			mGain.setStepCount(smoothTime * getNodeManager().getSamplesPerMillisecond());
			mGain.setValue(gain);
		}
		
		
	}
}

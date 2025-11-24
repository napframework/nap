/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "bufferplayernode.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::BufferPlayerNode)
		RTTI_PROPERTY("audioOutput", &nap::audio::BufferPlayerNode::audioOutput, nap::rtti::EPropertyMetaData::Embedded)
		RTTI_FUNCTION("play", &nap::audio::BufferPlayerNode::play)
		RTTI_FUNCTION("stop", &nap::audio::BufferPlayerNode::stop)
		RTTI_FUNCTION("setChannel", &nap::audio::BufferPlayerNode::setChannel)
		RTTI_FUNCTION("setSpeed", &nap::audio::BufferPlayerNode::setSpeed)
		RTTI_FUNCTION("setPosition", &nap::audio::BufferPlayerNode::setPosition)
RTTI_END_CLASS

namespace nap
{
	
	namespace audio
	{
		
		void BufferPlayerNode::play(int channel, DiscreteTimeValue position, ControllerValue speed)
		{
			mPlaying = true;
			mChannel = channel;
			mPosition = position;
			mSpeed = speed;
		}
		
		
		void BufferPlayerNode::stop()
		{
			mPlaying = false;
		}
		
		
		void BufferPlayerNode::setChannel(int channel)
		{
			mChannel = channel;
		}
		
		
		void BufferPlayerNode::setSpeed(ControllerValue speed)
		{
			mSpeed = speed;
		}
		
		
		void BufferPlayerNode::setPosition(DiscreteTimeValue position)
		{
			mPosition = position;
		}
		
		
		void BufferPlayerNode::setBuffer(SafePtr<MultiSampleBuffer> buffer)
		{
			assert(mPlaying == false); // It is not safe to do this while playing back!
			mBuffer = std::move(buffer);
		}
		
		
		void BufferPlayerNode::process()
		{
			auto& outputBuffer = getOutputBuffer(audioOutput);

			auto playing = mPlaying.load();
			auto channel = mChannel.load();
			auto position = mPosition.load();
			auto speed = mSpeed.load();
			
			// If we're not playing, fill the buffer with 0's and bail out.
			if (!playing || mBuffer == nullptr || channel >= mBuffer->getChannelCount()) {
				std::memset(outputBuffer.data(), 0, sizeof(SampleValue) * outputBuffer.size());
				return;
			}
			
			DiscreteTimeValue flooredPosition;
			SampleValue lastValue, newValue, fractionalPart;
			SampleBuffer& channelBuffer = (*mBuffer)[channel];
			
			// For each sample
			for (auto i = 0; i < outputBuffer.size(); i++) {
				// Have we reached the destination?
				if (position + 1 >= channelBuffer.size()) {
					outputBuffer[i] = 0;
					if (playing)
						playing = false;
				} else {
					flooredPosition = DiscreteTimeValue(position);
					lastValue = channelBuffer[flooredPosition];
					newValue = channelBuffer[flooredPosition + 1];
					
					fractionalPart = position - flooredPosition;
					
					outputBuffer[i] = lastValue + (fractionalPart * (newValue - lastValue));
					
					position += speed;
				}
			}
			
			mPosition.store(position);
			mPlaying.store(playing);
		}
		
	}
	
}

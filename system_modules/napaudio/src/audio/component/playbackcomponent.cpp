/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "playbackcomponent.h"

// Nap includes
#include <entity.h>
#include <nap/core.h>

// Audio includes
#include <audio/service/audioservice.h>
#include <mathutils.h>

// RTTI
RTTI_BEGIN_CLASS(nap::audio::PlaybackComponent, "Plays-back audio from an audio buffer resource")
	RTTI_PROPERTY("Buffer", &nap::audio::PlaybackComponent::mBuffer, nap::rtti::EPropertyMetaData::Required, "The buffer to play")
	RTTI_PROPERTY("ChannelRouting", &nap::audio::PlaybackComponent::mChannelRouting, nap::rtti::EPropertyMetaData::Default, "Channel selection, auto completed using buffer when left empty")
	RTTI_PROPERTY("Gain", &nap::audio::PlaybackComponent::mGain, nap::rtti::EPropertyMetaData::Default, "Overall input gain")
	RTTI_PROPERTY("StereoPanning", &nap::audio::PlaybackComponent::mStereoPanning, nap::rtti::EPropertyMetaData::Default, "Stereo field panning, only works with 2 channels: 0.0 = left, 1.0 = right & 0.5 = center")
	RTTI_PROPERTY("AutoPlay", &nap::audio::PlaybackComponent::mAutoPlay, nap::rtti::EPropertyMetaData::Default, "Start playback on initialization")
	RTTI_PROPERTY("StartPosition", &nap::audio::PlaybackComponent::mStartPosition, nap::rtti::EPropertyMetaData::Default, "Playback start position in ms")
	RTTI_PROPERTY("Pitch", &nap::audio::PlaybackComponent::mPitch, nap::rtti::EPropertyMetaData::Default, "Audio pitch as fractiion of original, ie: 2.0 = double speed, 0.5 = half speed")
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::PlaybackComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance &, nap::Component &)
	RTTI_FUNCTION("start", (void (nap::audio::PlaybackComponentInstance::*)(double))& nap::audio::PlaybackComponentInstance::start)
	RTTI_FUNCTION("stop", &nap::audio::PlaybackComponentInstance::stop)
	RTTI_FUNCTION("setGain", &nap::audio::PlaybackComponentInstance::setGain)
	RTTI_FUNCTION("setStereoPanning", &nap::audio::PlaybackComponentInstance::setStereoPanning)
	RTTI_FUNCTION("isStereo", &nap::audio::PlaybackComponentInstance::isStereo)
	RTTI_FUNCTION("isPlaying", &nap::audio::PlaybackComponentInstance::isPlaying)
	RTTI_FUNCTION("getGain", &nap::audio::PlaybackComponentInstance::getGain)
	RTTI_FUNCTION("getStereoPanning", &nap::audio::PlaybackComponentInstance::getStereoPanning)
RTTI_END_CLASS

namespace nap
{
	
	namespace audio
	{
		bool PlaybackComponentInstance::init(utility::ErrorState& errorState)
		{
			// Ensure sample rate is valid
			mResource = getComponent<PlaybackComponent>();
			auto sample_rate = mResource->mBuffer->getSampleRate();
			if (!errorState.check(!math::equal<float>(sample_rate, 0.0f),
				"Invalid buffer sample rate"))
				return false;

			// Copy from resource
			mGain = mResource->mGain;
			mStereoPanning = mResource->mStereoPanning;
			mPitch = mResource->mPitch;
			mLength = mResource->mBuffer->getSize() / static_cast<double>(sample_rate);

			// Get managers
			mAudioService = getEntityInstance()->getCore()->getService<AudioService>();
			mNodeManager = &mAudioService->getNodeManager();

			// If channel routing is left empty, fill it with the channels in the buffer in ascending order.
			mChannelRouting = mResource->mChannelRouting;
			if (mChannelRouting.empty())
			{
				for (auto channel = 0; channel < mResource->mBuffer->getChannelCount(); ++channel) {
					mChannelRouting.emplace_back(channel);
				}
			}

			mChannelGains.resize(mChannelRouting.size(), 1.f);
			for (auto channel = 0; channel < mChannelRouting.size(); ++channel)
			{
				// Ensure requested is within buffer channel bounds
				if (mChannelRouting[channel] >= mResource->mBuffer->getChannelCount())
				{
					errorState.fail("%s: Routed channel is out of buffer's channel bounds", mResource->mID.c_str());
					return false;
				}

				// Create the playback node
				auto bufferPlayer = mNodeManager->makeSafe<BufferPlayerNode>(*mNodeManager);
				bufferPlayer->setBuffer(mResource->mBuffer->getBuffer());

				// Create gain
				auto gain = mNodeManager->makeSafe<MultiplyNode>(*mNodeManager);
				auto gainControl = mNodeManager->makeSafe<ControlNode>(*mNodeManager);

				// Multiply output of buffer with gain
				gain->inputs.connect(bufferPlayer->audioOutput);
				gain->inputs.connect(gainControl->output);
				gainControl->setValue(0);

				// Stop buffer playback when gain reaches 0
				SafePtr<BufferPlayerNode> bufferPlayerPtr = bufferPlayer.get();
				gainControl->rampFinishedSignal.connect([&, bufferPlayerPtr](ControlNode& gainControl)
					{
						if (gainControl.getValue() <= 0) {
							bufferPlayerPtr->stop();
						}
					});
				
				mBufferPlayers.emplace_back(std::move(bufferPlayer));
				mGainNodes.emplace_back(std::move(gain));
				mGainControls.emplace_back(std::move(gainControl));
			}

			// Start playback if requested
			if (mResource->mAutoPlay)
				start(mResource->mStartPosition / 1000.0);

			return true;
		}
		
		
		void PlaybackComponentInstance::update(double deltaTime)
		{
			if (isPlaying())
			{
				mPlaytime += deltaTime;
			}
		}
		
		
		void PlaybackComponentInstance::stop()
		{
			for (auto& gainControl : mGainControls)
				gainControl->ramp(0, 1, RampMode::Linear);
		}
		
		
		void PlaybackComponentInstance::setGain(ControllerValue gain)
		{
			if (!math::equal(gain, mGain))
			{
				mGain = gain;
				applyGain();
			}
		}
		
		
		void PlaybackComponentInstance::setChannelGain(int channel, ControllerValue gain)
		{
			// Make sure the channel index is in bounds.
			assert(channel < mGainControls.size());
			mChannelGains[channel] = gain;
			applyGain();
		}
		
		
		void PlaybackComponentInstance::setStereoPanning(ControllerValue panning)
		{
			if (!math::equal(panning, mStereoPanning))
			{
				mStereoPanning = panning;
				applyGain();
			}
		}
		
		
		void PlaybackComponentInstance::setPitch(ControllerValue pitch)
		{
			if (!math::equal(pitch, mPitch))
			{
				mPitch = pitch;
				for (auto& bufferPlayer : mBufferPlayers) {
					bufferPlayer->setSpeed(getSpeed());
				}
			}
		}
		
		
		bool PlaybackComponentInstance::isPlaying() const
		{
			for (auto& player : mBufferPlayers)
			{
				if (!player->getPlaying())
					return false;
			}
			return true;
		}


		bool PlaybackComponentInstance::isFinished() const
		{
			return getSamplePosition() == getSampleCount() - 1;
		}


		float PlaybackComponentInstance::getSpeed() const
		{
			return mPitch * (mResource->mBuffer->getSampleRate() / mNodeManager->getSampleRate());
		}


		double PlaybackComponentInstance::getPosition(int c) const
		{
			auto i = getBufferIndex(c); assert(i >= 0);
			return mBufferPlayers[i]->getPosition() / static_cast<double>(mResource->mBuffer->getSampleRate());
		}


		double PlaybackComponentInstance::getPosition() const
		{
			assert(!mBufferPlayers.empty());
			return mBufferPlayers[0]->getPosition() / static_cast<double>(mResource->mBuffer->getSampleRate());
		}


		void PlaybackComponentInstance::setPosition(double pos)
		{
			DiscreteTimeValue sp = mResource->mBuffer->getSampleRate() * pos;
			sp = math::clamp<DiscreteTimeValue>(sp, 0, mResource->mBuffer->getSize() - 1);
			setSamplePosition(sp);
		}


		void PlaybackComponentInstance::setSamplePosition(DiscreteTimeValue pos)
		{
			assert(pos < mResource->mBuffer->getSize());
			for (auto& player : mBufferPlayers)
			{
				if (player != nullptr)
				{
					assert(pos < player->getSize());
					player->setPosition(pos);
				}
			}
		}


		DiscreteTimeValue PlaybackComponentInstance::getSamplePosition() const
		{
			assert(!mChannelRouting.empty());
			return mBufferPlayers[0]->getPosition();
		}


		DiscreteTimeValue PlaybackComponentInstance::getSamplePosition(int c) const
		{
			auto i = getBufferIndex(c); assert(i >= 0);
			return mBufferPlayers[i]->getPosition();
		}


		const nap::audio::AudioBufferResource& PlaybackComponentInstance::getBuffer() const
		{
			assert(mResource != nullptr);
			return *mResource->mBuffer;
		}


		const SampleBuffer& PlaybackComponentInstance::getSamples(int c) const
		{
			return getBuffer().getSamples(c);
		}


		void PlaybackComponentInstance::applyGain()
		{
			if (mResource->isStereo())
				equalPowerPan(mStereoPanning, mChannelGains[0], mChannelGains[1]);
			
			for (auto i = 0; i < mGainControls.size(); ++i)
				mGainControls[i]->setValue(mGain * mChannelGains[i]);
		}


		int PlaybackComponentInstance::getBufferIndex(int channel) const
		{
			for (int i = 0; i < mChannelRouting.size(); i++)
			{
				if (mChannelRouting[i] == channel)
					return i;
			}
			return -1;
		}


		void PlaybackComponentInstance::start(double startPosition)
		{
			setPosition(startPosition);
			start();
		}


		void PlaybackComponentInstance::start()
		{
			// Play from current position
			int idx = 0; auto speed = getSpeed();
			for (auto& player : mBufferPlayers)
			{
				if (player != nullptr)
					player->play(mChannelRouting[idx], player->getPosition(), speed);
				idx++;
			}

			// Fade to target in 1 ms
			for (auto i = 0; i < mGainControls.size(); ++i)
				mGainControls[i]->ramp(mGain * mChannelGains[i], 1);

			// Reset playback time
			mPlaytime = 0.0;
		}
	}
}

/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Nap includes
#include <nap/resourceptr.h>
#include <audio/utility/safeptr.h>

// Audio includes
#include <audio/component/audiocomponentbase.h>
#include <audio/resource/audiobufferresource.h>
#include <audio/node/bufferplayernode.h>
#include <audio/node/multiplynode.h>
#include <audio/node/controlnode.h>
#include <audio/node/filternode.h>

namespace nap
{
	namespace audio
	{
		
		// Forward declares
		class AudioService;
		
		class PlaybackComponentInstance;
		
		
		/**
		 * Straightforward component to playback audio from an @AudioBufferResource. Playback can be started on initialization using the AutoPlay property or using the @start() method, and is stopped using the @stop() method or by specifying the "Duration" property.
		 * The component has to be used in combination with an @OutputComponent to send the playback to DAC.
		 */
		class NAPAPI PlaybackComponent : public AudioComponentBase
		{
		RTTI_ENABLE(AudioComponentBase)
		DECLARE_COMPONENT(PlaybackComponent, PlaybackComponentInstance)
		
		public:
			PlaybackComponent() : AudioComponentBase() { }
			
			// Properties
			ResourcePtr<AudioBufferResource> mBuffer = nullptr; ///< property: 'Buffer' The buffer containing the audio to be played back
			std::vector<int> mChannelRouting = { };           ///< property: 'ChannelRouting' The size of this array indicates the number of channels to be played back. Each element indicates a channel number of the buffer to be played. If left empty it will be filled with the channels in the buffer in ascending order.
			bool mAutoPlay = true;                              ///< property: 'AutoPlay' If set to true, the component will start playing on initialization.
			TimeValue mStartPosition = 0;                       ///< property: 'StartPosition' Start position of playback in milliseconds.
			TimeValue mDuration = 0;                            ///< property: 'Duration' Duration of playback in milliseconds.
			TimeValue mFadeInTime = 0;                          ///< property: 'FadeInTime' Fade in time of playback in milliseconds, to prevent clicks.
			TimeValue mFadeOutTime = 0;                         ///< property: 'FadeOutTime' Fade out time of playback in milliseconds, te prevent clicks
			ControllerValue mPitch = 1.0;                       ///< property: 'Pitch' Pitch as a fraction of the original: 2.0 means double speed, 0.5 means halve speed.
			ControllerValue mGain = 1.0;                        ///< property: 'Gain' Overall gain/
			ControllerValue mStereoPanning = 0.5;               ///< property: 'StereoPanning' Panning in the stereo field: 0 means far left, 0.5 means center, 1.0  means far right. This property only applies when two channels are being played back.
			
			/**
			 * Returns if the playback consists of 2 audio channels
			 */
			bool isStereo() const { return mChannelRouting.size() == 2; }
		};
		
		
		/**
		 * Instance of @PlaybackComponent
		 */
		class NAPAPI PlaybackComponentInstance : public AudioComponentBaseInstance
		{
			RTTI_ENABLE(AudioComponentBaseInstance)
		public:
			PlaybackComponentInstance(EntityInstance& entity, Component& resource) : AudioComponentBaseInstance(entity, resource) { }
			
			// Inherited from ComponentInstance
			bool init(utility::ErrorState& errorState) override;
			
			void update(double deltaTime) override;
			
			// Inherited from AudioComponentBaseInstance
			int getChannelCount() const override { return mGainNodes.size(); }
			
			OutputPin* getOutputForChannel(int channel) override { return &mGainNodes[channel]->audioOutput; }
			
			/**
			 * @param startPosition: the start position in the buffer in milliseconds
			 * @param duration: the total duration of playback in milliseconds. 0 means play untill the end of the buffer
			 */
			void start(TimeValue startPosition = 0, TimeValue duration = 0);
			
			/**
			 * Fade out over fade out time and stop playback.
			 */
			void stop();
			
			/**
			 * Sets the overall gain of playback.
			 */
			void setGain(ControllerValue gain);
			
			/**
			 * Sets the gain for a single channel relative to the overall gain.
			 * Use this for manual panning with for example non-stereo channel setups.
			 */
			 void setChannelGain(int channel, ControllerValue gain);
			
			/**
			 * Sets the panning for stereo playback: 0 means far left, 0.5 means center and 1.0 means far right. Only applies when there are 2 channels of playback.
			 */
			void setStereoPanning(ControllerValue panning);
			
			/**
			 * Sets the fade in time used in milliseconds when starting playback.
			 */
			void setFadeInTime(TimeValue time);
			
			/**
			 * Sets the fade out time used in milliseconds when stopping playback.
			 */
			void setFadeOutTime(TimeValue time);
			
			/**
			 * Sets the pitch as a fraction of the original pitch of the audio material in the buffer.
			 */
			void setPitch(ControllerValue pitch);
			
			/**
			 * @return Tells wether the playback is stereo and consists of two channels of audio.
			 */
			bool isStereo() const { return mGainNodes.size() == 2; }
			
			/**
			 * @return true when the component is currently playing back audio.
			 */
			bool isPlaying() const { return mPlaying; }
			
			/**
			 * @return the current gain value
			 */
			ControllerValue getGain() const { return mGain; }
			
			/**
			 * @return the current stereo panning.
			 */
			ControllerValue getStereoPanning() const { return mStereoPanning; }
			
			/**
			 * @return the fade in time in milliseconds used when starting playback
			 */
			ControllerValue getFadeInTime() const { return mFadeInTime; }
			
			/**
			 * @return the fade out time in milliseconds used when stopping playback
			 */
			ControllerValue getFadeOutTime() const { return mFadeOutTime; }
			
			/**
			 * @return the pitch as a fraction of the original pitch of the audio material in the buffer.
			 */
			ControllerValue getPitch() const { return mPitch; }
			
			/**
			 * @return the amount of time the sequencer has been playing since the last call to play().
			 */
			TimeValue getCurrentPlayingTime() const { return mCurrentPlayingTime; }
		
		private:
			void applyGain(TimeValue rampTime);
			
			std::vector<SafeOwner<BufferPlayerNode>> mBufferPlayers; // Nodes for each channel performing the actual audio playback.
			std::vector<SafeOwner<MultiplyNode>> mGainNodes; // Nodes for each channel to gain the signal.
			std::vector<SafeOwner<ControlNode>> mGainControls; // Nodes to control the gain for each channel.
			
			ControllerValue mGain = 0;
			std::vector<ControllerValue> mChannelGains;
			ControllerValue mStereoPanning = 0.5;
			TimeValue mFadeInTime = 0;
			TimeValue mFadeOutTime = 0;
			ControllerValue mPitch = 1.0;
			TimeValue mDuration = 0;
			TimeValue mCurrentPlayingTime = 0;
			std::vector<int> mChannelRouting;
			
			bool mPlaying = false;  // Indicates wether the component is currently playing
			
			PlaybackComponent* mResource = nullptr; // The component's resource
			NodeManager* mNodeManager = nullptr; // The audio node manager this component's audio nodes are managed by
			AudioService* mAudioService = nullptr;
		};
		
	}
	
}

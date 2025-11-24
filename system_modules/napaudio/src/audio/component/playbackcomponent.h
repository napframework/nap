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
		 * Plays-back audio from a nap::audio::AudioBufferResource. Playback can be started on initialization using the AutoPlay property or using the start() method, and is stopped using the stop() method or by specifying the "Duration" property.
		 * The component has to be used in combination with an nap::audio::OutputComponent to send the playback to DAC.
		 */
		class NAPAPI PlaybackComponent : public AudioComponentBase
		{
		RTTI_ENABLE(AudioComponentBase)
		DECLARE_COMPONENT(PlaybackComponent, PlaybackComponentInstance)
		
		public:
			PlaybackComponent() : AudioComponentBase() { }
			
			// Properties
			ResourcePtr<AudioBufferResource> mBuffer = nullptr; ///< property: 'Buffer' Optional buffer containing the audio to be played back
			std::vector<int> mChannelRouting = { };				///< property: 'ChannelRouting' The size of this array indicates the number of channels to be played back. Each element indicates a channel number of the buffer to be played. If left empty it will be filled with the channels in the buffer in ascending order.
			bool mAutoPlay = true;                              ///< property: 'AutoPlay' If set to true, the component will start playing on initialization.
			TimeValue mStartPosition = 0;                       ///< property: 'StartPosition' Start position of playback in milliseconds.
			TimeValue mFadeInTime = 0;                          ///< property: 'FadeInTime' Fade in time of playback in milliseconds, to prevent clicks.
			TimeValue mFadeOutTime = 0;                         ///< property: 'FadeOutTime' Fade out time of playback in milliseconds, to prevent clicks
			ControllerValue mPitch = 1.0;                       ///< property: 'Pitch' Pitch as a fraction of the original: 2.0 means double speed, 0.5 means halve speed.
			ControllerValue mGain = 1.0;                        ///< property: 'Gain' Overall gain
			ControllerValue mStereoPanning = 0.5;               ///< property: 'StereoPanning' Panning in the stereo field: 0 means far left, 0.5 means center, 1.0  means far right. This property only applies when two channels are being played back.
			
			/**
			 * Returns if the playback consists of 2 audio channels
			 */
			bool isStereo() const { return mChannelRouting.size() == 2; }
		};
		
		
		/**
		 * Instance of PlaybackComponent.
		 * Plays back audio from a nap::audio::AudioBufferResource. Playback can be started on initialization using the AutoPlay property or using the start() method, and is stopped using the stop() method or by specifying the "Duration" property.
		 * The component has to be used in combination with an nap::audio::OutputComponent to send the playback to DAC.
		 */
		class NAPAPI PlaybackComponentInstance : public AudioComponentBaseInstance
		{
			RTTI_ENABLE(AudioComponentBaseInstance)
		public:
			PlaybackComponentInstance(EntityInstance& entity, Component& resource) : AudioComponentBaseInstance(entity, resource) { }
			
			// Inherited from ComponentInstance
			bool init(utility::ErrorState& errorState) override;

			// Called every frame
			void update(double deltaTime) override;
			
			/**
			 * @return total number of channels
			 */
			int getChannelCount() const override { return mGainNodes.size(); }

			/**
			 * @return output for given channel
			 */
			OutputPin* getOutputForChannel(int channel) override { return &mGainNodes[channel]->audioOutput; }

			/**
			 * Continue playback at current position
			 */
			void start();

			/**
			 * Start playback at given position in seconds 
			 * @param startPosition: the start position in the buffer in seconds
			 */
			void start(double startPosition);
			
			/**
			 * Fade out over fade out time and stop playback.
			 */
			void stop();

			/**
			 * Set the audio buffer to play
			 * @param resource the audio buffer to play
			 * @param error the error if buffer can't be set
			 */
			bool setBuffer(AudioBufferResource& resource, utility::ErrorState& error);

			/**
			 * @return audio playback buffer
			 */
			const AudioBufferResource& getBuffer() const { assert(mBuffer != nullptr); return *mBuffer; }

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
			bool isPlaying() const;

			/**
			 * Returns if playback reached the end
			 * @return if playback reached the end
			 */
			bool isFinished() const;
			
			/**
			 * @return the current gain value
			 */
			ControllerValue getGain() const { return mGain; }
			
			/**
			 * @return the current stereo panning.
			 */
			ControllerValue getStereoPanning() const { return mStereoPanning; }
			
			/**
			 * @return the pitch as a fraction of the original pitch of the audio material in the buffer.
			 */
			ControllerValue getPitch() const { return mPitch; }

			/**
			 * Returns buffer playback speed, which is a combination of the pitch and sample-rate
			 * @return current buffer playback speed
			 */
			float getSpeed() const;

			/**
			 * Returns the audio buffer sample rate
			 * @return the audio buffer sample rate
			 */
			float getSampleRate() const { return getBuffer().getSampleRate(); }
			
			/**
			 * @return the amount of time in ms the sequencer has been playing since the last call to play().
			 */
			[[deprecated]]
			TimeValue getCurrentPlayingTime() const	{ return mPlaytime * 1000.0; }

			/**
			 * @return time in seconds since start
			 */
			double getPlaytime() { return mPlaytime; }

			/**
			 * Returns the length of the buffer in seconds
			 * @param channel channel to get length for
			 * @return length of the buffer in seconds
			 */
			double getLength() const { return mLength; }

			/**
			 * Returns the current position in seconds in the buffer
			 * @return current position in seconds in the buffer, 0.0f if no channels exist
			 */
			double getPosition() const;

			/**
			 * Returns the current position in seconds in the buffer for the given channel
			 * @param channel channel to get position for
			 * @return current position in ms in the buffer for the given channel, 0.0 if channel isn't available
			 */
			double getPosition(int channel) const;

			/**
			 * Set position in seconds, can't exceed length
			 * @param pos position in seconds
			 */
			void setPosition(double pos);

			/**
			 * Update position in buffer, can't exceed buffer size
			 * @param sample sample position
			 */
			void setSamplePosition(DiscreteTimeValue pos);

			/**
			 * Returns the current position in the buffer
			 * @return current position in the buffer, 0 if channel isn't available
			 */
			DiscreteTimeValue getSamplePosition() const;

			/**
			 * @return total number of samples in the buffer
			 */
			DiscreteTimeValue getSampleCount() const { return getBuffer().getSize(); }

			/**
			 * Returns the current position in the buffer for the given channel
			 * @param channel channel to get position for
			 * @return current position in the buffer, 0 if channel isn't available
			 */
			DiscreteTimeValue getSamplePosition(int channel) const;

			/**
			 * Returns sample buffer for given channel, asserts if channel doesn't exist 
			 * @return sample buffer for given channel
			 */
			const SampleBuffer& getSamples(int channel) const;

			/**
			 * Returns available channels.
			 * For stereo 0,1; unless a custom mapping is provided.
			 * @return available channels
			 */
			const std::vector<int>& getChannels() const { return mChannelRouting; }

		private:
			void applyGain();
			int getBufferIndex(int channel) const;
			void createGraph();

			std::vector<SafeOwner<BufferPlayerNode>> mPlayerNodes;		// Nodes for each channel performing the actual audio playback.
			std::vector<SafeOwner<MultiplyNode>> mGainNodes;			// Nodes for each channel to gain the signal.
			std::vector<SafeOwner<ControlNode>> mGainControls;			// Nodes to control the gain for each channel.
			AudioBufferResource* mBuffer = nullptr;

			ControllerValue mGain = 1.0;
			std::vector<ControllerValue> mChannelGains;
			ControllerValue mStereoPanning = 0.5;
			ControllerValue mPitch = 1.0;
			double mPlaytime = 0;
			std::vector<int> mChannelRouting = { 0, 1 };				
			NodeManager* mNodeManager = nullptr;						// The audio node manager this component's audio nodes are managed by
			AudioService* mAudioService = nullptr;
			double mLength = 0.0;
		};	
	}
}

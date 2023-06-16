/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Std includes
#include <atomic>

// Nap includes
#include <audio/utility/safeptr.h>

// Audio includes
#include <audio/core/audionode.h>
#include <audio/core/audionodemanager.h>

namespace nap
{
	namespace audio
	{
		
		/**
		 * Node to play back audio from a buffer
		 */
		class NAPAPI BufferPlayerNode : public Node
		{
		RTTI_ENABLE(Node)
		
		public:
			BufferPlayerNode(NodeManager& manager) : Node(manager)
			{}
			
			/**
			 * The output to connect to other nodes
			 */
			OutputPin audioOutput = {this};
			
			/**
			 * Tells the node to start playback
			 * @param channel: the channel within the buffer to be played
			 * @param position: the starting position in the source buffer in samples
			 * @param speed: the playbackspeed, 1.0 means 1 sample per sample, 2 means double speed, etc.
			 */
			void play(int channel = 0, DiscreteTimeValue position = 0, ControllerValue speed = 1.);
			
			/**
			 * Stops playback
			 */
			void stop();
			
			/**
			 * Set the playback speed
			 * @param speed as a fraction of the original speed of the audio material in the buffer.
			 */
			void setSpeed(ControllerValue speed);
			
			/**
			 * Sets the current position of playback while playing.
			 * @param position in samples
			 */
			void setPosition(DiscreteTimeValue position);
			
			/**
			 * Sets the current channel of playback while playing.
			 * @param channel index of the channel
			 */
			void setChannel(int channel);
			
			/**
			 * Sets the buffer to be played back from. Can't be called while playing!
			 * @param buffer SafePtr to a multichannel sample buffer
			 */
			void setBuffer(SafePtr<MultiSampleBuffer> buffer);
			
			/**
			 * @return the playback speed as a fraction of the original speed of the audio material in the buffer.
			 */
			ControllerValue getSpeed() const { return mSpeed; }
			
			/**
			 * @return the current playback position within the source buffer.
			 */
			DiscreteTimeValue getPosition() const { return mPosition; }
			
			/**
			 * @return the current playback channel within the source buffer.
			 */
			int getChannel() const { return mChannel; }
		
		private:
			// Inherited from Node
			void process() override;
			
			std::atomic<bool> mPlaying = {false}; // Indicates wether the node is currently playing.
			std::atomic<int> mChannel = {0}; // The channel within the buffer that is being played bacl/
			std::atomic<double> mPosition = {0}; // Current position of playback in samples within the source buffer.
			std::atomic<ControllerValue> mSpeed = {1.f}; // Playback speed as a fraction of the original speed.
			SafePtr<MultiSampleBuffer> mBuffer = nullptr; // Pointer to the buffer with audio material being played back.
		};
		
	}
}

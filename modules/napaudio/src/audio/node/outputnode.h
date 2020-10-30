/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Std includes
#include <atomic>

// Audio includes
#include <audio/core/audionode.h>
#include <audio/core/process.h>

namespace nap
{
	namespace audio
	{
		
		/**
		 * Node to provide audio output for the node manager's audio processing, typically sent to an audio interface.
		 * The OutputNode is a root node that will be directly processed by the node manager.
		 */
		class NAPAPI OutputNode final : public Node
		{
			RTTI_ENABLE(Node)
			
		public:
			/**
			 * @param audioService: The nap AudioService
			 * @param rootProcess: true if the node is registered as root process and being processed from the moment of creation. This can cause glitches if the node tree and it's parameters are still being build.
			 */
			OutputNode(NodeManager& nodeManager, bool rootProcess = true);
			
			~OutputNode() override final;
			
			/**
			 * Through this input the node receives buffers of audio samples that will be presented to the node manager as output for its audio processing.
			 */
			InputPin audioInput = {this};
			
			/**
			 * Set the audio channel that this node's input will be played on by the node manager.
			 * @param outputChannel: the channel number
			 */
			void setOutputChannel(int outputChannel) { mOutputChannel = outputChannel; }
			
			/**
			 * @return: the audio channel that this node's input will be played on by the node manager.
			 */
			int getOutputChannel() const { return mOutputChannel; }
		
		private:
			void process() override;
			
			std::atomic<int> mOutputChannel = {
					0}; // The audio channel that this node's input will be played on by the node manager.
			
			bool mRootProcess = false;
		};
		
		
	}
}






/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Std includes
#include <atomic>

// Audio includes
#include <audio/core/audionode.h>

namespace nap
{
	namespace audio
	{
		
		
		/**
		 * This node outputs the audio input that is received from the node system's external input, typically an audio interface.
		 * Input from channel inputChannel can be pulled from audioOutput plug.
		 */
		class NAPAPI InputNode final : public Node
		{
			RTTI_ENABLE(Node)
			
			friend class NodeManager;
		
		public:
			/**
			 * @param manager: the node manager that this node will be registered to and processed by. This node provides audio output for the manager.
			 */
			InputNode(NodeManager& manager) : Node(manager) { }
			
			/**
			 * This output will contain the samples received from the node system's external input.
			 */
			OutputPin audioOutput = {this};
			
			/**
			 * Sets the channel from which this node receives input.
			 * First check if the inputChannel is available using getAvailableInputChannelCount().
			 */
			void setInputChannel(int inputChannel) { mInputChannel = inputChannel; }
			
			/**
			 * @return: the channel from which this node receives input.
			 */
			int getInputChannel() const { return mInputChannel; }
			
			/**
			 * @return: the number of input channels currently present in the audio service to be selected using getInputChannel().
			 */
			int getAvailableInputChannelCount();
		
		private:
			void process() override;
			
			std::atomic<int> mInputChannel = {0}; // Input channel of the audio interface to receive input data from.
		};
		
		
	}
}






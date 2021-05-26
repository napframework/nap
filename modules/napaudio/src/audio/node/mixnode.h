/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <audio/core/audionode.h>

namespace nap
{
	namespace audio
	{
		
		/**
		 * Node to scale an audio signal
		 */
		class NAPAPI MixNode : public Node
		{
			RTTI_ENABLE(Node)
		
		public:
			/**
			 * Constructor
			 * @param manager the node manager this node will be processed by
			 * @param reservedInputCount the number of input pointers that will be pre allocated to hold the result value.
			 */
			MixNode(NodeManager& manager, int reservedInputCount = 2);
			
			/**
			 * The input to be mixed
			 */
			MultiInputPin inputs;
			
			/**
			 * Outputs the mixed signal
			 */
			OutputPin audioOutput = {this};
		
		private:
			/**
			 * Calculate the output, perform the scaling
			 */
			void process() override;

			std::vector<SampleBuffer*> mInputBuffers; // Internal preallocated input result buffer
		};
		
	}
}


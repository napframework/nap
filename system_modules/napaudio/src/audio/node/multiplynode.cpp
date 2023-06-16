/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "multiplynode.h"

namespace nap
{
	namespace audio
	{

		MultiplyNode::MultiplyNode(NodeManager& nodeManager, int reservedInputCount) : Node(nodeManager), inputs(this, reservedInputCount)
		{
			mInputBuffers.reserve(reservedInputCount);
		}

		
		void MultiplyNode::process()
		{
			auto& outputBuffer = getOutputBuffer(audioOutput);
			inputs.pull(mInputBuffers);
			
			// In case no inputs are connected, return zeros
			if (mInputBuffers.empty()) {
				for (auto i = 0; i < outputBuffer.size(); ++i)
					outputBuffer[i] = 0;
				return;
			}
			
			// Copy the first input to the output
			auto inputIndex = 0;
			auto inputBuffer = *mInputBuffers.begin();
			for (auto i = 0; i < outputBuffer.size(); ++i)
				outputBuffer[i] = (*inputBuffer)[i];
			
			// Multiply the output with the consecutive inputs from the second onwards
			for (inputIndex = 1; inputIndex < mInputBuffers.size(); ++inputIndex) {
				inputBuffer = mInputBuffers[inputIndex];
				for (auto i = 0; i < outputBuffer.size(); ++i)
					outputBuffer[i] *= (*inputBuffer)[i];
			}
		}

	}
}

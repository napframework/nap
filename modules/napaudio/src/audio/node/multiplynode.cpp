/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "multiplynode.h"

namespace nap
{
	namespace audio
	{
		
		void MultiplyNode::process()
		{
			auto& outputBuffer = getOutputBuffer(audioOutput);
			auto& inputBuffers = inputs.pull();
			
			// In case no inputs are connected, return zeros
			if (inputBuffers.empty()) {
				for (auto i = 0; i < outputBuffer.size(); ++i)
					outputBuffer[i] = 0;
				return;
			}
			
			// Copy the first input to the output
			auto inputIndex = 0;
			auto inputBuffer = *inputBuffers.begin();
			for (auto i = 0; i < outputBuffer.size(); ++i)
				outputBuffer[i] = (*inputBuffer)[i];
			
			// Multiply the output with the consecutive inputs from the second onwards
			for (inputIndex = 1; inputIndex < inputBuffers.size(); ++inputIndex) {
				inputBuffer = inputBuffers[inputIndex];
				for (auto i = 0; i < outputBuffer.size(); ++i)
					outputBuffer[i] *= (*inputBuffer)[i];
			}
		}
		
	}
}

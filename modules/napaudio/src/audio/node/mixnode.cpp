/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "mixnode.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::MixNode)
	RTTI_PROPERTY("inputs", &nap::audio::MixNode::inputs, nap::rtti::EPropertyMetaData::Embedded)
	RTTI_PROPERTY("audioOutput", &nap::audio::MixNode::audioOutput, nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS

namespace nap
{
	namespace audio
	{

		MixNode::MixNode(NodeManager& manager, int reservedInputCount) : Node(manager), inputs(this, reservedInputCount)
		{
			mInputBuffers.reserve(reservedInputCount);
		}

		
		void MixNode::process()
		{
			auto& outputBuffer = getOutputBuffer(audioOutput);
			inputs.pull(mInputBuffers);
			
			for (auto i = 0; i < outputBuffer.size(); ++i)
				outputBuffer[i] = 0;
			
			for (auto& inputBuffer : mInputBuffers)
				if (inputBuffer)
					for (auto i = 0; i < outputBuffer.size(); ++i)
						outputBuffer[i] += (*inputBuffer)[i];
		}

	}
}


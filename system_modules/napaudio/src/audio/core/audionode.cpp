/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "audionode.h"
#include "audionodemanager.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::Node)
	RTTI_FUNCTION("getBufferSize", &nap::audio::Node::getBufferSize)
	RTTI_FUNCTION("getSampleRate", &nap::audio::Node::getSampleRate)
RTTI_END_CLASS

namespace nap
{
	namespace audio
	{

		Node::Node(NodeManager& manager) : Process(manager)
		{
		}
		
		
		SampleBuffer& Node::getOutputBuffer(OutputPin& output)
		{
			return output.mBuffer;
		}
		
		
		void Node::setBufferSize(int bufferSize)
		{
			for (auto& output : mOutputs)
				output->setBufferSize(bufferSize);
			
			bufferSizeChanged(bufferSize);
		}

	}
}


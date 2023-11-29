/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "fftnode.h"

#include <audio/core/audionodemanager.h>
#include <cmath>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::FFTNode)
	RTTI_PROPERTY("Input",		&nap::FFTNode::mInput,		nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS


namespace nap
{
	FFTNode::FFTNode(audio::NodeManager& nodeManager, FFTBuffer::EOverlap overlaps) :
		audio::Node(nodeManager)
	{
		mFFTBuffer = std::make_unique<FFTBuffer>(getNodeManager().getInternalBufferSize(), overlaps);
		getNodeManager().registerRootProcess(*this);
	}


	FFTNode::~FFTNode()
	{
		getNodeManager().unregisterRootProcess(*this);
	}


	void FFTNode::process()
	{
		audio::SampleBuffer* input_buffer = mInput.pull();
		if (input_buffer == nullptr)
			return;

		mFFTBuffer->supply(*input_buffer);
	}
}

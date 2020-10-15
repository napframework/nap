/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "pullnode.h"

// Audio includes
#include <audio/core/audionodemanager.h>

RTTI_DEFINE_BASE(nap::audio::PullNode)

namespace nap
{
	namespace audio
	{
		
		PullNode::PullNode(NodeManager& nodeManager, bool rootProcess) : Node(nodeManager), mRootProcess(rootProcess)
		{
			if (rootProcess)
				getNodeManager().registerRootProcess(*this);
		}
		
		
		PullNode::~PullNode()
		{
			if (mRootProcess)
				getNodeManager().unregisterRootProcess(*this);
		}
		
		
		void PullNode::process()
		{
			audioInput.pull();
		}
		
	}
}


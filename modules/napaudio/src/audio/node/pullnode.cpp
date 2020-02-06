#include "pullnode.h"

// Audio includes
#include <audio/service/audioservice.h>
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


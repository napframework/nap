#include "pullnode.h"

// Audio includes
#include <audio/service/audioservice.h>
#include <audio/core/audionodemanager.h>

RTTI_DEFINE_BASE(nap::audio::PullNode)

namespace nap
{
    
    namespace audio
    {
        
        PullNode::PullNode(NodeManager& nodeManager, bool active) : Node(nodeManager)
        {
            nodeManager.registerRootNode(*this);
            mActive = active;
        }
        
        
        PullNode::~PullNode()
        {
            getNodeManager().unregisterRootNode(*this);
        }
        
        
        void PullNode::process()
        {
            if (!mActive)
                return;
            
            audioInput.pull();
        }
        
    }
        
}


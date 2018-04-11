#include "outputnode.h"

// Audio includes
#include <audio/service/audioservice.h>
#include <audio/core/audionodemanager.h>

RTTI_DEFINE_BASE(nap::audio::OutputNode)

namespace nap
{
    
    namespace audio
    {
        
        OutputNode::OutputNode(NodeManager& nodeManager, bool active) : Node(nodeManager)
        {
            nodeManager.registerRootNode(*this);
            mActive = active;
        }
        
        
        OutputNode::~OutputNode()
        {
            getNodeManager().unregisterRootNode(*this);
        }
        
        
        void OutputNode::process()
        {
            if (!mActive)
                return;
            
            SampleBuffer* buffer = audioInput.pull();
            if (buffer)
                getNodeManager().provideOutputBufferForChannel(buffer, mOutputChannel);
        }
        
    }
        
}


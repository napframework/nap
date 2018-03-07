#include "outputnode.h"

#include <audio/core/audionodemanager.h>

RTTI_DEFINE_BASE(nap::audio::OutputNode)

namespace nap
{
    
    namespace audio
    {
        
        OutputNode::OutputNode(NodeManager& manager, bool active) : Node(manager)
        {
            manager.registerRootNode(*this);
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
            
            SampleBufferPtr buffer = audioInput.pull();
            if (buffer)
                getNodeManager().provideOutputBufferForChannel(buffer, mOutputChannel);
        }
        
    }
        
}


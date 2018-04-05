#include "outputnode.h"

// Audio includes
#include <audio/service/audioservice.h>
#include <audio/core/audionodemanager.h>

RTTI_DEFINE_BASE(nap::audio::OutputNode)

namespace nap
{
    
    namespace audio
    {
        
        OutputNode::OutputNode(AudioService& service, bool active) : Node(service.getNodeManager())
        {
            service.execute([&](){ getNodeManager().registerRootNode(*this); });
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


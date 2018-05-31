#include "outputnode.h"

// Audio includes
#include <audio/service/audioservice.h>
#include <audio/core/audionodemanager.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::OutputNode)
    RTTI_PROPERTY("audioInput", &nap::audio::OutputNode::audioInput, nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS


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
            
            auto outputChannel = mOutputChannel.load();
            
            SampleBuffer* buffer = audioInput.pull();
            if (buffer)
                getNodeManager().provideOutputBufferForChannel(buffer, outputChannel);
        }
        
    }
        
}


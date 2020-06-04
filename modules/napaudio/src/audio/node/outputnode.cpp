#include "outputnode.h"

// Audio includes
#include <audio/core/audionodemanager.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::OutputNode)
    RTTI_PROPERTY("audioInput", &nap::audio::OutputNode::audioInput, nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS


namespace nap
{
    
    namespace audio
    {
        
        OutputNode::OutputNode(NodeManager& nodeManager, bool rootProcess) : Node(nodeManager), mRootProcess(rootProcess)
        {
            if (rootProcess)
                getNodeManager().registerRootProcess(*this);
        }
        
        
        OutputNode::~OutputNode()
        {
            if (mRootProcess && isRegisteredWithNodeManager())
                getNodeManager().unregisterRootProcess(*this);
        }
        
        
        void OutputNode::process()
        {
            auto outputChannel = mOutputChannel.load();
            
            SampleBuffer* buffer = audioInput.pull();
            if (buffer)
                getNodeManager().provideOutputBufferForChannel(buffer, outputChannel);
        }
        
    }
        
}


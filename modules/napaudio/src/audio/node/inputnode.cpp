#include "inputnode.h"

#include <audio/core/audionodemanager.h>

RTTI_DEFINE_BASE(nap::audio::InputNode)

namespace nap
{
    
    namespace audio
    {
        
        void InputNode::process()
        {
            auto& buffer = getOutputBuffer(audioOutput);
            for (auto i = 0; i < buffer.size(); ++i)
                buffer[i] = getNodeManager().getInputSample(mInputChannel, i);
        }

    }
        
}


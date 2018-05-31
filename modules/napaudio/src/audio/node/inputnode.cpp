#include "inputnode.h"

#include <audio/core/audionodemanager.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::InputNode)
    RTTI_PROPERTY("audioOutput", &nap::audio::InputNode::audioOutput, nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS

namespace nap
{
    
    namespace audio
    {
        
        void InputNode::process()
        {
            auto inputChannel = mInputChannel.load();
            auto& buffer = getOutputBuffer(audioOutput);
            for (auto i = 0; i < buffer.size(); ++i)
                buffer[i] = getNodeManager().getInputSample(inputChannel, i);
        }

    }
        
}


#include "noisenode.h"

// Std includes
#include <stdlib.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::NoiseNode)
    RTTI_PROPERTY("audioOutput", &nap::audio::NoiseNode::audioOutput, nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS

namespace nap
{
    
    namespace audio
    {
        
        void NoiseNode::process()
        {
            auto& buffer = getOutputBuffer(audioOutput);
            for (auto i = 0; i < buffer.size(); ++i)
                buffer[i] = rand() / float(RAND_MAX);
        }
        
    }
    
}

#include "FastMixNode.h"
#include <Spatial/Utility/VectorExtension.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::FastMixNode)
    RTTI_PROPERTY("inputs", &nap::audio::FastMixNode::inputs, nap::rtti::EPropertyMetaData::Embedded)
    RTTI_PROPERTY("audioOutput", &nap::audio::FastMixNode::audioOutput, nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS

namespace nap
{
    
    namespace audio
    {
        
        void FastMixNode::process()
        {
            auto& outputBuffer = getOutputBuffer(audioOutput);
            auto& inputBuffers = inputs.pull();

			// note: we assume in the code below the input/output buffers are a multiple of
			// eight samples in size. this avoids the need of having to process any
			// 'remaining' samples after the vector addition.
			assert((outputBuffer.size() % 8) == 0);
			
			const size_t v_numSamples = outputBuffer.size() / 8;
			
			float8* __restrict v_outputBuffer = (float8*)outputBuffer.data();
			memset(v_outputBuffer, 0, v_numSamples * sizeof(float8));
            
            for (auto inputBuffer : inputBuffers)
            {
                if (inputBuffer != nullptr)
                {
                	const float8* __restrict v_inputBuffer = (const float8*)inputBuffer->data();
                    vectorAdd(v_outputBuffer, v_inputBuffer, v_numSamples);
                }
            }
        }
        
    }
    
}


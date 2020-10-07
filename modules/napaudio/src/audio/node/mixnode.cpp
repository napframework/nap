#include "mixnode.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::MixNode)
	RTTI_PROPERTY("inputs", &nap::audio::MixNode::inputs, nap::rtti::EPropertyMetaData::Embedded)
	RTTI_PROPERTY("audioOutput", &nap::audio::MixNode::audioOutput, nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS

namespace nap
{
	namespace audio
	{
		
		void MixNode::process()
		{
			auto& outputBuffer = getOutputBuffer(audioOutput);
			auto& inputBuffers = inputs.pull();
			
			for (auto i = 0; i < outputBuffer.size(); ++i)
				outputBuffer[i] = 0;
			
			for (auto& inputBuffer : inputBuffers)
				if (inputBuffer)
					for (auto i = 0; i < outputBuffer.size(); ++i)
						outputBuffer[i] += (*inputBuffer)[i];
		}
		
	}
}


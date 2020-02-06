#include "circularbufferplayernode.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::CircularBufferPlayerNode)
    RTTI_PROPERTY("audioOutput", &nap::audio::CircularBufferPlayerNode::audioOutput, nap::rtti::EPropertyMetaData::Embedded)
    RTTI_FUNCTION("play", &nap::audio::CircularBufferPlayerNode::play)
    RTTI_FUNCTION("stop", &nap::audio::CircularBufferPlayerNode::stop)
RTTI_END_CLASS

namespace nap
{
    
    namespace audio
    {
        
        
        void CircularBufferPlayerNode::play(CircularBufferNode& buffer, int relativePosition, ControllerValue speed)
        {
            mNewRelativePosition = relativePosition;
            mNewSpeed = speed;
            mNewBuffer.store(&buffer);
            mIsDirty.set();
        }
        
        
        void CircularBufferPlayerNode::stop()
        {
            mNewBuffer.store(nullptr);
            mIsDirty.set();
        }
        
        
        void CircularBufferPlayerNode::process()
        {
            auto& outputBuffer = getOutputBuffer(audioOutput);
            
            if (mIsDirty.check())
            {
                mBuffer = mNewBuffer.load();
                if (mBuffer)
                {
                    // We add one buffer size to the relative starting position of the playback.
                    // This is to make sure we don't start playing back data after the write position of the buffer in case mNewRelativePosition is 0
                    mPosition = mBuffer->getAbsolutePosition(mNewRelativePosition.load() + getBufferSize());
                    mSpeed = mNewSpeed.load();
                }
            }
            
            // If we're not playing, fill the buffer with 0's and bail out.
            if (mBuffer == nullptr)
            {
                std::memset(outputBuffer.data(), 0, sizeof(SampleValue) * outputBuffer.size());
                return;
            }
            
            DiscreteTimeValue flooredPosition;
            SampleValue lastValue, newValue, fractionalPart;
            
            // For each sample
            for (auto i = 0; i < outputBuffer.size(); i++)
            {
                flooredPosition = DiscreteTimeValue(mPosition);
                lastValue = mBuffer->getSample(flooredPosition);
                newValue = mBuffer->getSample(flooredPosition + 1);
                
                fractionalPart = mPosition - flooredPosition;
                
                outputBuffer[i] = lastValue + (fractionalPart * (newValue - lastValue));
                
                mPosition += mSpeed;
            }
        }
        
        
    }
    
}

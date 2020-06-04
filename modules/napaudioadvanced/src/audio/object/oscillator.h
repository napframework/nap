#pragma once

// Nap includes
#include <nap/resourceptr.h>

// Audio includes
#include <audio/core/nodeobject.h>
#include <audio/node/oscillatornode.h>
#include <audio/utility/safeptr.h>

namespace nap
{
    
    namespace audio
    {

        class NAPAPI WaveTableResource : public Resource {
            RTTI_ENABLE(Resource)
        public:
            WaveTableResource(NodeManager& nodeManager) : Resource(), mNodeManager(nodeManager) { }
            bool init(utility::ErrorState& errorState);

            int mSize = 2048; ///< Property: 'Size' Size of the wavetable. Has to be a power of two.

            SafePtr<WaveTable> getWave() { return mWave.get(); }

        private:
            SafeOwner<WaveTable> mWave = nullptr;
            NodeManager& mNodeManager;
        };

        
        /**
         * Multichannel oscillator object.
         */
        class NAPAPI Oscillator : public ParallelNodeObject<OscillatorNode>
        {
            RTTI_ENABLE(ParallelNodeObjectBase)
            
        public:
            std::vector<ControllerValue> mFrequency = { 220.f }; ///< property: 'Frequency' array of frequency values that will be mapped on the oscillators on each channel
            std::vector<ControllerValue> mAmplitude = { 1.f }; ///< property: 'Amplitude' array of amplitude values that will be mapped on the oscillators on each channel
            ResourcePtr<AudioObject> mFmInput = nullptr; ///< property: 'FmInput' audio object of which the outputs will modulate the frequencies of the oscillators on each channel.
            ResourcePtr<WaveTableResource> mWaveTable = nullptr; ///< property: 'WaveTable' Pointer to a wave table resource.
            
        private:
            bool initNode(int channel, OscillatorNode& node, utility::ErrorState& errorState) override
            {
                node.setWave(mWaveTable->getWave());
                node.setFrequency(mFrequency[channel % mFrequency.size()]);
                node.setAmplitude(mAmplitude[channel % mAmplitude.size()]);
                if (mFmInput != nullptr)
                {
                    node.fmInput.connect(*mFmInput->getInstance()->getOutputForChannel(channel % mFmInput->getInstance()->getChannelCount()));
                }

                return true;
            }
        };

        using WaveTableResourceObjectCreator = rtti::ObjectCreator<WaveTableResource, NodeManager>;

    }
    
}

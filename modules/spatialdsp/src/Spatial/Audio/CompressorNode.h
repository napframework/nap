#pragma once

// Nap includes
#include <nap/resourceptr.h>

// Audio includes
#include <audio/core/audionode.h>
#include <audio/core/nodeobject.h>
#include <Spatial/Audio/MultiChannelWithInput.h>

namespace nap
{
    
    namespace audio
    {
        
        // Class with compiled Faust code.
        /* Ported faust code:
         *
         * import("stdfaust.lib");
         * ratio = vslider("ratio", 4, 1, 20, 0.01);
         * thresh = vslider("thresh", -6, -90, 0, 0.1);
         * attack = vslider("attack", 0.0008, 0, 0.2, 0.0001);
         * release = vslider("release", 0.5, 0, 1, 0.0001);
         *
         * process = co.compressor_mono(ratio, thresh, attack, release);
         */
        
        class NAPAPI FaustCompressor {
            
        public:
            FaustCompressor(int samplerate);
            
            void setAttack(float attack){ fVslider0 = attack; }
            void setRatio(float ratio){ fVslider1 = ratio; }
            void setRelease(float release){ fVslider2 = release; }
            void setThreshold(float threshold){ fVslider3 = threshold; }

            void compute(int count, float* input0, float* output0);
            
        private:
            int fSamplingFreq;
            
            float fVslider0 = 0.001; // attack
            float fVslider1 = 4; // ratio
            float fVslider2 = 0.5; // release
            float fVslider3 = -6; // threshold
            
            float fConst0 = 0.f;
            float fConst1 = 0.f;
            float fConst2 = 0.f;
            
            float fRec2[2] = { 0.f, 0.f };
            float fRec1[2] = { 0.f, 0.f };
            float fRec0[2] = { 0.f, 0.f };
            
        };
        
        /**
         * CompressorNode is just a port of compiled Faust code.
         */
        class NAPAPI CompressorNode : public Node
        {
            RTTI_ENABLE(Node)
        public:
            CompressorNode(NodeManager& manager) : Node(manager), faustCompressor(getSampleRate()) {
                setRatio(4.);
                setThreshold(-6.);
                setAttack(0.0008);
                setRelease(0.5);
            }
            
            InputPin audioInput = { this };
            
            OutputPin audioOutput = { this };
            
            void setAttack(float attack){ faustCompressor.setAttack(attack); }
            void setRatio(float ratio){ faustCompressor.setRatio(ratio); }
            void setRelease(float release){ faustCompressor.setRelease(release); }
            void setThreshold(float threshold){ faustCompressor.setThreshold(threshold); }
            
        private:
            void process() override {
                
                auto& inputBuffer = *audioInput.pull();
                auto& outputBuffer = getOutputBuffer(audioOutput);

                // because buffers are std::vectors, I have to get them as float arrays
                float* inputArray = &inputBuffer[0];
                float* outputArray = &outputBuffer[0];
                
                faustCompressor.compute(getBufferSize(), inputArray, outputArray);
            }
            
            FaustCompressor faustCompressor;
            
        };
        
        using Compressor = MultiChannelWithInput<CompressorNode>;
        using CompressorInstance = ParallelNodeObjectInstance<CompressorNode>;

        
    }
}


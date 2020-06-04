#pragma once


// Nap includes
#include <rtti/rtti.h>
#include <mathutils.h>
#include <nap/resourceptr.h>

// Audio includes
#include <audio/core/audioobject.h>

#define RAMP_TIME 10


namespace nap {
    
    namespace audio {
        

        enum FilterChainType { LowPass6dB, LowPass12dB, LowPass18dB, LowPass24dB, HighPass6dB, HighPass12dB, HighPass18dB, HighPass24dB };
        
        class FilterChainInstance;
        

        
        
        /**
         * @MultiChannelObject containing one or more filter nodes for each channel.
         */
        class NAPAPI FilterChain : public AudioObject
        {
            RTTI_ENABLE(AudioObject)
        public:
            FilterChain() = default;
            
            // Properties.
            ResourcePtr<AudioObject> mInput = nullptr; ///< Property 'Input': Points to the input.
            FilterChainType mType = FilterChainType::LowPass6dB; ///<Property 'Type': Indicates the type and strength of filter.
            
        private:
            std::unique_ptr<AudioObjectInstance> createInstance(NodeManager& nodeManager, utility::ErrorState& errorState) override;
            
        };
        
        class NAPAPI FilterChainInstance : public AudioObjectInstance
        {
            RTTI_ENABLE(AudioObjectInstance)
        public:
            FilterChainInstance() : AudioObjectInstance() { }
            
            bool init(AudioObjectInstance* input, FilterChainType type, NodeManager& nodeManager, utility::ErrorState& errorState);
            
            OutputPin* getOutputForChannel(int channel) override { return &mChain->getOutput(); };
            int getChannelCount() const override { return 1; }
            
            void connect(unsigned int channel, OutputPin& pin) override { mChain->getInput().connect(pin); }
            int getInputChannelCount() const override { return 1; }
            
            void setFrequency(float frequency){
                mChain->setCutoffFrequency(frequency);
            }
            
        private:
            class ChainBase
            {
            public:
            	virtual ~ChainBase() = default;
            	
                virtual void setCutoffFrequency(ControllerValue frequency) = 0;
                virtual InputPinBase& getInput() = 0;
                virtual OutputPin& getOutput() = 0;
            };
            
            template <typename T>
            class Chain : public ChainBase
            {
            public:
                Chain(NodeManager& nodeManager, int count)
                {
                    for (auto i = 0; i < count; ++i)
                    {
                        auto node = nodeManager.makeSafe<T>(nodeManager);
                        node->setRampTime(RAMP_TIME);
                        
                        if (i > 0)
                            (*node->getInputs().begin())->connect(**mNodes.back()->getOutputs().begin());
                        mNodes.emplace_back(std::move(node));
                    }
                }
                
                void setCutoffFrequency(ControllerValue frequency) override
                {
                    for (auto& node : mNodes)
                        node->setCutoffFrequency(frequency);
                }
                
                OutputPin& getOutput() override { return **mNodes.back()->getOutputs().begin(); }
                InputPinBase& getInput() override { return **mNodes[0]->getInputs().begin(); }
                
            private:
                std::vector<SafeOwner<T>> mNodes;
            };
            
        private:
            std::unique_ptr<ChainBase> mChain = nullptr;
        };
        
    }
    
}


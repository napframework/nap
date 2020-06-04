
#include "FilterChain.h"

// Audio includes
#include <audio/node/onepolenode.h>


RTTI_BEGIN_ENUM(nap::audio::FilterChainType)
    RTTI_ENUM_VALUE(nap::audio::FilterChainType::LowPass6dB, "LowPass6dB"),
    RTTI_ENUM_VALUE(nap::audio::FilterChainType::LowPass12dB, "LowPass12dB"),
    RTTI_ENUM_VALUE(nap::audio::FilterChainType::LowPass18dB, "LowPass18dB"),
    RTTI_ENUM_VALUE(nap::audio::FilterChainType::LowPass24dB, "LowPass24dB"),
    RTTI_ENUM_VALUE(nap::audio::FilterChainType::HighPass6dB, "HighPass6dB"),
    RTTI_ENUM_VALUE(nap::audio::FilterChainType::HighPass12dB, "HighPass12dB"),
    RTTI_ENUM_VALUE(nap::audio::FilterChainType::HighPass18dB, "HighPass18dB"),
    RTTI_ENUM_VALUE(nap::audio::FilterChainType::HighPass24dB, "HighPass24dB")
RTTI_END_ENUM


RTTI_BEGIN_CLASS(nap::audio::FilterChain)
    RTTI_PROPERTY("Input", &nap::audio::FilterChain::mInput, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("FilterChainType", &nap::audio::FilterChain::mType, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::audio::FilterChainInstance)
RTTI_END_CLASS

namespace nap
{
    
    namespace audio
    {
        
        std::unique_ptr<AudioObjectInstance> FilterChain::createInstance(NodeManager& nodeManager, utility::ErrorState& errorState)
        {
            auto instance = std::make_unique<FilterChainInstance>();
            AudioObjectInstance* input = nullptr;
            if (mInput != nullptr)
                input = mInput->getInstance();
            if (!instance->init(input, mType, nodeManager, errorState))
                return nullptr;
            
            return std::move(instance);
        }
        
        
        bool FilterChainInstance::init(AudioObjectInstance* input, FilterChainType type, NodeManager& nodeManager, utility::ErrorState& errorState)
        {
            switch (type)
            {
                case LowPass6dB:
                    mChain = std::make_unique<Chain<OnePoleLowPassNode>>(nodeManager, 1);
                    break;
                case LowPass12dB:
                    mChain = std::make_unique<Chain<OnePoleLowPassNode>>(nodeManager, 2);
                    break;
                case LowPass18dB:
                    mChain = std::make_unique<Chain<OnePoleLowPassNode>>(nodeManager, 3);
                    break;
                case LowPass24dB:
                    mChain = std::make_unique<Chain<OnePoleLowPassNode>>(nodeManager, 4);
                    break;
                case HighPass6dB:
                    mChain = std::make_unique<Chain<OnePoleHighPassNode>>(nodeManager, 1);
                    break;
                case HighPass12dB:
                    mChain = std::make_unique<Chain<OnePoleHighPassNode>>(nodeManager, 2);
                    break;
                case HighPass18dB:
                    mChain = std::make_unique<Chain<OnePoleHighPassNode>>(nodeManager, 3);
                    break;
                case HighPass24dB:
                    mChain = std::make_unique<Chain<OnePoleHighPassNode>>(nodeManager, 4);
                    break;
            }
            
            // if input is nullptr (for example when it's the input audioobject of a graph), the input will be connected by the graphobject
            if(input != nullptr)
                mChain->getInput().connect(*input->getOutputForChannel(0));
            
            return true;
        }

    
    }
}

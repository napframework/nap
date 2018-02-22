#include "levelmetercomponent.h"

// Nap includes
#include <entity.h>
#include <nap/core.h>

// Audio includes
#include <audio/service/audioservice.h>

// RTTI
RTTI_BEGIN_CLASS(nap::audio::LevelMeterComponent)
    RTTI_PROPERTY("Input", &nap::audio::LevelMeterComponent::mInput, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("AnalysisWindowSize", &nap::audio::LevelMeterComponent::mAnalysisWindowSize, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("MeterType", &nap::audio::LevelMeterComponent::mMeterType, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::LevelMeterComponentInstance)
    RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
    RTTI_FUNCTION("getLevel", &nap::audio::LevelMeterComponentInstance::getLevel)
RTTI_END_CLASS

namespace nap
{
    
    namespace audio
    {
        
        bool LevelMeterComponentInstance::init(utility::ErrorState& errorState)
        {
            for (auto channel = 0; channel < mInput->getChannelCount(); ++channel)
            {
                mMeters.emplace_back(std::make_unique<LevelMeterNode>(getNodeManager()));
                mMeters.back()->input.connect(mInput->getOutputForChannel(channel));
            }
            
            return true;
        }
        
        
        NodeManager& LevelMeterComponentInstance::getNodeManager()
        {
            return getEntityInstance()->getCore()->getService<AudioService>(rtti::ETypeCheck::EXACT_MATCH)->getNodeManager();
        }
        
        
        SampleValue LevelMeterComponentInstance::getLevel(int channel)
        {
            assert(channel < mMeters.size());
            return mMeters[channel]->getLevel();
        }
        
        
    }
    
}

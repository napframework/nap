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
    RTTI_PROPERTY("FilterInput", &nap::audio::LevelMeterComponent::mFilterInput, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("CenterFrequency", &nap::audio::LevelMeterComponent::mCenterFrequency, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("BandWidth", &nap::audio::LevelMeterComponent::mBandWidth, nap::rtti::EPropertyMetaData::Default)
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
            auto resource = getComponent<LevelMeterComponent>();
            auto audioService = getEntityInstance()->getCore()->getService<AudioService>(rtti::ETypeCheck::EXACT_MATCH);
            for (auto channel = 0; channel < mInput->getChannelCount(); ++channel)
            {
                mMeters.emplace_back(audioService->makeSafe<LevelMeterNode>(*audioService));
                
                if (resource->mFilterInput)
                {
                    auto filter = audioService->makeSafe<FilterNode>(getNodeManager());
                    filter->setMode(FilterNode::EMode::BandPass);
                    filter->setFrequency(resource->mCenterFrequency);
                    filter->setGain(resource->mFilterGain);
                    filter->audioInput.connect(mInput->getOutputForChannel(channel));
                    mMeters.back()->input.connect(filter->audioOutput);
                    mFilters.emplace_back(std::move(filter));
                }
                else {
                    mMeters.back()->input.connect(mInput->getOutputForChannel(channel));
                }
            }
            
            return true;
        }
        
        
        NodeManager& LevelMeterComponentInstance::getNodeManager()
        {
            return getEntityInstance()->getCore()->getService<AudioService>(rtti::ETypeCheck::EXACT_MATCH)->getNodeManager();
        }
        
        
        ControllerValue LevelMeterComponentInstance::getLevel(int channel)
        {
            assert(channel < mMeters.size());
            return mMeters[channel]->getLevel();
        }
        
        
        void LevelMeterComponentInstance::setCenterFrequency(ControllerValue centerFrequency)
        {
            for (auto& filter : mFilters)
                filter->setFrequency(centerFrequency);
        }
        
        
        void LevelMeterComponentInstance::setBandWidth(ControllerValue bandWidth)
        {
            for (auto& filter : mFilters)
                filter->setBand(bandWidth);
        }
        
        
        void LevelMeterComponentInstance::setFilterGain(ControllerValue gain)
        {
            for (auto& filter : mFilters)
                filter->setGain(gain);
        }
        
        
        ControllerValue LevelMeterComponentInstance::getCenterFrequency() const
        {
            if (mFilters.size() > 0)
                return (*mFilters.begin())->getFrequency();
            else
                return 0;
        }
        
        
        ControllerValue LevelMeterComponentInstance::getBandWidth() const
        {
            if (mFilters.size() > 0)
                return (*mFilters.begin())->getBand();
            else
                return 0;
        }
        
        
        ControllerValue LevelMeterComponentInstance::getFilterGain() const
        {
            if (mFilters.size() > 0)
                return (*mFilters.begin())->getGain();
            else
                return 0;
        }

        
    }
    
}

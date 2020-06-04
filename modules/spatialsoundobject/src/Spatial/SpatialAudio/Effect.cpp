#include "Effect.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::EffectBase)
    RTTI_PROPERTY("Name", &nap::spatial::EffectBase::mName, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::EffectInstanceBase)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::EffectProcessor)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::spatial::EffectChain)
    RTTI_PROPERTY("Effects", &nap::spatial::EffectChain::mEffects, nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::spatial::EffectChainInstance)
    RTTI_FUNCTION("addProcessorChain", &nap::spatial::EffectChainInstance::addProcessorChain)
RTTI_END_CLASS


namespace nap
{
    
    namespace spatial
    {
        
        
        std::unique_ptr<EffectChainInstance> EffectChain::instantiate(int channelCount, audio::AudioService& audioService, nap::utility::ErrorState& errorState, nap::EntityInstance* entity)
        {
            
            auto instance = std::make_unique<EffectChainInstance>();
            bool initSucceeded = instance->init(this, channelCount, audioService, errorState, entity);
            if(initSucceeded)
                return instance;
            else
                return nullptr;
        }
        
        
        bool EffectChainInstance::init(EffectChain* effectChain, int channelCount, audio::AudioService& audioService, nap::utility::ErrorState& errorState, nap::EntityInstance* entity)
        {
            
            // instantiate effect instances (which will instantiate parameters)
            for(auto& effect : effectChain->mEffects){
                
                mEffectInstances.push_back(effect->instantiate(channelCount, audioService, errorState, entity));
                
                // return false if the instantiation failed
                if(!errorState.check(mEffectInstances.back() != nullptr, "Instantiation of effect failed."))
                    return false;
            }
            
            return true;
        }
        
        
        audio::IMultiChannelOutput* EffectChainInstance::addProcessorChain(audio::IMultiChannelOutput& input, utility::ErrorState& errorState)
        {
            
            // if the effectchain is empty, just pass the input through as the output.
            if(mEffectInstances.empty())
            {
                mChainOutputs.emplace_back(&input);
                return &input;
            }
            
            // add input to first effect instances
            if (!mEffectInstances[0]->addProcessor(input, errorState))
            {
                errorState.fail("Failed to add processor chain.");
                return nullptr;
            }
            
            // add the output of the first effect, to the next effect, and so forth.
            for (int i = 1; i <= mEffectInstances.size() - 1; i++)
            {
                if (!mEffectInstances[i]->addProcessor(*mEffectInstances[i - 1]->getProcessorNonTyped(mProcessorCount), errorState))
                {
                    errorState.fail("Failed to add processor chain.");
                    return nullptr;
                }
            }
            
            // increase the number of processors counter
            mProcessorCount++;
            
            // return the final processor
            mChainOutputs.emplace_back(mEffectInstances.back()->getProcessorNonTyped(mProcessorCount - 1));
            return mChainOutputs.back();
        }
        
        
        void EffectChainInstance::update(double deltaTime)
        {
            for(auto& effectInstance : mEffectInstances)
                effectInstance->update(deltaTime);
        }
        
        
        audio::IMultiChannelOutput& EffectChainInstance::getChainOutput(int chainIndex)
        {
            assert(chainIndex < mChainOutputs.size());
            return *mChainOutputs[chainIndex];
        }


        
        EffectInstanceBase* EffectChainInstance::getEffectByName(std::string name)
        {
            for(auto& effect : mEffectInstances)
            {
                if(effect->getName() == name)
                    return effect.get();
            }
            return nullptr;
        }


        
    }
}

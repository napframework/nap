#include "FaustReverbEffect.h"

RTTI_DEFINE_CLASS(nap::spatial::FaustReverbEffect)

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::FaustReverbEffectProcessor)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::FaustReverbEffectInstance)
RTTI_END_CLASS

namespace nap
{
    namespace spatial
    {

        bool FaustReverbEffectInstance::onInit(EntityInstance* entity, utility::ErrorState& errorState)
        {
            mFeedback = getParameterManager().addParameterFloat("feedback", 0.5, 0.0, 1.0);
            mFeedback->connect([&, this](float value){
                for(int i = 0; i < getProcessorCount(); i++){
                    for(int j = 0; j < getChannelCount(); j++){
                        getProcessor(i)->getNode(j)->setFeedback1(value);
                    }
                }
            });

            mFeedback2 = getParameterManager().addParameterFloat("feedback2", 1.f, 0.0, 1.0);
            mFeedback2->connect([&, this](float value){
                for(int i = 0; i < getProcessorCount(); i++){
                    for(int j = 0; j < getChannelCount(); j++){
                        getProcessor(i)->getNode(j)->setFeedback2(value);
                    }
                }
            });

            mDamping = getParameterManager().addParameterFloat("damping", 0.5, 0.0, 1.0);
            mDamping->connect([&, this](float value){
                for(int i = 0; i < getProcessorCount(); i++){
                    for(int j = 0; j < getChannelCount(); j++){
                        getProcessor(i)->getNode(j)->setDamping(value);
                    }
                }
            });
            
            // dry wet parameter
            mDryWet = getParameterManager().addParameterFloat("dryWet", 0.0, 0.0, 1.0);
            mDryWet->connect([&](float value)
            {
                setDryWet(value);
            });

            setDryWet(1.f);
            return true;
        }


        void FaustReverbEffectInstance::update(double deltaTime)
        {
            auto activeParticleCount = getActiveParticleCount();
            if (activeParticleCount != mParticleCount)
            {
                mParticleCount = activeParticleCount;
                for (int i = 0; i < getProcessorCount(); i++)
                {
                    for (int j = 0; j < getChannelCount(); j++)
                    {
                        getProcessor(i)->getNode(j)->setSpread(j / float(getChannelCount()) * 100.f);
                    }

                }
            }
        }
        
        
        void FaustReverbEffectInstance::onProcessorAdded(FaustReverbEffectProcessor& processor)
        {
            for (int j = 0; j < getChannelCount(); j++)
            {
                processor.getNode(j)->setSpread(j / float(getChannelCount()) * 100.f);
                processor.getNode(j)->setDamping(mDamping->mValue);
                processor.getNode(j)->setFeedback1(mFeedback->mValue);
                processor.getNode(j)->setFeedback2(mFeedback2->mValue);
            }
        }


    }

}

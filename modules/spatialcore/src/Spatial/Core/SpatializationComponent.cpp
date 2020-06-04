#include "SpatializationComponent.h"

// Spatial includes
#include <Spatial/Core/SpatialTypes.h>
#include <Spatial/Core/SpatialService.h>

// Audio includes
#include <audio/core/audiopin.h>

// Nap includes
#include <transformcomponent.h>
#include <entity.h>
#include <nap/core.h>


// RTTI
using SpatialSoundComponentSignal = nap::Signal<nap::spatial::SpatializationComponentInstance&>;
RTTI_BEGIN_CLASS(SpatialSoundComponentSignal)
    RTTI_FUNCTION("connect", (void(SpatialSoundComponentSignal::*)(const pybind11::function))&SpatialSoundComponentSignal::connect)
RTTI_END_CLASS

using SpatialSoundComponentFloatSignal = nap::Signal<float>;
RTTI_BEGIN_CLASS(SpatialSoundComponentFloatSignal)
RTTI_FUNCTION("connect", (void(SpatialSoundComponentFloatSignal::*)(const pybind11::function))&SpatialSoundComponentFloatSignal::connect)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::spatial::SpatializationComponent)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::SpatializationComponentInstance)
    RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
    RTTI_FUNCTION("addParticle", &nap::spatial::SpatializationComponentInstance::addParticle)
    RTTI_FUNCTION("getParticle", &nap::spatial::SpatializationComponentInstance::getParticle)
    RTTI_FUNCTION("getParticleCount", &nap::spatial::SpatializationComponentInstance::getParticleCount)
    RTTI_FUNCTION("connectInput", &nap::spatial::SpatializationComponentInstance::connectInput)
    RTTI_FUNCTION("getInputs", &nap::spatial::SpatializationComponentInstance::getInputs)
    RTTI_FUNCTION("getOutputs", &nap::spatial::SpatializationComponentInstance::getOutputs)
    RTTI_FUNCTION("setInputGain", &nap::spatial::SpatializationComponentInstance::setInputGain)
    RTTI_FUNCTION("getInputGain", &nap::spatial::SpatializationComponentInstance::getInputGain)
    RTTI_FUNCTION("setDryGain", &nap::spatial::SpatializationComponentInstance::setDryGain)
    RTTI_FUNCTION("getDryGain", &nap::spatial::SpatializationComponentInstance::getDryGain)
    RTTI_FUNCTION("getInputConnectedSignal", &nap::spatial::SpatializationComponentInstance::getInputConnectedSignal)
    RTTI_FUNCTION("getOutputConnectedSignal", &nap::spatial::SpatializationComponentInstance::getOutputConnectedSignal)
    RTTI_FUNCTION("getInputGainChangedSignal", &nap::spatial::SpatializationComponentInstance::getInputGainChangedSignal)
    RTTI_FUNCTION("getDryGainChangedSignal", &nap::spatial::SpatializationComponentInstance::getDryGainChangedSignal)
RTTI_END_CLASS

namespace nap
{

    namespace spatial
    {

        SpatializationComponentInstance::SpatializationComponentInstance(EntityInstance& entity, Component& resource) : ComponentInstance(entity, resource)
        {
            mSpatialService = entity.getCore()->getService<SpatialService>();
        }


        SpatializationComponentInstance::~SpatializationComponentInstance()
        {
            mSpatialService->disconnectSpatializationComponent(*this);
        }


        bool SpatializationComponentInstance::init(utility::ErrorState& errorState)
        {
            mSpatialService->connectSpatializationComponent(*this);
			
			auto entity = getEntityInstance();
			
            mTransformComponent = entity->findComponent<TransformComponentInstance>();
			
            return true;
        }


        Particle* SpatializationComponentInstance::addParticle(audio::OutputPin* pin, const glm::vec3& position, const glm::vec4& rotation, const glm::vec3& scale, bool active)
        {
            if (!pin)
                return nullptr;
            mParticles.emplace_back(std::make_unique<Particle>(pin, position, rotation, scale, active));
            mParticleCreatedSignal(*this, *mParticles.back());
            return mParticles.back().get();
        }
		
		
        int SpatializationComponentInstance::calculateActiveParticleCount() const
        {
        	int result = 0;
        	for (auto & particle : mParticles)
        		if (particle->isActive())
        			result++;
			return result;
		}


        Particle* SpatializationComponentInstance::getParticle(unsigned int index) const
        {
            if (index < mParticles.size())
                return mParticles[index].get();
            else
                return nullptr;
        }


        std::vector<SpatializationComponentInstance*> SpatializationComponentInstance::getInputs() const
        {
            std::vector<SpatializationComponentInstance*> result;
            for (auto& input : mInputs)
                result.emplace_back(input->mInput);
            return result;
        }


        audio::ControllerValue SpatializationComponentInstance::getInputGain(SpatializationComponentInstance& aInput) const
        {
            for (auto& input : mInputs)
                if (input->mInput == &aInput)
                    return input->mGain;
            return 0;
        }


        void SpatializationComponentInstance::setInputGain(SpatializationComponentInstance& aInput, audio::ControllerValue gain)
        {
            for (auto& input : mInputs)
                if (input->mInput == &aInput)
                {
                    input->mGain = gain;
                    input->mInputGainChangedSignal(gain);
                }
        }


        void SpatializationComponentInstance::connectInput(SpatializationComponentInstance* newInput, audio::ControllerValue gain)
        {
            // Check for duplicates
            for (auto& input : mInputs)
                if (input->mInput == newInput)
                    return;

            auto input = std::make_unique<Input>(newInput, gain);
            mInputs.emplace_back(std::move(input));
            newInput->mOutputs.emplace(this);
            mInputConnectedSignal(*newInput);
            newInput->mOutputConnectedSignal(*this);
        }


        Signal<audio::ControllerValue>* SpatializationComponentInstance::getInputGainChangedSignal(SpatializationComponentInstance* aInput)
        {
            for (auto& input : mInputs)
                if (input->mInput == aInput)
                    return &input->mInputGainChangedSignal;
            return nullptr;
        }


        void SpatializationComponentInstance::setDryGain(audio::ControllerValue gain)
        {
            mDryGain = gain;
            mDryGainChangedSignal(mDryGain);
        }


    }

}

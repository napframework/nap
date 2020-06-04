#include "MeasurementComponent.h"

// Spatial includes
#include <Spatial/Core/SpatialTypes.h>
#include <Spatial/Core/ParameterComponent.h>
#include <Spatial/Core/ExposedDataComponent.h>
#include <Spatial/Core/SpatializationComponent.h>
#include <Spatial/Core/RootProcess.h>

// Nap includes
#include <entity.h>

// RTTI
RTTI_BEGIN_CLASS(nap::spatial::MeasurementComponent)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::MeasurementComponentInstance)
    RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{
    namespace spatial
    {
        
        ParticleMeasurer::ParticleMeasurer(Particle& particle, MeasurementComponentInstance& measurementComponent) : mParticle(particle), mMeasurementComponent(measurementComponent)
        {
            // recalculate data when transform changed.
            mParticle.getTransformChangedSignal()->connect(transformChangedSlot);
            
            // recalculate data when vantagepoint changed.
            mMeasurementComponent.getVantagePointChangedSignal()->connect(vantagePointChangedSlot);
            
            // recalculate height when groundlevel changed.
            mMeasurementComponent.getGroundLevelChangedSignal()->connect(groundLevelChangedSlot);

            recalculate();
        }


        const SpatialTransform& ParticleMeasurer::getTransform()
        {
            return mParticle.getTransform();
        }


        const glm::vec3& ParticleMeasurer::getPosition()
        {
            return mParticle.getPosition();
        }


        const glm::vec3& ParticleMeasurer::getDimensions()
        {
            return mParticle.getScale();
        }


        const glm::vec4& ParticleMeasurer::getRotation()
        {
            return mParticle.getRotation();
        }

        
        void ParticleMeasurer::recalculate()
        {
            mDistanceToVantagePoint = glm::distance(getVantagePoint(), mParticle.getPosition());
            recalculateHeight();
            recalculateInputRelatedData();
        }

        
        void ParticleMeasurer::recalculateInputRelatedData()
        {
            for (int i = 0; i < mInputTransforms.size(); i++)
            {
                mDistancesToInputs[i] = glm::distance(mInputTransforms[i]->mPosition, mParticle.getPosition());
            }
            
            mDataChangedSignal.trigger(*this);
        }
        
        void ParticleMeasurer::recalculateHeight(){
            mHeight = mParticle.getPosition().y - getGroundLevel();
        }


        glm::vec3 ParticleMeasurer::getVantagePoint()
        {
            return mMeasurementComponent.mVantagePoint->mValue;
        }
        
        float ParticleMeasurer::getGroundLevel()
        {
            return mMeasurementComponent.mGroundLevel->mValue;
        }
        
        
        void ParticleMeasurer::addInputTransform(const SpatialTransform* inputTransform)
        {
            mInputTransforms.push_back(inputTransform);
            mDistancesToInputs.resize(mInputTransforms.size());
            recalculateInputRelatedData();
        }


        void MeasurementComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
        {
            components.emplace_back(RTTI_OF(SpatializationComponent)); // for particles
            components.emplace_back(RTTI_OF(ParameterComponent));
            components.emplace_back(RTTI_OF(ExposedDataComponent));
        }

        
    
        bool MeasurementComponentInstance::init(utility::ErrorState& errorState)
        {
            auto& spatialSoundComponent = getEntityInstance()->getComponent<SpatializationComponentInstance>();
            spatialSoundComponent.getParticleCreatedSignal()->connect(mParticleCreatedSlot);
            
            auto& parameterComponent = getEntityInstance()->getComponent<ParameterComponentInstance>();

            auto& exposedData = getEntityInstance()->getComponent<ExposedDataComponentInstance>().getRoot();
            exposedData.expose<glm::vec3>("particlePositions", [&](
                    int index){ return mParticleMeasurers[index]->getPosition(); }, [&](){ return mActiveParticleCount.load(); });
            exposedData.expose<glm::vec3>("particleDimensions", [&](
                    int index){ return mParticleMeasurers[index]->getDimensions(); }, [&](){ return mActiveParticleCount.load(); });
            exposedData.expose<glm::vec3>("particleRotations", [&](
                    int index){ return mParticleMeasurers[index]->getRotation(); }, [&](){ return mActiveParticleCount.load(); });
            exposedData.expose<int>("particleCount", [&](){ return getActiveParticleCount(); });

            mVantagePoint = &parameterComponent.addParameterVec3("vantagePoint", glm::vec3(0.0), -9999, 9999);
            mVantagePoint->valueChanged.connect([&, this](const glm::vec3& value){ mVantagePointChangedSignal.trigger(value); } );
            
            mGroundLevel = &parameterComponent.addParameterFloat("floorLevel", 0.0, -100.0, 100.0);
            mGroundLevel->valueChanged.connect([&, this](float x){ mGroundLevelChangedSignal.trigger(x); } );

            return true;
        }
        
        
        void MeasurementComponentInstance::particleCreated(SpatializationComponentInstance& spatialSoundComponent, Particle& particle)
        {
            mParticleMeasurers.push_back(std::make_unique<ParticleMeasurer>(particle, *this));
            particle.getActiveChangedSignal()->connect(mParticleActivatedSlot);
        }

        
        void MeasurementComponentInstance::sourceCreated(SpatialSourceInstance& source)
        {
            // recalculate all input related data when input transform data changed
            Signal<const SpatialSourceInstance&>* sig = source.getDataChangedSignal();
            sig->connect(mInputDataChangedSlot);
                        
            // add input transform to each particle measurer
            for(int i = 0; i < mParticleMeasurers.size(); i++)
                mParticleMeasurers[i]->addInputTransform(source.getInputTransformForChannel(i));
            
        }


        void MeasurementComponentInstance::particleActivated(Particle& particle)
        {
            if (particle.isActive())
                mActiveParticleCount++;
            else
                mActiveParticleCount--;
        }




    }
}

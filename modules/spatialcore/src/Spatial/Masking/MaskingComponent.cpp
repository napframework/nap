#include "MaskingComponent.h"

#include "MaskingOccluderComponent.h"

// Spatial includes
#include <Spatial/Core/SpatialTypes.h>
#include <Spatial/Core/SpatialService.h>
#include <Spatial/Core/SpatializationComponent.h>

// NAP includes
#include <nap/core.h>
#include <entity.h>
#include <transformcomponent.h>

namespace nap
{
	namespace spatial
	{
		RTTI_BEGIN_CLASS(nap::spatial::MaskingComponent)
			RTTI_PROPERTY("PerParticle", &nap::spatial::MaskingComponent::mPerParticle, nap::rtti::EPropertyMetaData::Default)
		RTTI_END_CLASS
		
		RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::MaskingComponentInstance)
    		RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
    		RTTI_FUNCTION("addSourceWithOccluder", &nap::spatial::MaskingComponentInstance::addSourceWithOccluder)
    		RTTI_FUNCTION("sampleMask", &nap::spatial::MaskingComponentInstance::sampleMask)
    		RTTI_FUNCTION("getParticleMask", &nap::spatial::MaskingComponentInstance::getParticleMask)
		RTTI_END_CLASS

		//

		MaskingComponentInstance::MaskingComponentInstance(EntityInstance& entity, Component& resource)
			: ComponentInstance(entity, resource)
		{
		}
		
		MaskingComponentInstance::~MaskingComponentInstance()
		{
			auto spatialService = getEntityInstance()->getCore()->getService<SpatialService>();
			spatialService->unregisterMaskingComponent(*this);
		}
		
		bool MaskingComponentInstance::init(utility::ErrorState& errorState)
		{
			if (!ComponentInstance::init(errorState))
				return false;
			
			auto spatialService = getEntityInstance()->getCore()->getService<SpatialService>();
			spatialService->registerMaskingComponent(*this);
			
			return true;
		}
		
		void MaskingComponentInstance::updateMasking()
		{
			const MaskingComponent * component = getComponent<MaskingComponent>();
			
			for (MaskingSourceAndOccluder & elem : mMaskingSources)
			{
				const TransformComponentInstance * transformComponent = elem.source->findComponent<TransformComponentInstance>();
				
				if (transformComponent == nullptr)
				{
					elem.sourcePosition = glm::vec3();
				}
				else
				{
					elem.sourcePosition = transformComponent->getTranslate();
				}
				
				elem.sourceSoundObject = elem.source->findComponent<SpatializationComponentInstance>();
			}
			
			if (component->mPerParticle)
			{
				const EntityInstance * entity = getEntityInstance();
				
				if (entity->hasComponent<SpatializationComponentInstance>())
				{
					const SpatializationComponentInstance & soundObject = entity->getComponent<SpatializationComponentInstance>();
					const int numParticles = soundObject.getParticleCount();
					
					if (numParticles != mParticleMaskingValues.size())
					{
						mParticleMaskingValues.resize(numParticles);
					}
					
					for (int i = 0; i < numParticles; ++i)
					{
						float & maskingValue = mParticleMaskingValues[i];
						
						const Particle * particle = soundObject.getParticle(i);
						
						const glm::vec3 & target = particle->getPosition();
						
						maskingValue = sampleMask(target);
					}
				}
				else
				{
					mParticleMaskingValues.clear();
				}
			}
			else
			{
				glm::vec3 target;
				
				const nap::EntityInstance * entity = getEntityInstance();
				
				const TransformComponentInstance * transformComponent = entity->findComponent<TransformComponentInstance>();
				
				if (transformComponent != nullptr)
				{
					target = transformComponent->getTranslate();
				}
				
				mMaskingValue = sampleMask(target);
			}
		}
		
		void MaskingComponentInstance::addSourceWithOccluder(EntityInstance * source, bool sourceIsPerParticle, MaskingOccluderComponentInstance * occluder)
		{
			MaskingSourceAndOccluder elem;
			elem.source = source;
			elem.sourceIsPerParticle = sourceIsPerParticle;
			elem.occluder = occluder;
			
			mMaskingSources.push_back(elem);
		}
		
		float MaskingComponentInstance::sampleMask(const glm::vec3 & target) const
		{
			float result = 1.f;
			
			for (const MaskingSourceAndOccluder & elem : mMaskingSources)
			{
				if (elem.sourceIsPerParticle && elem.sourceSoundObject != nullptr && elem.sourceSoundObject->getParticleCount() > 0)
				{
					// calculate the average masking value and occlude
					
					const int numParticles = elem.sourceSoundObject->getParticleCount();
					
					float totalMask = 0.f;
					
					for (int i = 0; i < numParticles; ++i)
					{
						const Particle * particle = elem.sourceSoundObject->getParticle(i);
						
						const float mask = elem.occluder->sampleMask(particle->getPosition(), target);
						
						totalMask += mask;
					}
					
					const float averageMask = totalMask / numParticles;
					
					result *= 1.f - averageMask;
				}
				else
				{
					const float mask = elem.occluder->sampleMask(elem.sourcePosition, target);
					
					result *= 1.f - mask;
				}
			}
			
			return 1.f - result;
		}
		
		float MaskingComponentInstance::getParticleMask(const int particleIndex) const
		{
			const MaskingComponent * component = getComponent<MaskingComponent>();
			
			if (component->mPerParticle && particleIndex >= 0 && particleIndex < mParticleMaskingValues.size())
			{
				return mParticleMaskingValues[particleIndex];
			}
			else
			{
				return mMaskingValue;
			}
		}
	}
}

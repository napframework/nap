#pragma once

// Nap includes
#include <component.h>
#include <imagefromfile.h>
#include <entityptr.h>
#include <nap/resourceptr.h>

namespace nap
{
	namespace spatial
	{
		class MaskingComponent;
		class MaskingComponentInstance;
		class MaskingOccluderComponent;
		class MaskingOccluderComponentInstance;
		class SpatializationComponentInstance;
		
		/**
		 * MaskingComponent settings.
		 */
		class NAPAPI MaskingComponent : public Component
		{
			RTTI_ENABLE(Component)
			DECLARE_COMPONENT(MaskingComponent, MaskingComponentInstance)
			
		public:
			bool mPerParticle = false; ///< Property: PerParticle. When true, masking is performed for each SpatialSoundObject particle individually. Otherwise, the entity's position is used when casting rays.
		};
		
		/**
		 * The masking component provides services to set up masking (occlusion) between a source and destination entity.
		 * Masking can be performed full 'particle vs. particle', or the faster option, just between entity positions.
		 * Masking results can be queried from Python script, are always between 0.0 and 1.0, and represent a measure for the amount of occlusion for particles or the entity as a whole.
		 */
		class NAPAPI MaskingComponentInstance : public ComponentInstance
		{
			RTTI_ENABLE(ComponentInstance)
			
		public:
			MaskingComponentInstance(EntityInstance& entity, Component& resource);
			virtual ~MaskingComponentInstance() override;
			
			virtual bool init(utility::ErrorState& errorState) override;
			
			/**
			 * Performs ray-casting between entities, taking into account occluders, and updates the masking values for each particle or the entity as a whole.
			 */
			void updateMasking();
			
			/**
			 * Adds a source, occluded by an occluder, to integrate into the computed masking values.
			 * @param source: The source entity.
			 * @param sourceIsPerParticle: When sourceIsPerParticle is true, a ray is cast between each particle of source and our own entity or particles, and an average masking value is computed. Otherwise, the source entity's position is used as the origin for the ray-cast(s).
			 * @param occluder: The occlusion component. The occlusion component contains an occlusion mask which is intersected with rays during ray-casting, to determine the occlusion factors and compute the masking value(s).
			 */
			void addSourceWithOccluder(EntityInstance * source, bool sourceIsPerParticle, MaskingOccluderComponentInstance * occluder);
			
			/**
			 * Compute the masking value given the target position. This will iterate all registered <source, occluder> pairs and performs one or more ray-casts between the source and target. The result is the average masking value for target.
			 * @param target: The target location of the ray-cast(s) to perform.
			 */
			float sampleMask(const glm::vec3 & target) const;
			
			/**
			 * @param particleIndex: Index of the particle. Ignored when PerParticle is false.
			 * @return Returns the pre-computed masking value for the given particle, or for the entity if PerParticle is false. This method should be used from script for fast access to the computed masking values.
			 */
			float getParticleMask(const int particleIndex) const;
			
		private:
			/**
			 * Registered <source, occluder> pair registered through addSourceWithOccluder.
			 */
			struct MaskingSourceAndOccluder
			{
				EntityInstance * source = nullptr;                           ///< Ray-casting source.
				glm::vec3 sourcePosition;                                    ///< Source position. Evaluated once during each update, to avoid having to compute it for each ray-cast individually.
				SpatializationComponentInstance * sourceSoundObject = nullptr; ///< The sound object of the source entity. Fetched once during each update, to avoid having to fetch it for each ray-cast individually.
				bool sourceIsPerParticle = false;                            ///< Perform per-particle ray-casts between the source and target?
				MaskingOccluderComponentInstance * occluder = nullptr;       ///< Occluder object.
			};
			
			std::vector<MaskingSourceAndOccluder> mMaskingSources; ///< All registered <source, occluder> pairs.
			std::vector<float> mParticleMaskingValues;             ///< Computed per-particle masking values.
			float mMaskingValue = 0.f;                             ///< Computed masking value for the entity when PerParticle is false.
		};
	}
}

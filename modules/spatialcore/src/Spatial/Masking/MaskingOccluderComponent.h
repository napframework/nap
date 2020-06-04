#pragma once

// Nap includes
#include <component.h>
#include <imagefromfile.h>
#include <nap/resourceptr.h>

namespace nap
{
	namespace spatial
	{
		class MaskingOccluderComponent;
		class MaskingOccluderComponentInstance;
		
		/**
		 * MaskingOccluderComponent properties.
		 */
		class NAPAPI MaskingOccluderComponent : public Component
		{
			RTTI_ENABLE(Component)
			DECLARE_COMPONENT(MaskingOccluderComponent, MaskingOccluderComponentInstance)
			
		public:
			nap::ResourcePtr<nap::ImageFromFile> mImage = nullptr; ///< Image to be used as a mask. Areas where the image is white (more luminous) are interpreted as being more opaque and less transparent, yielding a higher masking value as a result.
		};
		
		class NAPAPI MaskingOccluderComponentInstance : public ComponentInstance
		{
			RTTI_ENABLE(ComponentInstance)
			
		public:
			MaskingOccluderComponentInstance(EntityInstance& entity, Component& resource);
			virtual ~MaskingOccluderComponentInstance() override;
			
			virtual bool init(utility::ErrorState& errorState) override;
			
			/**
			 * Updates the 'world to entity' transform once, before performing ray-casts.
			 */
			void updateTransform();
			
			/**
			 * Sample the occlusion mask using pixel coordinates.
			 * @param x: X-coordinate of the occlusion mask to sample.
			 * @param y: Y-coordinate of the occlusion mask to sample.
			 * @return Occlusion amount.
			 */
			float sampleMask2D_nearest(const int x, const int y) const;
			
			/**
			 * Sample the occlusion mask using pixel coordinates and bi-linear interpolation.
			 * @param x: X-coordinate of the occlusion mask to sample.
			 * @param y: Y-coordinate of the occlusion mask to sample.
			 * @return Occlusion amount.
			 */
			float sampleMask2D_linear(const float x, const float y) const;
			
			/**
			 * Sample the occlusion mask using a ray-cast between source and target. When the ray doesn't intersect the occlusion mask, the result is always zero (no occlusion).
			 * @param source: The source position of the ray-cast.
			 * @param target: The target position of the ray-cast.
			 * @return Occlusion amount.
			 */
			float sampleMask(const glm::vec3 & source, const glm::vec3 & target) const;
			
		private:
			float * mMaskValues = nullptr; ///< Image converted into occlusion mask values (luminosity in the range of 0.0 to 1.0).
			int mMaskSx = 0;               ///< Horizontal size of the occlusion mask.
			int mMaskSy = 0;               ///< Vertical size of the occlusion mask.
			
		// todo : would be nice if this were world to image directly
			glm::mat4 mWorldToEntity;      ///< Transform of the parent entity, pre-computed during update.
		};
	}
}

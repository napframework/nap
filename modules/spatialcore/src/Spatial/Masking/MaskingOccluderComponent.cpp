#include "MaskingOccluderComponent.h"

// Spatial includes
#include <Spatial/Core/SpatialService.h>

// NAP includes
#include <nap/core.h>
#include <entity.h>
#include <transformcomponent.h>

namespace nap
{
	namespace spatial
	{
		RTTI_BEGIN_CLASS(nap::spatial::MaskingOccluderComponent)
			RTTI_PROPERTY("Image", &nap::spatial::MaskingOccluderComponent::mImage, nap::rtti::EPropertyMetaData::Required)
		RTTI_END_CLASS
		
		RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::MaskingOccluderComponentInstance)
    		RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
    		RTTI_FUNCTION("sampleMask", &nap::spatial::MaskingOccluderComponentInstance::sampleMask)
		RTTI_END_CLASS
		
		//

		MaskingOccluderComponentInstance::MaskingOccluderComponentInstance(EntityInstance& entity, Component& resource)
			: ComponentInstance(entity, resource)
		{
		}
		
		MaskingOccluderComponentInstance::~MaskingOccluderComponentInstance()
		{
			delete [] mMaskValues;
			mMaskValues = nullptr;
			
			auto spatialService = getEntityInstance()->getCore()->getService<SpatialService>();
			spatialService->unregisterMaskingOccluderComponent(*this);
		}
		
		bool MaskingOccluderComponentInstance::init(utility::ErrorState& errorState)
		{
			const MaskingOccluderComponent * maskingOccluderComponent = getComponent<MaskingOccluderComponent>();
			
			if (!errorState.check(maskingOccluderComponent->mImage != nullptr, "Image not set"))
				return false;
			
			const nap::Bitmap * bitmap = &maskingOccluderComponent->mImage->getBitmap();
			
			mMaskSx = bitmap->getWidth();
			mMaskSy = bitmap->getHeight();
			
			mMaskValues = new float[mMaskSx * mMaskSy];
			
			auto color = bitmap->makePixel();
			
			for (int x = 0; x < mMaskSx; ++x)
			{
				for (int y = 0; y < mMaskSy; ++y)
				{
					bitmap->getPixel(x, y, *color);
					
					float luminance = 0.f;
					
					if (color->getNumberOfChannels() >= 3)
					{
						const nap::RGBColorFloat temp = color->convert<nap::RGBColorFloat>();
						
						luminance = (temp.getRed() + temp.getGreen() + temp.getBlue()) / 3.f;
					}
					else if (color->getNumberOfChannels() >= 1)
					{
						const nap::RColorFloat temp = color->convert<nap::RColorFloat>();
						
						luminance = temp.getRed();
					}
					
					mMaskValues[x + y * mMaskSx] = luminance;
				}
			}
			
			auto spatialService = getEntityInstance()->getCore()->getService<SpatialService>();
			spatialService->registerMaskingOccluderComponent(*this);
			
			return true;
		}
		
		void MaskingOccluderComponentInstance::updateTransform()
		{
			const nap::EntityInstance * entity = getEntityInstance();
			
			const TransformComponentInstance * transformComponent = entity->findComponent<TransformComponentInstance>();
			
			if (transformComponent == nullptr)
			{
				mWorldToEntity = glm::mat4();
			}
			else
			{
				const glm::mat4 & entityToWorld = transformComponent->getGlobalTransform();
				const glm::mat4 worldToEntity = glm::inverse(entityToWorld);
				
				mWorldToEntity = worldToEntity;
			}
		}
		
		float MaskingOccluderComponentInstance::sampleMask2D_nearest(const int _x, const int _y) const
		{
			const int x = _x < 0 ? 0 : _x >= mMaskSx ? mMaskSx - 1 : _x;
			const int y = _y < 0 ? 0 : _y >= mMaskSy ? mMaskSy - 1 : _y;
			
			return mMaskValues[x + y * mMaskSx];
		}
		
		float MaskingOccluderComponentInstance::sampleMask2D_linear(const float x, const float y) const
		{
			const int x1 = std::floor(x);
			const int y1 = std::floor(y);
			const int x2 = x1 + 1;
			const int y2 = y1 + 1;
			
			const float tx2 = x - x1;
			const float tx1 = 1.f - tx2;
			
			const float ty2 = y - y1;
			const float ty1 = 1.f - ty2;
			
			const float l11 = sampleMask2D_nearest(x1, y1);
			const float l21 = sampleMask2D_nearest(x2, y1);
			const float l12 = sampleMask2D_nearest(x1, y2);
			const float l22 = sampleMask2D_nearest(x2, y2);
			
			const float l1 = l11 * tx1 + l21 * tx2;
			const float l2 = l12 * tx1 + l22 * tx2;
			
			const float l = l1 * ty1 + l2 * ty2;
			
			return l;
		}
		
		float MaskingOccluderComponentInstance::sampleMask(const glm::vec3 & _source, const glm::vec3 & _target) const
		{
			const glm::vec3 source = mWorldToEntity * glm::vec4(_source, 1.f);
			const glm::vec3 target = mWorldToEntity * glm::vec4(_target, 1.f);
			
			const float t1 = source[2];
			const float t2 = target[2];
			
			const float eps = 1e-6f;
			
			// if both points are on the same side of the image plane, there is no intersection and the masking result is zero
			
			if (t1 <= +eps && t2 <= +eps)
				return 0.f;
			if (t1 >= -eps && t2 >= -eps)
				return 0.f;
			
			// compute the intersection point with the image plane
			
			const float t = -t1 / (t2 - t1);
			
			const glm::vec3 p = (source + (target - source) * t);
			
			// if the intersection point lies outside of the image bounds, there is no intersectionand the masking result is zero
			
			if (p[0] < -.5f || p[0] >= +.5f)
				return 0.f;
			if (p[1] < -.5f || p[1] >= +.5f)
				return 0.f;
			
		// todo : is the -1 correct ? test with a really small masking image
			const float x = (p[0] + .5f) * (mMaskSx - 1);
			const float y = (p[1] + .5f) * (mMaskSy - 1);
			
			const float result =  sampleMask2D_linear(x, y);
			
			//printf("%.2f %.2f > %.2f\n", p[0], p[1], result);
			
			return result;
		}
	}
}

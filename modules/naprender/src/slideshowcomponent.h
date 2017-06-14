#pragma once

#include "nap/entity.h"
#include "imageresource.h"
#include <glm/glm.hpp>

namespace nap
{
	class SlideShowComponent;

	class SlideShowComponentResource : public ComponentResource
	{
		RTTI_ENABLE(ComponentResource)

		virtual const rtti::TypeInfo getInstanceType() const
		{
			return RTTI_OF(SlideShowComponent);
		}

	public:
 		std::vector<ObjectPtr<nap::ImageResource>>		mImages;
		ObjectPtr<nap::EntityResource>					mEntityPrototype;
	};

	class SlideShowComponent : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)

	public:
		SlideShowComponent(EntityInstance& entity);

		/**
		 * 
		 */
		virtual bool init(const ObjectPtr<ComponentResource>& resource, utility::ErrorState& errorState);

		void update(double deltaTime);

		void cycleLeft();
		void cycleRight();

	private:
		void assignTexture(nap::EntityInstance& entity, int imageIndex);
		void setTranslate(nap::EntityInstance& entity, const glm::vec3& translate);
		void Switch();
		void setVisible(nap::EntityInstance& entity, bool visible);

	private:
		ObjectPtr<SlideShowComponentResource>		mResource;
		ObjectPtr<nap::EntityInstance>				mLeftChildInstance;
		ObjectPtr<nap::EntityInstance>				mCenterChildInstance;
		ObjectPtr<nap::EntityInstance>				mRightChildInstance;
		int											mImageIndex = 0;
		int											mTargetImageIndex = 0;
		double										mTimer;
	};
}

#pragma once

#include "nap/componentinstance.h"
#include "nap/entityinstance.h"
#include "imageresource.h"
#include <glm/glm.hpp>

namespace nap
{
	class SlideShowComponent;

	/**
	 * Resource for SlideShowComponent, holds static data from file.
	 */
	class SlideShowComponentResource : public ComponentResource
	{
		RTTI_ENABLE(ComponentResource)

		/**
		 * @return Instance type to create for this resource.
		 */
		virtual const rtti::TypeInfo getInstanceType() const
		{
			return RTTI_OF(SlideShowComponent);
		}

	public:
 		std::vector<ObjectPtr<nap::ImageResource>>		mImages;			///< Array of images to display in the slidesho2
		ObjectPtr<nap::EntityResource>					mEntityPrototype;	///< Prototype of entity to instantiate during Instance::init
	};

	/**
	 * SlideShowComponent can cycle through a list of images.
	 * Internally it spawns three entities based on a prototype. The textures are switched when needed by setting
	 * MaterialInstance properties.
	 * It is assumed that any clipping that is required is done a different system, for instance, the layout system.
	 */
	class SlideShowComponent : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)

	public:
		SlideShowComponent(EntityInstance& entity);

		/**
		 * Spawns left/center/right children, sets intial textures and positions.
		 * @return false if prototype does not match requirements.
		 */
		virtual bool init(const ObjectPtr<ComponentResource>& resource, EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState) override;

		/**
		 * Updates animation.
		 */
		virtual void update(double deltaTime) override;

		/**
		 * Cycles one element to the left. When reaching the beginning it will cycle to the end of the list.
		 */
		void cycleLeft();

		/**
		* Cycles one element to the right. When reaching the end it will cycle to the beginning of the list.
		*/
		void cycleRight();

	private:
		void assignTexture(nap::EntityInstance& entity, int imageIndex);
		void setTranslate(nap::EntityInstance& entity, const glm::vec3& translate);
		void Switch();
		void setVisible(nap::EntityInstance& entity, bool visible);

	private:
		ObjectPtr<SlideShowComponentResource>		mResource;				// Stati resource data
		ObjectPtr<nap::EntityInstance>				mLeftChildInstance;		// Instance of left child
		ObjectPtr<nap::EntityInstance>				mCenterChildInstance;	// Instance of center child
		ObjectPtr<nap::EntityInstance>				mRightChildInstance;	// Instance of right child
		int											mImageIndex = 0;		// Current image index
		int											mTargetImageIndex = 0;	// Target image we want to cycle to
		double										mTimer;					// Timer, used for animation
	};
}

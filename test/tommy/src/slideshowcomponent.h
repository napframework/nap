#pragma once

#include "nap/component.h"
#include "nap/entity.h"
#include "image.h"
#include <glm/glm.hpp>
#include <nap/entityptr.h>

namespace nap
{
	class SlideShowComponentInstance;

	/**
	 * Resource for SlideShowComponent, holds static data from file.
	 */
	class SlideShowComponent : public Component
	{
		RTTI_ENABLE(Component)

		/**
		 * @return Instance type to create for this resource.
		 */
		virtual const rtti::TypeInfo getInstanceType() const
		{
			return RTTI_OF(SlideShowComponentInstance);
		}

	public:
 		std::vector<ObjectPtr<nap::Image>>		mImages;			///< Array of images to display in the slidesho2
		ObjectPtr<nap::Entity>					mEntityPrototype;	///< Prototype of entity to instantiate during Instance::init
	};

	/**
	 * SlideShowComponent can cycle through a list of images.
	 * Internally it spawns three entities based on a prototype. The textures are switched when needed by setting
	 * MaterialInstance properties.
	 * It is assumed that any clipping that is required is done a different system, for instance, the layout system.
	 */
	class SlideShowComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)

	public:
		SlideShowComponentInstance(EntityInstance& entity, Component& resource);

		/**
		 * Spawns left/center/right children, sets intial textures and positions.
		 * @return false if prototype does not match requirements.
		 */
		virtual bool init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState) override;

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
		ObjectPtr<nap::EntityInstance>				mLeftChildInstance;		// Instance of left child
		ObjectPtr<nap::EntityInstance>				mCenterChildInstance;	// Instance of center child
		ObjectPtr<nap::EntityInstance>				mRightChildInstance;	// Instance of right child
		int											mImageIndex = 0;		// Current image index
		int											mTargetImageIndex = 0;	// Target image we want to cycle to
		double										mTimer;					// Timer, used for animation
	};
}

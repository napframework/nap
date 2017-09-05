#include "slideshowcomponent.h"
#include "nap/core.h"
#include "nap/resourcemanager.h"
#include <nap/entity.h>
#include "renderablemeshcomponent.h"
#include "transformcomponent.h"

RTTI_BEGIN_CLASS(nap::SlideShowComponent)
	RTTI_PROPERTY("Images",				&nap::SlideShowComponent::mImages,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("EntityPrototype",	&nap::SlideShowComponent::mEntityPrototype, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("ComponentPointer",	 &nap::SlideShowComponent::mComponentPointer, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SlideShowComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{
	static const float imageDistance = 0.8f;

	SlideShowComponentInstance::SlideShowComponentInstance(EntityInstance& entity, Component& resource) :
		ComponentInstance(entity, resource)
	{
	}


	bool SlideShowComponentInstance::init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState)
	{
		ResourceManagerService& resource_manager = *getEntity()->getCore()->getService<nap::ResourceManagerService>();

 		SlideShowComponent* resource = getResource<SlideShowComponent>();

		// Prototype needs RenderableMeshComponent
		if (!errorState.check(resource->mEntityPrototype->hasComponent<RenderableMeshComponent>(), "Entity prototype is missing RenderableMeshComponent"))
			return false;

		// Prototype needs TransformComponent
		if (!errorState.check(resource->mEntityPrototype->hasComponent<TransformComponent>(), "Entity prototype is missing TransformComponent"))
			return false;

		// Spawn left child
		mLeftChildInstance = resource_manager.createEntity(*resource->mEntityPrototype, entityCreationParams, errorState);
		if (mLeftChildInstance == nullptr)
			return false;
		getEntity()->addChild(*mLeftChildInstance);

		// Spawn center child
 		mCenterChildInstance = resource_manager.createEntity(*resource->mEntityPrototype, entityCreationParams, errorState);
 		if (mCenterChildInstance == nullptr)
 			return false;
		getEntity()->addChild(*mCenterChildInstance);

		// Spawn right child
		mRightChildInstance = resource_manager.createEntity(*resource->mEntityPrototype, entityCreationParams, errorState);
		if (mRightChildInstance == nullptr)
			return false;
		getEntity()->addChild(*mRightChildInstance);

		// Perform initial switch to set all textures and positions correctly
		Switch();

		return true;
	}

	/** 
	 * Sets correct textures for left/center/right children based on mImageIndex
	 * Sets positions based on start position.
	 */
	void SlideShowComponentInstance::Switch()
	{
		assignTexture(*mLeftChildInstance, mImageIndex - 1);
		setTranslate(*mLeftChildInstance, glm::vec3(-imageDistance, 0.0f, 0.0f));
		setVisible(*mLeftChildInstance, false);
		assignTexture(*mCenterChildInstance, mImageIndex);
		setTranslate(*mCenterChildInstance, glm::vec3(0.0f, 0.0f, 0.0f));
		setVisible(*mCenterChildInstance, true);
		assignTexture(*mRightChildInstance, mImageIndex + 1);
		setTranslate(*mRightChildInstance, glm::vec3(imageDistance, 0.0f, 0.0f));
		setVisible(*mRightChildInstance, false);
	}


	void SlideShowComponentInstance::update(double deltaTime)
	{
		if (mTargetImageIndex != mImageIndex)
		{
			SlideShowComponent* resource = getResource<SlideShowComponent>();

			mTimer += deltaTime;
			const float scroll_time = 0.7f;
			const float timeScale = mTimer / scroll_time;

			if (timeScale >= 1.0f)
			{
				mImageIndex = mTargetImageIndex;
				if (mImageIndex < 0)
					mImageIndex = resource->mImages.size() - 1;
				else if (mImageIndex > resource->mImages.size() - 1)
					mImageIndex = 0;
				mTargetImageIndex = mImageIndex;

				Switch();
			}
			else
			{
				float dir = -1.0f;
				if (mTargetImageIndex < mImageIndex)
					dir = 1.0f;

				const float translate = imageDistance * dir * sin(timeScale * glm::half_pi<float>());

				setTranslate(*mLeftChildInstance, glm::vec3(translate - imageDistance, 0.0f, 0.0f));
				setTranslate(*mCenterChildInstance, glm::vec3(translate, 0.0f, 0.0f));
				setTranslate(*mRightChildInstance, glm::vec3(translate + imageDistance, 0.0f, 0.0f));
			}
		}
	}


	void SlideShowComponentInstance::assignTexture(nap::EntityInstance& entity, int imageIndex)
	{
		SlideShowComponent* resource = getResource<SlideShowComponent>();

		if (imageIndex < 0)
			imageIndex = resource->mImages.size() - 1;
		else if (imageIndex > resource->mImages.size() - 1)
			imageIndex = 0;

		RenderableMeshComponentInstance& renderable = entity.getComponent<RenderableMeshComponentInstance>();
		MaterialInstance& material_instance = renderable.getMaterialInstance();
		material_instance.getOrCreateUniform<nap::UniformTexture2D>("mTexture").setTexture(*resource->mImages[imageIndex]);
	}


	void SlideShowComponentInstance::setVisible(nap::EntityInstance& entity, bool visible)
	{
		RenderableMeshComponentInstance& renderable = entity.getComponent<RenderableMeshComponentInstance>();
		renderable.setVisible(visible);
	}


	void SlideShowComponentInstance::setTranslate(nap::EntityInstance& entity, const glm::vec3& translate)
	{
		TransformComponentInstance& transform = entity.getComponent<TransformComponentInstance>();
		transform.setTranslate(translate);
	}


	void SlideShowComponentInstance::cycleLeft()
	{
		if (mImageIndex == mTargetImageIndex)
		{
			--mTargetImageIndex;
			mTimer = 0.0f;
			setVisible(*mLeftChildInstance, true);
			setVisible(*mCenterChildInstance, true);
			setVisible(*mRightChildInstance, true);
		}
	}

	void SlideShowComponentInstance::cycleRight()
	{
		if (mImageIndex == mTargetImageIndex)
		{
			++mTargetImageIndex;
			mTimer = 0.0f;
			setVisible(*mLeftChildInstance, true);
			setVisible(*mCenterChildInstance, true);
			setVisible(*mRightChildInstance, true);
		}
	}
}


#include "slideshowcomponent.h"
#include "nap/core.h"
#include "nap/resourcemanager.h"
#include "renderablemeshcomponent.h"
#include "transformcomponent.h"

RTTI_BEGIN_CLASS(nap::SlideShowComponentResource)
	RTTI_PROPERTY("Images",				&nap::SlideShowComponentResource::mImages,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("EntityPrototype",	&nap::SlideShowComponentResource::mEntityPrototype, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_CONSTRUCTOR1(nap::SlideShowComponent, nap::EntityInstance&)
RTTI_END_CLASS

namespace nap
{
	static const float imageDistance = 0.8f;

	SlideShowComponent::SlideShowComponent(EntityInstance& entity) :
		ComponentInstance(entity)
	{
	}


	bool SlideShowComponent::init(const ObjectPtr<ComponentResource>& resource, utility::ErrorState& errorState)
	{
		ResourceManagerService& resource_manager = *getEntity()->getCore()->getService<nap::ResourceManagerService>();

 		mResource = rtti_cast<SlideShowComponentResource>(resource.get());

		// Prototype needs RenderableMeshComponent
		if (!errorState.check(mResource->mEntityPrototype->hasComponent<RenderableMeshComponentResource>(), "Entity prototype is missing RenderableMeshComponent"))
			return false;

		// Prototype needs TransformComponent
		if (!errorState.check(mResource->mEntityPrototype->hasComponent<TransformComponentResource>(), "Entity prototype is missing TransformComponent"))
			return false;

		// Spawn left child
		mLeftChildInstance = resource_manager.createEntity(*mResource->mEntityPrototype, "SlideShowLeftEntity", errorState);
		if (mLeftChildInstance == nullptr)
			return false;
		getEntity()->addChild(*mLeftChildInstance);

		// Spawn center child
 		mCenterChildInstance = resource_manager.createEntity(*mResource->mEntityPrototype, "SlideShowCenterEntity", errorState);
 		if (mCenterChildInstance == nullptr)
 			return false;
		getEntity()->addChild(*mCenterChildInstance);

		// Spawn right child
		mRightChildInstance = resource_manager.createEntity(*mResource->mEntityPrototype, "SlideShowRightEntity", errorState);
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
	void SlideShowComponent::Switch()
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


	void SlideShowComponent::update(double deltaTime)
	{
		if (mTargetImageIndex != mImageIndex)
		{
			mTimer += deltaTime;
			const float scroll_time = 0.7f;
			const float timeScale = mTimer / scroll_time;

			if (timeScale >= 1.0f)
			{
				mImageIndex = mTargetImageIndex;
				if (mImageIndex < 0)
					mImageIndex = mResource->mImages.size() - 1;
				else if (mImageIndex > mResource->mImages.size() - 1)
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


	void SlideShowComponent::assignTexture(nap::EntityInstance& entity, int imageIndex)
	{
		if (imageIndex < 0)
			imageIndex = mResource->mImages.size() - 1;
		else if (imageIndex > mResource->mImages.size() - 1)
			imageIndex = 0;

		RenderableMeshComponent& renderable = entity.getComponent<RenderableMeshComponent>();
		MaterialInstance& material_instance = renderable.getMaterialInstance();
		material_instance.getOrCreateUniform<nap::UniformTexture2D>("mTexture").setTexture(*mResource->mImages[imageIndex]);
	}


	void SlideShowComponent::setVisible(nap::EntityInstance& entity, bool visible)
	{
		RenderableMeshComponent& renderable = entity.getComponent<RenderableMeshComponent>();
		renderable.setVisible(visible);
	}


	void SlideShowComponent::setTranslate(nap::EntityInstance& entity, const glm::vec3& translate)
	{
		TransformComponent& transform = entity.getComponent<TransformComponent>();
		transform.setTranslate(translate);
	}


	void SlideShowComponent::cycleLeft()
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

	void SlideShowComponent::cycleRight()
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


// Local Includes
#include "orthocameracomponent.h"

// External Includes
#include <glm/gtc/matrix_transform.hpp> 
#include "transformcomponent.h"

RTTI_BEGIN_CLASS(nap::OrthoCameraProperties)
	RTTI_PROPERTY("NearClippingPlane",	&nap::OrthoCameraProperties::mNearClippingPlane,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FarClippingPlane",	&nap::OrthoCameraProperties::mFarClippingPlane,		nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::OrthoCameraComponentResource)
	RTTI_PROPERTY("Properties",			&nap::OrthoCameraComponentResource::mProperties,	nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_CONSTRUCTOR1(nap::OrthoCameraComponent, nap::EntityInstance&)
RTTI_END_CLASS

namespace nap
{
	// Hook up attribute changes
	OrthoCameraComponent::OrthoCameraComponent(EntityInstance& entity) :
		CameraComponent(entity)
	{
	}

	bool OrthoCameraComponent::init(const ObjectPtr<ComponentResource>& resource, utility::ErrorState& errorState)
	{
		mProperties = rtti_cast<OrthoCameraComponentResource>(resource.get())->mProperties;
		mTransformComponent = getEntity()->findComponent<TransformComponent>();
		if (!errorState.check(mTransformComponent != nullptr, "Missing transform component"))
			return false;

		return true;
	}


	// Set camera aspect ratio derived from width and height
	void OrthoCameraComponent::setAspectRatio(float width, float height)
	{
		mAspectRatio = width / height;
		setDirty();
	}

	// Computes projection matrix if dirty, otherwise returns the
	// cached version
	const glm::mat4& OrthoCameraComponent::getProjectionMatrix() const
	{
		if (mDirty)
		{
			mProjectionMatrix = glm::ortho(-1.0f * mAspectRatio, 1.0f * mAspectRatio, -1.0f, 1.0f, 0.0f, 1000.0f);
			mDirty = false;
		}

		return mProjectionMatrix;
	}

	const glm::mat4 OrthoCameraComponent::getViewMatrix() const
	{
		const glm::mat4& global_transform = mTransformComponent->getGlobalTransform();
		return glm::inverse(global_transform);
	}
}
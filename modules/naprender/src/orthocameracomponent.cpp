// Local Includes
#include "orthocameracomponent.h"
#include <nap/entity.h>

// External Includes
#include <glm/gtc/matrix_transform.hpp> 
#include "transformcomponent.h"

RTTI_BEGIN_CLASS(nap::OrthoCameraProperties)
	RTTI_PROPERTY("NearClippingPlane",	&nap::OrthoCameraProperties::mNearClippingPlane,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FarClippingPlane",	&nap::OrthoCameraProperties::mFarClippingPlane,		nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::OrthoCameraComponent)
	RTTI_PROPERTY("Properties",			&nap::OrthoCameraComponent::mProperties,	nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::OrthoCameraComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{
	// Hook up attribute changes
	OrthoCameraComponentInstance::OrthoCameraComponentInstance(EntityInstance& entity, Component& resource) :
		CameraComponentInstance(entity, resource)
	{
	}


	bool OrthoCameraComponentInstance::init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState)
	{
		mProperties = getComponent<OrthoCameraComponent>()->mProperties;
		mTransformComponent =	getEntityInstance()->findComponent<TransformComponentInstance>();
		if (!errorState.check(mTransformComponent != nullptr, "Missing transform component"))
			return false;

		return true;
	}


	// Set camera aspect ratio derived from width and height
	void OrthoCameraComponentInstance::setRenderTargetSize(glm::ivec2 size)
	{
		if (mRenderTargetSize != size)
		{
			mRenderTargetSize = size;
			setDirty();
		}
	}


	// Computes projection matrix if dirty, otherwise returns the
	// cached version
	const glm::mat4& OrthoCameraComponentInstance::getProjectionMatrix() const
	{
		if (mDirty)
		{
			mProjectionMatrix = glm::ortho(0.0f, (float)mRenderTargetSize.x, (float)mRenderTargetSize.y, 0.0f, mProperties.mNearClippingPlane, mProperties.mFarClippingPlane);
			mDirty = false;
		}

		return mProjectionMatrix;
	}


	const glm::mat4 OrthoCameraComponentInstance::getViewMatrix() const
	{
		const glm::mat4& global_transform = mTransformComponent->getGlobalTransform();
		return glm::inverse(global_transform);
	}
}
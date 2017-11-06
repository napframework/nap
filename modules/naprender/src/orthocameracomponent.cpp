// Local Includes
#include "orthocameracomponent.h"
#include <nap/entity.h>

// External Includes
#include <glm/gtc/matrix_transform.hpp> 
#include "transformcomponent.h"

RTTI_BEGIN_CLASS(nap::OrthoCameraProperties)
	RTTI_PROPERTY("LeftPlane",			&nap::OrthoCameraProperties::mLeftPlane,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("RightPlane",			&nap::OrthoCameraProperties::mRightPlane,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("TopPlane",			&nap::OrthoCameraProperties::mTopPlane,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("BottomPlane",		&nap::OrthoCameraProperties::mBottomPlane,			nap::rtti::EPropertyMetaData::Default)
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


	void OrthoCameraComponentInstance::setRenderTargetSize(glm::ivec2 size)
	{
		if (size != getRenderTargetSize())
		{
			CameraComponentInstance::setRenderTargetSize(size);
			setDirty();
		}
	}


	void OrthoCameraComponentInstance::setProperties(const OrthoCameraProperties& properties)
	{
		mProperties = properties;
		setDirty();
	}


	void OrthoCameraComponentInstance::setMode(EMode mode)
	{
		if (mMode != mode)
		{
			mMode = mode;
			setDirty();
		}
	}


	// Computes projection matrix if dirty, otherwise returns the
	// cached version
	const glm::mat4& OrthoCameraComponentInstance::getProjectionMatrix() const
	{
		if (mDirty)
		{
			switch (mMode)
			{
				case EMode::PixelSpace:
				{
					// In this mode we use the rendertarget size to set the left/right/top/bottom planes.
					glm::ivec2 render_target_size = getRenderTargetSize();
					mProjectionMatrix = glm::ortho(0.0f, (float)render_target_size.x, (float)render_target_size.y, 0.0f, mProperties.mNearClippingPlane, mProperties.mFarClippingPlane);
					break;
				}
				case EMode::CorrectAspectRatio:
				{
					// In this mode, we scale the top and bottom planes based on the aspect ratio
					glm::ivec2 renderTargetSize = getRenderTargetSize();
					float aspect_ratio = (float)renderTargetSize.y / (float)renderTargetSize.x;
					float top_plane = mProperties.mTopPlane * aspect_ratio;
					float bottom_plane = mProperties.mBottomPlane * aspect_ratio;

					mProjectionMatrix = glm::ortho(mProperties.mLeftPlane, mProperties.mRightPlane, bottom_plane, top_plane, mProperties.mNearClippingPlane, mProperties.mFarClippingPlane);
				}
				case EMode::Custom:
				{
					mProjectionMatrix = glm::ortho(mProperties.mLeftPlane, mProperties.mRightPlane, mProperties.mBottomPlane, mProperties.mTopPlane, mProperties.mNearClippingPlane, mProperties.mFarClippingPlane);
				}
			}

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
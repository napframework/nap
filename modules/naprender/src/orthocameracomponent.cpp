// Local Includes
#include "orthocameracomponent.h"

// External Includes
#include <glm/gtc/matrix_transform.hpp> 
#include "transformcomponent.h"
#include <entity.h>

RTTI_BEGIN_ENUM(nap::EOrthoCameraMode)
	RTTI_ENUM_VALUE(nap::EOrthoCameraMode::PixelSpace,			"PixelSpace"),
	RTTI_ENUM_VALUE(nap::EOrthoCameraMode::CorrectAspectRatio,	"CorrectAspectRatio"),
	RTTI_ENUM_VALUE(nap::EOrthoCameraMode::Custom,				"Custom")
RTTI_END_ENUM

RTTI_BEGIN_CLASS(nap::OrthoCameraProperties)
	RTTI_PROPERTY("Mode",				&nap::OrthoCameraProperties::mMode,					nap::rtti::EPropertyMetaData::Default)
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
	glm::mat4 OrthoCameraComponentInstance::createRenderProjectionMatrix(float left, float right, float bottom, float top, float zNear, float zFar)
	{
		glm::mat4x4 o_matrix(1);
		o_matrix[0][0] = static_cast<float>(2) / (right - left);
		o_matrix[1][1] = static_cast<float>(2) / (bottom - top);
		o_matrix[3][0] = -(right + left) / (right - left);
		o_matrix[3][1] = -(top + bottom) / (bottom - top);
		o_matrix[2][2] = -static_cast<float>(1) / (zFar - zNear);
		o_matrix[3][2] = -zNear / (zFar - zNear);
		return o_matrix;
	}


	glm::mat4 OrthoCameraComponentInstance::createRenderProjectionMatrix(float left, float right, float bottom, float top)
	{
		glm::mat4x4 o_matrix(1);
		o_matrix[0][0] = static_cast<float>(2) / (right - left);
		o_matrix[1][1] = static_cast<float>(2) / (bottom - top);
		o_matrix[2][2] = -static_cast<float>(1);
		o_matrix[3][0] = -(right + left) / (right - left);
		o_matrix[3][1] = -(top + bottom) / (bottom - top);
		return o_matrix;
	}


	// Hook up attribute changes
	OrthoCameraComponentInstance::OrthoCameraComponentInstance(EntityInstance& entity, Component& resource) :
		CameraComponentInstance(entity, resource)
	{
	}


	bool OrthoCameraComponentInstance::init(utility::ErrorState& errorState)
	{
		mProperties = getComponent<OrthoCameraComponent>()->mProperties;
		mTransformComponent =	getEntityInstance()->findComponent<TransformComponentInstance>();
		if (!errorState.check(mTransformComponent != nullptr, "%s: missing transform component", mID.c_str()))
			return false;

		return true;
	}


	void OrthoCameraComponentInstance::setRenderTargetSize(const glm::ivec2& size)
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


	void OrthoCameraComponentInstance::setMode(EOrthoCameraMode mode)
	{
		if(mProperties.mMode != mode)
		{
			mProperties.mMode = mode;
			setDirty();
		}
	}


	void OrthoCameraComponentInstance::updateProjectionMatrices() const
	{
		if (mDirty)
		{
			switch (mProperties.mMode)
			{
				case EOrthoCameraMode::PixelSpace:
				{
					// In this mode we use the render target size to set the planes.
					glm::ivec2 render_target_size = getRenderTargetSize();
					mRenderProjectionMatrix = createRenderProjectionMatrix(0.0f, (float)render_target_size.x, 0.0f, (float)render_target_size.y, mProperties.mNearClippingPlane, mProperties.mFarClippingPlane);
					mProjectionMatrix = glm::ortho(0.0f, (float)render_target_size.x, 0.0f, (float)render_target_size.y, mProperties.mNearClippingPlane, mProperties.mFarClippingPlane);
					break;
				}
				case EOrthoCameraMode::CorrectAspectRatio:
				{
					// In this mode, we scale the top and bottom planes based on the aspect ratio
					glm::ivec2 renderTargetSize = getRenderTargetSize();
					float aspect_ratio = (float)renderTargetSize.y / (float)renderTargetSize.x;
					float top_plane = mProperties.mTopPlane * aspect_ratio;
					float bottom_plane = mProperties.mBottomPlane * aspect_ratio;
					mRenderProjectionMatrix = createRenderProjectionMatrix(mProperties.mLeftPlane, mProperties.mRightPlane, bottom_plane, top_plane, mProperties.mNearClippingPlane, mProperties.mFarClippingPlane);
					mProjectionMatrix = mProjectionMatrix = glm::ortho(mProperties.mLeftPlane, mProperties.mRightPlane, bottom_plane, top_plane, mProperties.mNearClippingPlane, mProperties.mFarClippingPlane);
					break;
				}
				case EOrthoCameraMode::Custom:
				{
					mRenderProjectionMatrix = createRenderProjectionMatrix(mProperties.mLeftPlane, mProperties.mRightPlane, mProperties.mBottomPlane, mProperties.mTopPlane, mProperties.mNearClippingPlane, mProperties.mFarClippingPlane);
					mProjectionMatrix = glm::ortho(mProperties.mLeftPlane, mProperties.mRightPlane, mProperties.mBottomPlane, mProperties.mTopPlane, mProperties.mNearClippingPlane, mProperties.mFarClippingPlane);
					break;
				}
			}
			mDirty = false;
		}
	}


	const glm::mat4& OrthoCameraComponentInstance::getProjectionMatrix() const
	{
		updateProjectionMatrices();
		return mProjectionMatrix;
	}


	const glm::mat4& OrthoCameraComponentInstance::getRenderProjectionMatrix() const
	{
		updateProjectionMatrices();
		return mRenderProjectionMatrix;
	}


	const glm::mat4 OrthoCameraComponentInstance::getViewMatrix() const
	{
		return glm::inverse(mTransformComponent->getGlobalTransform());
	}


	void OrthoCameraComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{
		components.emplace_back(RTTI_OF(TransformComponent));
	}

}
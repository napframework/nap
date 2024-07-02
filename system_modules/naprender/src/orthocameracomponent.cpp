/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

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

RTTI_BEGIN_CLASS(nap::OrthoCameraProperties, "Orthographic camera properties")
	RTTI_PROPERTY("Mode",				&nap::OrthoCameraProperties::mMode,					nap::rtti::EPropertyMetaData::Default, "Camera space operation mode")
	RTTI_PROPERTY("LeftPlane",			&nap::OrthoCameraProperties::mLeftPlane,			nap::rtti::EPropertyMetaData::Default, "Left plane coordinates, used with 'CorrectAspectRatio' or 'Custom' mode")
	RTTI_PROPERTY("RightPlane",			&nap::OrthoCameraProperties::mRightPlane,			nap::rtti::EPropertyMetaData::Default, "Right plane coordinates, used with 'CorrectAspectRatio' or 'Custom' mode")
	RTTI_PROPERTY("TopPlane",			&nap::OrthoCameraProperties::mTopPlane,				nap::rtti::EPropertyMetaData::Default, "Top plane coordinates, used with 'CorrectAspectRatio' or 'Custom' mode")
	RTTI_PROPERTY("BottomPlane",		&nap::OrthoCameraProperties::mBottomPlane,			nap::rtti::EPropertyMetaData::Default, "Bottom plane coordinates, used with 'CorrectAspectRatio' or 'Custom' mode")
	RTTI_PROPERTY("NearClippingPlane",	&nap::OrthoCameraProperties::mNearClippingPlane,	nap::rtti::EPropertyMetaData::Default, "Camera near clipping plane")
	RTTI_PROPERTY("FarClippingPlane",	&nap::OrthoCameraProperties::mFarClippingPlane,		nap::rtti::EPropertyMetaData::Default, "Camera far clipping plane")
	RTTI_PROPERTY("ClipRect",			&nap::OrthoCameraProperties::mClipRect,				nap::rtti::EPropertyMetaData::Default, "Normalized camera view cropping (clip) rectangle")
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::OrthoCameraComponent, "Creates an orthographic camera projection and view matrix")
	RTTI_PROPERTY("Properties",			&nap::OrthoCameraComponent::mProperties,			nap::rtti::EPropertyMetaData::Default, "Orthographic camera properties")
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
	{ }


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


	void OrthoCameraComponentInstance::setClipRect(const math::Rect& clipRect)
	{
		if (mProperties.mClipRect != clipRect)
		{
			mProperties.mClipRect = clipRect;
			setDirty();
		}
	}


	void OrthoCameraComponentInstance::restoreClipRect()
	{
		if (mProperties.mClipRect != math::topRightRect)
		{
			mProperties.mClipRect = math::topRightRect;
			setDirty();
		}
	}


	void OrthoCameraComponentInstance::updateProjectionMatrices() const
	{
		if (mDirty)
		{
			const auto& prop = mProperties;
			math::Rect rect = prop.mClipRect;

			switch (prop.mMode)
			{
				case EOrthoCameraMode::PixelSpace:
				{
					// In this mode we use the render target size to set the planes.
					const glm::vec2& rt_size = getRenderTargetSize();
					rect =
					{
						{ prop.mClipRect.getMin().x * rt_size.x, prop.mClipRect.getMin().y * rt_size.y },
						{ prop.mClipRect.getMax().x * rt_size.x, prop.mClipRect.getMax().y * rt_size.y }
					};
					break;
				}
				case EOrthoCameraMode::CorrectAspectRatio:
				{
					// In this mode, we scale the top and bottom planes based on the aspect ratio
					const glm::vec2& rt_size = getRenderTargetSize();
					float aspect_ratio = rt_size.y / rt_size.x;
					rect =
					{
						{ prop.mClipRect.getMin().x * prop.mLeftPlane, prop.mClipRect.getMin().y * prop.mBottomPlane * aspect_ratio },
						{ prop.mClipRect.getMax().x * prop.mRightPlane, prop.mClipRect.getMax().y * prop.mTopPlane * aspect_ratio }
					};
					break;
				}
				case EOrthoCameraMode::Custom:
				{
                    glm::vec2 size(prop.mRightPlane - prop.mLeftPlane, prop.mTopPlane - prop.mBottomPlane);
					rect =
					{
						{ prop.mClipRect.getMin().x * size.x + prop.mLeftPlane, prop.mClipRect.getMin().y * size.y + prop.mBottomPlane },
						{ prop.mClipRect.getMax().x * size.x + prop.mLeftPlane, prop.mClipRect.getMax().y * size.y + prop.mBottomPlane }
					};
					break;
				}
				assert(false);
			}
			mRenderProjectionMatrix = createRenderProjectionMatrix(rect.getMin().x, rect.getMax().x, rect.getMin().y, rect.getMax().y, prop.mNearClippingPlane, prop.mFarClippingPlane);
			mProjectionMatrix = glm::ortho(rect.getMin().x, rect.getMax().x, rect.getMin().y, rect.getMax().y, prop.mNearClippingPlane, prop.mFarClippingPlane);
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


	float OrthoCameraComponentInstance::getNearClippingPlane() const
	{
		return mProperties.mNearClippingPlane;
	}


	float OrthoCameraComponentInstance::getFarClippingPlane() const
	{
		return mProperties.mFarClippingPlane;
	}
}

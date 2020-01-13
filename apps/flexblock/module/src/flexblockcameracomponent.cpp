// Local Includes
#include "flexblockcameracomponent.h"

// External Includes
#include <glm/gtc/matrix_transform.hpp> 
#include <entity.h>
#include "transformcomponent.h"

RTTI_BEGIN_CLASS(nap::FlexblockCameraComponent)
RTTI_PROPERTY("Left Offset", &nap::FlexblockCameraComponent::mLeftOffset, nap::rtti::EPropertyMetaData::Default)
RTTI_PROPERTY("Top Offset", &nap::FlexblockCameraComponent::mTopOffset, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::FlexblockCameraComponentInstance)
RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{
	// Helper function to calculate either left/right or top/bottom camera planes. The output is the physical location )
	// of the plane in camera space.
	static void calculateCameraPlanes(float fov, float aspectRatio, float nearPlane, int numDimensions, int location, float& min, float& max)
	{
		assert(location < numDimensions);
		assert(numDimensions > 0);

		const float start_angle = -fov * 0.5f;
		const float split_angle = fov / (float)numDimensions;
		const float min_angle = start_angle + split_angle * (float)location;
		const float max_angle = min_angle + split_angle;
		min = glm::tan(min_angle) * aspectRatio * nearPlane;
		max = glm::tan(max_angle) * aspectRatio * nearPlane;
	}

	static glm::mat4 createASymmetricProjection(float nearPlane, float farPlane, float leftPlane, float rightPlane, float topPlane, float bottomPlane)
	{
		glm::mat4 projection(0.0f);
		projection[0][0] = (2.0f * nearPlane) / (rightPlane - leftPlane);
		projection[1][0] = 0.0f;
		projection[2][0] = (rightPlane + leftPlane) / (rightPlane - leftPlane);
		projection[3][0] = 0.0f;

		projection[0][1] = 0.0f;
		projection[1][1] = (2.0f * nearPlane) / (topPlane - bottomPlane);
		projection[2][1] = (topPlane + bottomPlane) / (topPlane - bottomPlane);
		projection[3][1] = 0.0f;

		projection[0][2] = 0.0f;
		projection[1][2] = 0.0f;
		projection[2][2] = -(farPlane + nearPlane) / (farPlane - nearPlane);
		projection[3][2] = -2.0f * farPlane * nearPlane / (farPlane - nearPlane);

		projection[0][3] = 0.0f;
		projection[1][3] = 0.0f;
		projection[2][3] = -1.0f;
		projection[3][3] = 0.0f;

		return projection;
	}

	bool FlexblockCameraComponentInstance::init(utility::ErrorState& errorState)
	{
		FlexblockCameraComponent* resource = getComponent<FlexblockCameraComponent>();

		if (!PerspCameraComponentInstance::init(errorState))
			return false;

		mLeftOffset = resource->mLeftOffset;
		mTopOffset = resource->mTopOffset;

		return true;
	}

	FlexblockCameraComponentInstance::FlexblockCameraComponentInstance(EntityInstance& entity, Component& resource) :
		PerspCameraComponentInstance(entity, resource)
	{
	}

	FlexblockCameraComponentInstance::~FlexblockCameraComponentInstance() {}

	// Computes projection matrix if dirty, otherwise returns the
	// cached version
	const glm::mat4& FlexblockCameraComponentInstance::getProjectionMatrix() const
	{
		if (mDirty)
		{
			const float fov = glm::radians(mProperties.mFieldOfView);
			const float near_plane = mProperties.mNearClippingPlane;
			const float far_plane = mProperties.mFarClippingPlane;
			const float aspect_ratio = ((float)(getRenderTargetSize().x * mProperties.mGridDimensions.x)) / ((float)(getRenderTargetSize().y * mProperties.mGridDimensions.y));

			float left, right, top, bottom;
			calculateCameraPlanes(fov, aspect_ratio, near_plane, mProperties.mGridDimensions.x, mProperties.mGridLocation.x, left, right);
			calculateCameraPlanes(fov, 1.0f, near_plane, mProperties.mGridDimensions.y, mProperties.mGridLocation.y, bottom, top);

			float w = right - left;
			w *= mLeftOffset;
			float h = top - bottom;
			h *= mTopOffset;

			mProjectionMatrix = createASymmetricProjection(near_plane, far_plane, left - w, right - w, top + h, bottom + h);

			mDirty = false;
		}

		return mProjectionMatrix;
	}
}
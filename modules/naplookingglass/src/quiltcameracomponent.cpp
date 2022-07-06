/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "quiltcameracomponent.h"

// External Includes
#include <glm/gtc/matrix_transform.hpp>
#include <transformcomponent.h>
#include <mathutils.h>
#include <entity.h>
#include <nap/logger.h>

RTTI_BEGIN_CLASS(nap::QuiltCameraProperties)
	RTTI_PROPERTY("NearClippingPlane",	&nap::QuiltCameraProperties::mNearClippingPlane,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FarClippingPlane",	&nap::QuiltCameraProperties::mFarClippingPlane,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("CameraSize",			&nap::QuiltCameraProperties::mCameraSize,			nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::QuiltCameraComponent)
	RTTI_PROPERTY("LookingGlassDevice",	&nap::QuiltCameraComponent::mDevice,				nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Properties",			&nap::QuiltCameraComponent::mProperties,			nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::QuiltCameraComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{
	// 
	// This is a default function to create an asymmetric projection. An asymmetric projection is a projection where the center of projection is not halfway
	// the left and right, top and bottom planes, but off-center. 
	//
	// This can be used for rendering to multiple render targets with a single camera using modified projection matrices per render target.
	// For example, consider the following view frustrum V:
	// 
	//	*---------------*
	//    *     V     *
	//      *       *
	//        *   *
	//          *
	//
	// We can render this frustum by splitting the projection into a render for the left render target and the right render target:
	// 
	//	*-------*----- *    *--------*    *-------* 
	//    *     *     *        *  L  *	  *  R  *   
	//      *   *   *            *   *	  *   *     
	//        * * *                * *	  * *       
	//          *                    *	  *         
	//
	// Or, in a more complicated environment, we could have a single world split into multiple projections:
	//
	//
	//   *---------*---------*---------*
	//      *   1   *   2   *   3   *
	//         *     *     *     *
	//            *   *   *   *
	//               * * * *
	//                  *
	//
	// Notice how a single projection matrix for a render target can look like:
	//
	//
	//   * ---------*
	//      *   1   *
	//         *     *
	//            *   *
	//               * *
	//                  *
	//
	// The center of projection is not in the middle of the left or right plane anymore, so we need to be able to build a projection matrix with custom planes.
	// Because it is hard to find proper documentation why this matrix is built up the way it is, here's a brief explanation of the math
	// that make this projection matrix different from a regular symmetric matrix:
	//
	// Projection into clip space is done in two steps: we first project x/y onto the near plane. It is then still in camera space and it needs
	// to be converted to clip space (=converted to [-1,1]).
	// 
	// Projection onto the near plane is done by using similar triangles. In the following figure you see a point P that needs to be projected
	// onto the near plane (displayed as P`). In the figure, n is the distance to the near plane and Pz the z coordinate of point P.
	// 
	//       P--------
	//        \      |
	//  *      \     |Pz         *
	//    *     \    |         *
	//      *----P`--|       *
	//        *   \  |n    *
	//          *  \ |   *
	//            * \| *
	//               *
	// Using similar triangles, we know that P`x = Px * n / Pz.
	// Note, however, that the z-axis is negative towards the distance (away from the camera). Because 'n' is given as a distance and not the z position of the near plane, we should
	// divide against -Pz instead. So, the equation becomes:
	//		P`x = Px * n / -Pz
	//
	// We now have the projected point on the near plane, but we need to have it in clip space: [-1..1]. It needs to be -1 on the left side of the near plane, or 1 on the right side. First, let's
	// calculate the left and right side of the near plane:
	//		r = (tan(angle_right) * n)
	//		l = (tan(angle_left) * n)
	// 
	// All we need to do is scale and bias the projected value P`x. To understand how to do this, it is best to start thinking simply about how one would map any value that lies between a 
	// value 'l' and 'r' to a [-1,1] range. We'd start by mapping it into a [0..1] range first like so:
	//		(x-l)/(r-l)
	// Then, we'd need to multiply by two, and subtract one to get into the [-1..1] range:
	//		2 * (x-l) / (r-l) - 1
	// This equation contains a scale and bias, let's separate the two first (we'll see why we do this in a moment):
	//		2x/(r-l) -2l/(r-l) - 1
	//
	// Now, let's simplify the bias part:
	//		-2l/(r-l) -(r-l)/(r-l)
	//		(-2l -r + l) / (r-l)
	//		(-l-r) / (r-l)
	//		-(r+l) / (r-l)
	// So the full equation becomes:
	//		2x/(r-l) - (r+l)/(r-l)
	//
	// The scale and bias are:
	//
	// scale: 2x/(r-l)
	// bias: -(r+l)/(r-l)
	//
	// We substitute x into the scale:
	//	2 (Px * n) * (1/-Pz) / (r-l), or:
	//	2 (Px * n) / (r-l) * (1/-Pz)
	// 1/-Pz is again isolated. We can put [2n / (r-l)] in [0][0] so that it gets multiplied with Px and -Pz in [2][3] so that it ends up in the w component for quiltective division later. 
	// That will yield 2 (Px * n) / (r-l) * (1/-Pz) after division by 'w'.
	//
	// To understand why the scale and bias are separated, we need to see what would happen if we would add the bias to [0][0]:
	//		(A+B)/Z = A/Z+B/Z
	// This quiltective division by Z is undesirable for our bias. We can compensate for the division by -z by multiplying with -z:
	//		(A+B*Z)/Z = A/Z + B
	// To do this in our equation, we perform two steps. First we put the bias in [2][0], so that it gets multiplied with Pz. Then, we multiply with -1. Our bias becomes:
	//		(r+l)/(r-l)
	// This is what we put in [2][0].
	// 
	// The same applies for the y values as it does for the x values, except that the Y gets another multiplication with -1 for the inversion of the y axis. This difference is visible in [1][1].
	//
	// The z-conversion is a bit different and a bit tricky. We need to scale and bias it to the (Vulkan) range of z values: [0..1]. We can scale and bias
	// by using the Pz and Pw components. Because the Pw component is always 1, we can use that to bias the z value. However, the resulting value is always
	// divided by -Pz after quiltective division. So we begin the equation like this:
	//		P`z = (Pz * S + Pw * B) / -Pz
	// We know that Pw is 1, so:
	// 		P`z = (Pz * S + B) / -Pz
	// We need to find S (scale) and B (bias) so that we can fill it in in the matrix. For Vulkan, the calculated value should be 0 when on the near plane, 
	// and 1 when on the far plane. We can fill those values in for P`z and Pz and solve for S and B. Note that we need to negate the 'n' and 'f' values 
	// because they represent distance, and we need the position on the z-axis instead.
	//		0 = (-nS + B) / -(-n)
	//		0 = (-nS + B)
	//		B = nS
	//
	//		1 = (-fS + B) / -(-f)
	//		f = -fS + B
	//
	// Substitute B into last equation:
	//		f = -fS + nS
	//		f = S(-f + n)
	//		S = f / (n - f)
	//
	// Substituate S into B equation:
	//		B = (n * f) / (n - f)
	//
	static glm::mat4 createASymmetricProjection(float nearPlane, float farPlane, float leftPlane, float rightPlane, float topPlane, float bottomPlane)
	{
		glm::mat4 projection(0.0f);
		projection[0][0] = (2.0f * nearPlane) / (rightPlane - leftPlane);
		projection[1][0] = 0.0f;
		projection[2][0] = (rightPlane + leftPlane) / (rightPlane - leftPlane);
		projection[3][0] = 0.0f;

		projection[0][1] = 0.0f;
		projection[1][1] = -(2.0f * nearPlane) / (topPlane - bottomPlane);
		projection[2][1] = (topPlane + bottomPlane) / (topPlane - bottomPlane);
		projection[3][1] = 0.0f;

		projection[0][2] = 0.0f;
		projection[1][2] = 0.0f;
		projection[2][2] = farPlane / (nearPlane - farPlane);
		projection[3][2] = nearPlane * farPlane / (nearPlane - farPlane);

		projection[0][3] = 0.0f;
		projection[1][3] = 0.0f;
		projection[2][3] = -1.0f;
		projection[3][3] = 0.0f;

		return projection;
	}


	// Helper function to calculate either left/right or top/bottom camera planes. The output is the physical location of the near plane in camera space.
	static void calculateCameraPlanes(float fov, float aspectRatio, float nearPlane, int numDimensions, int location, float& min, float& max)
	{
		assert(location < numDimensions);
		assert(numDimensions > 0);

		const float angle_extent = fov * 0.5f;
		const float near_extent = glm::tan(angle_extent) * aspectRatio * nearPlane;

		min = math::lerp(-near_extent, near_extent, location / static_cast<float>(numDimensions));
		max = math::lerp(-near_extent, near_extent, (location+1) / static_cast<float>(numDimensions));
	}

	// Hook up attribute changes
	QuiltCameraComponentInstance::QuiltCameraComponentInstance(EntityInstance& entity, Component& resource) :
		CameraComponentInstance(entity, resource)
	{
	}


	bool QuiltCameraComponentInstance::init(utility::ErrorState& errorState)
	{
		const auto* resource = getComponent<QuiltCameraComponent>();
		mProperties = resource->mProperties;

		mTransformComponent = getEntityInstance()->findComponent<TransformComponentInstance>();
		if (!errorState.check(mTransformComponent != nullptr, "%s: missing transform component", mID.c_str()))
			return false;

		const auto& settings = resource->mDevice->getQuiltSettings();
		glm::ivec2 view_size = { settings.mWidth / settings.mColumns, settings.mHeight / settings.mRows };

		setRenderTargetSize(view_size);
		setViewCone(resource->mDevice->getViewCone());
		resetView();

		updateMatrices();

		return true;
	}


	void QuiltCameraComponentInstance::setView(int view, int viewCount)
	{
		mViewCount = math::max(viewCount, 1);
		mCurrentViewIndex = math::clamp(view, 0, mViewCount);
		setDirty();
	}


	void QuiltCameraComponentInstance::resetView()
	{
		mCurrentViewIndex = 0;
		mViewCount = 1;
		setDirty();
	}


	void QuiltCameraComponentInstance::setViewCone(float degrees)
	{
		mViewCone = glm::radians(degrees);
		setDirty();
	}


	void QuiltCameraComponentInstance::setCameraSize(float size)
	{
		mProperties.mCameraSize = math::max(size, 0.5f);
	}


	float QuiltCameraComponentInstance::getCameraSize() const
	{
		return mProperties.mCameraSize;
	}


	void QuiltCameraComponentInstance::setRenderTargetSize(const glm::ivec2& size)
	{
		if (size != getRenderTargetSize())
		{
			// Set camera aspect ratio derived from width and height
			CameraComponentInstance::setRenderTargetSize(size);
			setDirty();
		}
	}


	float QuiltCameraComponentInstance::getFieldOfView() const
	{
		// The Lookingglass field of view is always 14 degrees according to measurement
		// See: https://docs.lookingglassfactory.com/keyconcepts/camera#field-of-view
		return glm::radians(14.0f);
	}


	float QuiltCameraComponentInstance::getDistanceToFocalPlane() const
	{
		return mDistanceToFocal;
	}


	glm::vec3 QuiltCameraComponentInstance::getFocalPoint() const
	{
		return math::extractPosition(mTransformComponent->getGlobalTransform());
	}


	glm::vec3 QuiltCameraComponentInstance::getFocalOffset() const
	{
		const glm::vec3& camera_direction = mTransformComponent->getLocalTransform()[2];
		return -glm::normalize(camera_direction) * mDistanceToFocal;
	}


	void QuiltCameraComponentInstance::updateMatrices() const
	{
		if (mDirty)
		{
			const float fov = getFieldOfView();
			const float near = mProperties.mNearClippingPlane;
			const float far = mProperties.mFarClippingPlane;
			const float aspect_ratio = static_cast<float>(getRenderTargetSize().x) / static_cast<float>(getRenderTargetSize().y);

			float left, right, top, bottom;
 			calculateCameraPlanes(fov, aspect_ratio, near, 1, 0, left, right);
			calculateCameraPlanes(fov, 1.0f, near, 1, 0, bottom, top);
			mRenderProjectionMatrix = createASymmetricProjection(near, far, left, right, top, bottom);
			
			mProjectionMatrix = glm::perspective(getFieldOfView(), aspect_ratio, near, far);

			// calculate the camera distance, which is actually static when camera size keeps the same
			mDistanceToFocal = mProperties.mCameraSize / tan(fov * 0.5f);

			// start at -viewCone * 0.5 and go up to viewCone * 0.5
			float frac = (mCurrentViewIndex == 0 && mViewCount == 1) ? 0.5f : (mCurrentViewIndex / static_cast<float>(mViewCount - 1) - 0.5f);
			float offset_angle = frac * mViewCone; 

			// calculate offset
			mHorizontalOffset = mDistanceToFocal * tan(offset_angle);

			// changes the projection matrix to be parallel
			float proj_offset = mHorizontalOffset / (mProperties.mCameraSize * aspect_ratio);
			mProjectionMatrix[2][0] += proj_offset;
			mRenderProjectionMatrix[2][0] += proj_offset;

			mDirty = false;
		}
	}


	const glm::mat4& QuiltCameraComponentInstance::getRenderProjectionMatrix() const
	{
		updateMatrices();
		return mRenderProjectionMatrix;
	}


	const glm::mat4& QuiltCameraComponentInstance::getProjectionMatrix() const
	{
		updateMatrices();
		return mProjectionMatrix;
	}


	const glm::mat4 QuiltCameraComponentInstance::getViewMatrix() const
	{
		updateMatrices();

		// Calculate the offset that the camera should move.
		glm::mat4 view_matrix = glm::inverse(mTransformComponent->getGlobalTransform());

		// The offset matrix moves the camera along a line parallel to the focal plane based on the current view.
		glm::mat4 offset_matrix = glm::translate({}, glm::vec3(mHorizontalOffset, 0.0f, 0.0f));

		return offset_matrix * view_matrix;
	}


	void QuiltCameraComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{
		components.push_back(RTTI_OF(TransformComponent));
	}
}

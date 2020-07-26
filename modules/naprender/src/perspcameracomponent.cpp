// Local Includes
#include "perspcameracomponent.h"

// External Includes
#include <glm/gtc/matrix_transform.hpp> 
#include <entity.h>
#include "transformcomponent.h"

RTTI_BEGIN_CLASS(nap::PerpCameraProperties)
	RTTI_PROPERTY("FieldOfView",		&nap::PerpCameraProperties::mFieldOfView,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("NearClippingPlane",	&nap::PerpCameraProperties::mNearClippingPlane,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FarClippingPlane",	&nap::PerpCameraProperties::mFarClippingPlane,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("GridDimensions",		&nap::PerpCameraProperties::mGridDimensions,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("GridLocation",		&nap::PerpCameraProperties::mGridLocation,		nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::PerspCameraComponent)
	RTTI_PROPERTY("Properties",			&nap::PerspCameraComponent::mProperties,		nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::PerspCameraComponentInstance)
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
	// 1/-Pz is again isolated. We can put [2n / (r-l)] in [0][0] so that it gets multiplied with Px and -Pz in [2][3] so that it ends up in the w component for perspective division later. 
	// That will yield 2 (Px * n) / (r-l) * (1/-Pz) after division by 'w'.
	//
	// To understand why the scale and bias are separated, we need to see what would happen if we would add the bias to [0][0]:
	//		(A+B)/Z = A/Z+B/Z
	// This perspective division by Z is undesirable for our bias. We can compensate for the division by -z by multiplying with -z:
	//		(A+B*Z)/Z = A/Z + B
	// To do this in our equation, we perform two steps. First we put the bias in [2][0], so that it gets multiplied with Pz. Then, we multiply with -1. Our bias becomes:
	//		(r+l)/(r-l)
	// This is what we put in [2][0].
	// 
	// The same applies for the y values as it does for the x values, except that the Y gets another multiplication with -1 for the inversion of the y axis. This difference is visible in [1][1].
	//
	// The z-conversion is a bit different and a bit tricky. We need to scale and bias it to the (Vulkan) range of z values: [0..1]. We can scale and bias
	// by using the Pz and Pw components. Because the Pw component is always 1, we can use that to bias the z value. However, the resulting value is always
	// divided by -Pz after perspective division. So we begin the equation like this:
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

		const float start_angle = -fov * 0.5f;
		const float split_angle = fov / (float)numDimensions;
		const float min_angle = start_angle + split_angle * (float)location;
		const float max_angle = min_angle + split_angle;
		min = glm::tan(min_angle) * aspectRatio * nearPlane;
		max = glm::tan(max_angle) * aspectRatio * nearPlane;
	}


	// Hook up attribute changes
	PerspCameraComponentInstance::PerspCameraComponentInstance(EntityInstance& entity, Component& resource) :
		CameraComponentInstance(entity, resource)
	{
	}


	bool PerspCameraComponentInstance::init(utility::ErrorState& errorState)
	{
		mProperties = getComponent<PerspCameraComponent>()->mProperties;
		mTransformComponent = getEntityInstance()->findComponent<TransformComponentInstance>();
		if (!errorState.check(mTransformComponent != nullptr, "%s: missing transform component", mID.c_str()))
			return false;

		return true;
	}


	// Set camera aspect ratio derived from width and height
	void PerspCameraComponentInstance::setRenderTargetSize(const glm::ivec2& size)
	{
		if (size != getRenderTargetSize())
		{
			CameraComponentInstance::setRenderTargetSize(size);
			setDirty();
		}
	}


	// Use this function to split the projection into a grid of same-sized rectangles.
	void PerspCameraComponentInstance::setGridDimensions(int numRows, int numColumns)
	{
		if (numColumns != mProperties.mGridDimensions.x || numRows != mProperties.mGridDimensions.y)
		{
			assert(mProperties.mGridLocation.x < numColumns && mProperties.mGridLocation.y < numRows);
			mProperties.mGridDimensions.x = numColumns;
			mProperties.mGridDimensions.y = numRows;
			setDirty();
		}
	}


	// Sets the horizontal and vertical index into the projection grid as set by setSplitDimensions.
	void PerspCameraComponentInstance::setGridLocation(int row, int column)
	{
		if (column != mProperties.mGridLocation.x || row != mProperties.mGridLocation.y)
		{
			assert(column >= 0 && column < mProperties.mGridDimensions.x && row >= 0 && row < mProperties.mGridDimensions.y);
			mProperties.mGridLocation.x = column;
			mProperties.mGridLocation.y = row;
			setDirty();
		}
	}


	void PerspCameraComponentInstance::setFieldOfView(float fov)
	{
		mProperties.mFieldOfView = fov;
		setDirty();
	}


	float PerspCameraComponentInstance::getFieldOfView() const
	{
		return mProperties.mFieldOfView;
	}


	void PerspCameraComponentInstance::updateProjectionMatrices() const
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

			mRenderProjectionMatrix = createASymmetricProjection(near_plane, far_plane, left, right, top, bottom);
			mProjectionMatrix = glm::perspective(fov, aspect_ratio, near_plane, far_plane);
			mDirty = false;
		}
	}


	const glm::mat4& PerspCameraComponentInstance::getRenderProjectionMatrix() const
	{
		updateProjectionMatrices();
		return mRenderProjectionMatrix;
	}


	const glm::mat4& PerspCameraComponentInstance::getProjectionMatrix() const
	{
		updateProjectionMatrices();
		return mProjectionMatrix;
	}


	const glm::mat4 PerspCameraComponentInstance::getViewMatrix() const
	{
		const glm::mat4& global_transform = mTransformComponent->getGlobalTransform();
		return glm::inverse(global_transform);
	}


	void PerspCameraComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{
		components.push_back(RTTI_OF(TransformComponent));
	}

}
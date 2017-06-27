// Local Includes
#include "perspcameracomponent.h"

// External Includes
#include <glm/gtc/matrix_transform.hpp> 
#include <nap/entityinstance.h>
#include "transformcomponent.h"

RTTI_BEGIN_CLASS(nap::PerpCameraProperties)
	RTTI_PROPERTY("FieldOfView",		&nap::PerpCameraProperties::mFieldOfView,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("NearClippingPlane",	&nap::PerpCameraProperties::mNearClippingPlane,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FarClippingPlane",	&nap::PerpCameraProperties::mFarClippingPlane,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("GridDimensions",		&nap::PerpCameraProperties::mGridDimensions,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("GridLocation",		&nap::PerpCameraProperties::mGridLocation,		nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::PerspCameraComponentResource)
	RTTI_PROPERTY("Properties",			&nap::PerspCameraComponentResource::mProperties,		nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_CONSTRUCTOR1(nap::PerspCameraComponent, nap::EntityInstance&)
RTTI_END_CLASS

namespace nap
{

	// 
	// This is a default function to create an asymmetric projection. This means a projection where the center of projection is not halfway
	// the left and right, top and bottom planes, but off-center. 
	//
	// This can be used for rendering to multiple render targets with a single camera using modified projection matrices per render target.
	// For example, consider the following view frustrum V:
	// 
	//	*               *
	//    *     V     *
	//      *       *
	//        *   *
	//          *
	//
	// We can render this frustum by splitting the projection into a render for the left render target and the right render target:
	// 
	//	*       *       *
	//    *  L  *  R  *
	//      *   *   *
	//        * * *
	//          *
	// The center of projection is not symmetric anymore, so we need to be able to build a projection matrix with custom planes.
	// Because it is hard to find proper documentation why this matrix is built up the way it is, here's a brief explanation of the math
	// that make this projection matrix different from a regular symmetric matrix:
	//
	// Projection into clip space is done in two steps: we first project x/y onto the near plane. It is then still in camera space and it needs
	// to be converted to clip space (=converted to [-1,1]).
	// 
	// Projection onto the near plane is done by using similar triangles. In the following figure you see a point P that needs to be projected
	// onto the near plane (displayed as P`). In the figure, n is the near plane distance and Pz the z coordinate of point P.
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
	// Using similar triangles, we know that P`x = Px * n / Pz. We are using homogenous coordinates so that we can linearly interpolate coordinates
	// on the GPU. division by z is performed later (by performing div by 'w'). The division by Pz is separated. By placing Pz in w, it will be divided on the GPU.
	// 
	// To scale to [-1,1], we divide by the the width of the near plane. For symmetric perspective projections, we can divide by the width of half of the near plane. 
	// This can be calculated by:
	//		width = tan(fov * 0.5) * n
	// That leads to:
	//		(Px * n) / (tan(fov * 0.5) * n)
	// The n cancels out, so:
	//		Px * (1 / tan(fov * 0.5))
	// This is the value that is put into [0][0] for symmetric projection. For asymmetric projections, this is different. The width of the plane is calculated by:
	//		r - l, where r and l are the right and left planes.
	// r and l are calculated separately:
	//		r = (tan(angle_right) * n)
	//		l = (tan(angle_left) * n)
	// This leads to:
	//		(Px * n) / (r-l)
	// At this point, the scale of the projected values is 1, so we need to multiply by 2 to be able to go to clip space [-1,1]. 
	//		Px * (2*n)/(r-l)
	// This is the value that is put into [0][0]. Because the value is still off-center and not in [-1,1] space, we still need to add a bias to it. Important to notice is that as soon as we 
	// start adding, the division by Pz becomes 'problematic' as it is taken out of the calculation and performed as a poststep. To illustrate:
	//		(A+B)/Z = A*Z+B*Z
	//		(A+B*Z)/Z = A*Z + B
	// So we compensate by multiplying the part that we add by Pz. This is the reason why the shift and bias part is placed in [2][0] (it gets multiplied by Pz).
	//		The shift and bias is performed by:
	//		(r+l)/(r-l)
	// Which is placed in [2][0].
	// The same applies for the y values as it does for the x values. 
	//
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


	// Hook up attribute changes
	PerspCameraComponent::PerspCameraComponent(EntityInstance& entity) :
		CameraComponent(entity)
	{
	}


	bool PerspCameraComponent::init(const ObjectPtr<ComponentResource>& resource, EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState)
	{
		mProperties = rtti_cast<PerspCameraComponentResource>(resource.get())->mProperties;
		mTransformComponent = getEntity()->findComponent<TransformComponent>();
		if (!errorState.check(mTransformComponent != nullptr, "Missing transform component"))
			return false;

		return true;
	}


	// Set camera aspect ratio derived from width and height
	void PerspCameraComponent::setRenderTargetSize(glm::ivec2 size)
	{
		if (mRenderTargetSize != size)
		{
			mRenderTargetSize = size;
			setDirty();
		}
	}

	// Use this function to split the projection into a grid of same-sized rectangles.
	void PerspCameraComponent::setGridDimensions(int numRows, int numColumns)
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
	void PerspCameraComponent::setGridLocation(int row, int column)
	{
		if (column != mProperties.mGridLocation.x || row != mProperties.mGridLocation.y)
		{
			assert(column >= 0 && column < mProperties.mGridDimensions.x && row >= 0 && row < mProperties.mGridDimensions.y);
			mProperties.mGridLocation.x = column;
			mProperties.mGridLocation.y = row;
			setDirty();
		}
	}


	// Computes projection matrix if dirty, otherwise returns the
	// cached version
	const glm::mat4& PerspCameraComponent::getProjectionMatrix() const
	{
		if (mDirty)
		{
			const float fov = glm::radians(mProperties.mFieldOfView);
			const float near_plane = mProperties.mNearClippingPlane;
			const float far_plane = mProperties.mFarClippingPlane;
			const float aspect_ratio = ((float)(mRenderTargetSize.x * mProperties.mGridDimensions.x)) / ((float)(mRenderTargetSize.y * mProperties.mGridDimensions.y));

			float left, right, top, bottom;
			calculateCameraPlanes(fov, aspect_ratio, near_plane, mProperties.mGridDimensions.x, mProperties.mGridLocation.x, left, right);
			calculateCameraPlanes(fov, 1.0f, near_plane, mProperties.mGridDimensions.y, mProperties.mGridLocation.y, bottom, top);

			mProjectionMatrix = createASymmetricProjection(near_plane, far_plane, left, right, top, bottom);

			mDirty = false;
		}

		return mProjectionMatrix;
	}


	const glm::mat4 PerspCameraComponent::getViewMatrix() const
	{
		const glm::mat4& global_transform = mTransformComponent->getGlobalTransform();
		return glm::inverse(global_transform);
	}
}
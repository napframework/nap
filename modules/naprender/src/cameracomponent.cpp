// Local Includes
#include "cameracomponent.h"

// External Includes
#include <glm/gtc/matrix_transform.hpp> 

namespace nap
{
	// Hook up attribute changes
	CameraComponent::CameraComponent()
	{
		// Connect attribute changes, marks this node to be dirty
		fieldOfView.valueChanged.connect(cameraValueChanged);
		aspectRatio.valueChanged.connect(cameraValueChanged);
		clippingPlanes.valueChanged.connect(cameraValueChanged);
	}


	// Set camera aspect ratio derived from width and height
	void CameraComponent::setAspectRatio(float width, float height)
	{
		aspectRatio.setValue( width / height );
	}


	// Computes projection matrix if dirty, otherwise returns the
	// cached version
	const glm::mat4& CameraComponent::getProjectionMatrix() const
	{
		if (mDirty)
		{
			mProjectionMatrix = glm::perspective<double>(fieldOfView.getValue(), aspectRatio.getValue(), clippingPlanes.getValue().x, clippingPlanes.getValue().y);
			mDirty = false;
		}
		return mProjectionMatrix;
	}
}

RTTI_DEFINE(nap::CameraComponent)
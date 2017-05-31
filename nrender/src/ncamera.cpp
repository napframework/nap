// Local Includes
#include "ncamera.h"
#include "nglutils.h"

// External Includes
#include <glm/gtc/matrix_transform.hpp> 

namespace opengl
{
	// Initialize projection matrix
	Camera::Camera()
	{
		updateProjectionMatrix();
	}


	// Set camera field of view
	void Camera::setFieldOfView(float value)
	{
		float fov(value);
		if (fov < 0.0f)
		{
			printMessage(MessageType::WARNING, "invalid camera field of view, can't be negative");
			fov = 50.0f;
		}

		mFov = fov;
		updateProjectionMatrix();
	}


	// Set camera aspect ratio (width / height)
	void Camera::setAspectRatio(float value)
	{
		if (value <= 0.0f)
		{
			printMessage(MessageType::WARNING, "negative camera aspect ratio specified");
		}
		mAspectRatio = value;
		updateProjectionMatrix();
	}


	// Set camera aspect ratio derived from width and height
	void Camera::setAspectRatio(float width, float height)
	{
		setAspectRatio(width / height);
	}


	// Set camera clipping planes
	// Points lower than near and higher than far won't be included
	void Camera::setClippingPlanes(float near, float far)
	{
		setNearClippingPlane(near);
		setFarClippingPlane(far);
	}


	// Set camera near clipping plane, points below value won't be included
	void Camera::setNearClippingPlane(float value)
	{
		if (value < 0.0f)
		{
			printMessage(MessageType::WARNING, "negative camera near clipping value specified");
		}

		if (value > mFarClipPlane)
		{
			printMessage(MessageType::WARNING, "near clipping plane exceeds far clipping plane value");
		}

		mNearClipPlane = value;
		updateProjectionMatrix();
	}


	// Set far clipping plane, points beyond this value won't be considered
	void Camera::setFarClippingPlane(float value)
	{
		if (value < 0.0f)
		{
			printMessage(MessageType::WARNING, "negative camera far clipping value specified");
		}

		if (value < mNearClipPlane)
		{
			printMessage(MessageType::WARNING, "far clipping plane falls below near clipping plane");
		}

		mFarClipPlane = value;
		updateProjectionMatrix();
	}


	// Utility for returning camera clipping planes
	void Camera::getClippingPlanes(float& near, float& far)
	{
		near = mNearClipPlane;
		far  = mFarClipPlane;
	}


	// Updates camera projection matrix
	void Camera::updateProjectionMatrix()
	{
		mProjectionMatrix = glm::perspective<double>(mFov, mAspectRatio, mNearClipPlane, mFarClipPlane);
	}

} // opengl
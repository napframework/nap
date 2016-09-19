#include <napofsimplecamcomponent.h>

RTTI_DEFINE(nap::OFSimpleCamComponent)

namespace nap
{
	/**
	@brief Constructor
	**/
	OFSimpleCamComponent::OFSimpleCamComponent()
	{
		mOrthographic.connectToValue(mOrthoModeChanged);
	}


	/**
	@brief Returns current projection mode
	**/
	ProjectionMode OFSimpleCamComponent::getProjectionMode() const
	{
		return mCamera.getValue().getOrtho() ? ProjectionMode::Orthographic : ProjectionMode::Perspective;
	}


	/**
	@brief Sets the projection mode
	**/
	void OFSimpleCamComponent::setProjectionMode(ProjectionMode inMode)
	{
		if (inMode == ProjectionMode::Orthographic)
		{
			mCamera.getValueRef().enableOrtho();
		}
		else
		{
			mCamera.getValueRef().disableOrtho();
		}
	}


	/**
	@brief Starts the transform relative to the cam component
	**/
	void OFSimpleCamComponent::begin(const ofRectangle& inRect)
	{
		mCamera.getValueRef().begin(inRect);
	}


	/**
	@brief Ends the transform relative to the cam component
	**/
	void OFSimpleCamComponent::end()
	{
		mCamera.getValueRef().end();
	}


	/**
	@brief Resets the transformation matrix of target and camera
	**/
	void OFSimpleCamComponent::reset()
	{
		mCamera.getValueRef().reset();
	}


	/**
	@brief Sets the distance from the target
	**/
	void OFSimpleCamComponent::setDistance(float inValue)
	{
		mCamera.getValueRef().setDistance(inValue);
	}


	/**
	@brief Sets far clipping mode
	**/
	void OFSimpleCamComponent::setFarClip(float inValue)
	{
		mCamera.getValueRef().setFarClip(inValue);
	}

	/**
	@brief Sets near clipping mode
	**/
	void OFSimpleCamComponent::setNearClip(float inValue)
	{
		mCamera.getValueRef().setNearClip(inValue);
	}
}
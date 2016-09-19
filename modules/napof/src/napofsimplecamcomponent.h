#pragma once

// NAP Includes
#include <nap/serviceablecomponent.h>
#include <nap/attribute.h>
#include <nap/coremodule.h>

#include <napofattributes.h>
#include <nap/signalslot.h>

// OF Includes
#include <ofEasyCam.h>

namespace nap
{
	enum class ProjectionMode
	{
		Perspective		= 0,
		Orthographic
	};

	/**
	@brief OF Easy Camera Component
	**/
	class OFSimpleCamComponent : public ServiceableComponent
	{
		RTTI_ENABLE_DERIVED_FROM(ServiceableComponent)

	public:
		// Default constructor
		OFSimpleCamComponent();

		// Attributes
		Attribute<ofEasyCam>				mCamera { this, "Camera" };
		Attribute<bool>						mOrthographic{ this, "Orthographic", false };

		//@name Projection
		ProjectionMode						getProjectionMode() const;
		void								reset();
		void								begin(const ofRectangle& inRect);
		void								end();

		//@name Distance
		void								setDistance(float inValue);
		void								setFarClip(float inValue);
		void								setNearClip(float inValue);
		ofVec3f								getGlobalPosition() const { return mCamera.getValue().getGlobalPosition(); }

		//@name Control
		void								enableMouseInput(bool inValue) { inValue ? mCamera.getValueRef().enableMouseInput() : mCamera.getValueRef().disableMouseInput(); }

		// Slots
		NSLOT(mOrthoModeChanged, const bool&, orthoModeChanged)

	private:
		void								setProjectionMode(ProjectionMode inMode);
		void orthoModeChanged(const bool& inValue) { setProjectionMode(inValue ? ProjectionMode::Orthographic : ProjectionMode::Perspective); }
	};
}

RTTI_DECLARE(nap::OFSimpleCamComponent)

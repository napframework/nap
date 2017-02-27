#pragma once

// External Includes
#include <nap/serviceablecomponent.h>
#include <nap/coreattributes.h>
#include <nap/signalslot.h>
#include <ofEasyCam.h>

// Local Includes
#include "napofattributes.h"


/**
 * Wrapper around ofEasyCam so we can use it as an attribute
 */
class ofCam : public ofEasyCam
{
public:
	bool operator == (const ofCam& other) const { return false; }
};


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
		Attribute<ofCam>					mCamera { this, "Camera" };
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
		ofVec3f								getGlobalPosition() const						{ return mCamera.getValue().getGlobalPosition(); }

		//@name Control
		void								enableMouseInput(bool inValue)					{ inValue ? mCamera.getValueRef().enableMouseInput() : mCamera.getValueRef().disableMouseInput(); }

		// Slots
		NSLOT(mOrthoModeChanged, AttributeBase&, orthoModeChanged)

	private:
		void								setProjectionMode(ProjectionMode inMode);
		void orthoModeChanged(AttributeBase& attr)											{ setProjectionMode(attr.getValue<bool>() ? ProjectionMode::Orthographic : ProjectionMode::Perspective); }
	};

	// Type converters
	bool convert_string_to_ofEasyCam(const std::string& inValue, ofCam& outValue);
	bool convert_ofEasyCam_to_string(const ofCam& inValue, std::string& outValue);
}

RTTI_DECLARE_DATA(ofCam)
RTTI_DECLARE(nap::OFSimpleCamComponent)

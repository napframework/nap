// Local Includes
#include <napofsimpleshapecomponent.h>
#include <of3dGraphics.h>
#include <nap/entity.h>
#include <napoftransform.h>


//////////////////////////////////////////////////////////////////////////

namespace nap
{

	// Draw
	void OFPlaneComponent::onDraw()
	{

		mPlane.draw();
	}


	// Constructor
	OFPlaneComponent::OFPlaneComponent()
	{
		// Connect attributes
		mWidth.valueChanged.connect(mSizeChanged);
		mHeight.valueChanged.connect(mSizeChanged);

		// Init plane
		InitPlane();
	}


	// Initializes the plane based on the dimensions
	void OFPlaneComponent::InitPlane()
	{
		mPlane.setWidth(mWidth.getValue());
		mPlane.setHeight(mHeight.getValue());
	}


	// Returns the bounds of the plane
	void OFPlaneComponent::getBounds(ofVec3f& OutMin, ofVec3f& OutMax) const
	{
		ofVec3f local_position = mPlane.getPosition();

		float diff_x = mWidth.getValue()  / 2.0f;
		float diff_y = mHeight.getValue() / 2.0f;

		OutMin.x = local_position.x - diff_x;
		OutMin.y = local_position.y - diff_y;
		OutMin.z = 0.0f;

		OutMax.x = local_position.x + diff_x;
		OutMax.y = local_position.y + diff_y;
		OutMax.z = 0.0f;
	}
}


//////////////////////////////////////////////////////////////////////////

namespace nap
{
	/**
	@brief Default constructor
	**/
	OFTextComponent::OFTextComponent()
	{
		loadFont(mSize);
		mSize.valueChanged.connect(mSizeChanged);
	}

	/**
	@brief Loads the font
	**/
	void OFTextComponent::loadFont(AttributeBase& attr)
	{
		mFont.loadFont("Arial", attr.getValue<float>());
		assert(mFont.isLoaded());
	}

	/**
	@brief Draws the font
	**/
	void OFTextComponent::onDraw()
	{

	}


	// Draws the shape
	void OFTextComponent::onPostDraw()
	{
		ofSetColor(mColor.getValue());
		float offset_x = mFont.stringWidth(mText.getValue()) / 2.0f;
		float offset_y = mFont.stringHeight(mText.getValue()) / 2.0f;
		mFont.drawString(mText.getValue(), 0.0f - offset_x, 0.0f - offset_y);
		ofSetColor(ofColor::white);
	}


	/**
	@brief Returns the text bounds
	**/
	void OFTextComponent::getBounds(ofVec3f& OutMin, ofVec3f& OutMax) const
	{
		float offset_x = mFont.stringWidth(mText.getValue())  / 2.0f;
		float offset_y = mFont.stringHeight(mText.getValue()) / 2.0f;

		OutMin.x = 0.0f - offset_x;
		OutMin.y = 0.0f - offset_y;
		OutMin.z = 0.0f;

		OutMax.x = 0.0f + offset_x;
		OutMax.y = 0.0f + offset_y;
		OutMax.z = 0.0f;
	}
}

RTTI_DEFINE(nap::OFShapeComponent)
RTTI_DEFINE(nap::OFPlaneComponent)
RTTI_DEFINE(nap::OFTextComponent)

#pragma once

// NAP Includes
#include <nap/component.h>
#include <nap/attribute.h>

#include <napofattributes.h>
#include <napofrendercomponent.h>

// OF Includes
#include "utils/nofUtils.h"
#include <ofTrueTypeFont.h>
#include <of3dPrimitives.h>
#include <utils/ofVec2i.h>

namespace nap
{
	class OFShapeComponent : public OFRenderableComponent
	{
		RTTI_ENABLE_DERIVED_FROM(OFRenderableComponent)

	public:
		// Default Constructor
		OFShapeComponent() = default;
		virtual void getBounds(ofVec3f& OutMin, ofVec3f& OutMax) const = 0;
	};


	//////////////////////////////////////////////////////////////////////////

	class OFPlaneComponent : public OFShapeComponent
	{
		RTTI_ENABLE_DERIVED_FROM(OFShapeComponent)

	public:
		// Default constructor
		OFPlaneComponent();

		// Attributes
		Attribute<float>	mWidth		{ this, "Width",  {1.0f} };
		Attribute<float>	mHeight		{ this, "Height", {1.0f} };

		// Utility
		void  setWidth(float inWidth)	{ mWidth.setValue(inWidth); }
		float getWidth() const			{ return mWidth.getValue(); }
		void  setHeight(float inHeight)	{ mHeight.setValue(inHeight); }
		float getHeight() const			{ return mHeight.getValue(); }

		// Override
		void onDraw() override;
		void getBounds(ofVec3f& OutMin, ofVec3f& OutMax) const override;

		// Slots
		NSLOT(mSizeChanged, AttributeBase&, SizeChanged)

	private:
		ofPlanePrimitive	mPlane;

		void InitPlane();
		void SizeChanged(AttributeBase& inValue) { InitPlane(); }
	};

	//////////////////////////////////////////////////////////////////////////

	class OFTextComponent : public OFShapeComponent
	{
		RTTI_ENABLE_DERIVED_FROM(OFShapeComponent)

	public:
		// Default constructor
		OFTextComponent();

		Attribute<std::string>	mText	{ this, "Text" };
		Attribute<float>		mSize	{ this, "Size", 10.0f };

		// Override
		void onDraw() override;
		void onPostDraw() override;
		void getBounds(ofVec3f& OutMin, ofVec3f& OutMax) const override;

		// Draw Color
		Attribute<ofFloatColor>	mColor{ this, "Color",{ 1.0f } };

		// Utility
		void setColor(const ofFloatColor& inColor) { mColor.setValue(inColor); }

		// Slots
		NSLOT(mSizeChanged, AttributeBase&, loadFont)

	private:
		ofTrueTypeFont			mFont;
		void loadFont(AttributeBase& attr);
	};
}

// RTTI
RTTI_DECLARE_BASE(nap::OFShapeComponent)
RTTI_DECLARE(nap::OFPlaneComponent)
RTTI_DECLARE(nap::OFTextComponent)

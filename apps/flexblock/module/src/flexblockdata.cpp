#include "flexblockdata.h"

RTTI_BEGIN_CLASS(nap::FlexBlockShapeSizeValues)
RTTI_PROPERTY("Object", &nap::FlexBlockShapeSizeValues::mObject, nap::rtti::EPropertyMetaData::Default)
RTTI_PROPERTY("Frame", &nap::FlexBlockShapeSizeValues::mFrame, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::FlexBlockShapeSize)
RTTI_PROPERTY("Name", &nap::FlexBlockShapeSize::mName, nap::rtti::EPropertyMetaData::Default)
RTTI_PROPERTY("Values", &nap::FlexBlockShapeSize::mValues, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::FlexBlockShapePoints)
RTTI_PROPERTY("Object", &nap::FlexBlockShapePoints::mObject, nap::rtti::EPropertyMetaData::Default)
RTTI_PROPERTY("Frame", &nap::FlexBlockShapePoints::mFrame, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::FlexBlockElements)
RTTI_PROPERTY("Object", &nap::FlexBlockElements::mObject, nap::rtti::EPropertyMetaData::Default)
RTTI_PROPERTY("Object2Frame", &nap::FlexBlockElements::mObject2Frame, nap::rtti::EPropertyMetaData::Default)
RTTI_PROPERTY("Frame", &nap::FlexBlockElements::mFrame, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::FlexBlockShape)
RTTI_PROPERTY("Name", &nap::FlexBlockShape::mName, nap::rtti::EPropertyMetaData::Default)
RTTI_PROPERTY("Inputs", &nap::FlexBlockShape::mInputs, nap::rtti::EPropertyMetaData::Default)
RTTI_PROPERTY("Sizes", &nap::FlexBlockShape::mSizes, nap::rtti::EPropertyMetaData::Default)
RTTI_PROPERTY("Elements", &nap::FlexBlockShape::mElements, nap::rtti::EPropertyMetaData::Default)
RTTI_PROPERTY("Points", &nap::FlexBlockShape::mPoints, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{

	FlexBlockShapeSizeValues::~FlexBlockShapeSizeValues() { }


	bool FlexBlockShapeSizeValues::init(utility::ErrorState& errorState)
	{
		return true;
	}



	FlexBlockShapeSize::~FlexBlockShapeSize() { }


	bool FlexBlockShapeSize::init(utility::ErrorState& errorState)
	{
		return true;
	}



	FlexBlockShapePoints::~FlexBlockShapePoints() { }


	bool FlexBlockShapePoints::init(utility::ErrorState& errorState)
	{
		return true;
	}



	FlexBlockElements::~FlexBlockElements() { }


	bool FlexBlockElements::init(utility::ErrorState& errorState)
	{
		return true;
	}



	FlexBlockShape::~FlexBlockShape() { }


	bool FlexBlockShape::init(utility::ErrorState& errorState)
	{
		return true;
	}
}
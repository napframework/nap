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
RTTI_PROPERTY("Object Element Connections", &nap::FlexBlockElements::mObject, nap::rtti::EPropertyMetaData::Default)
RTTI_PROPERTY("Object Element Connections With Frame", &nap::FlexBlockElements::mObject2Frame, nap::rtti::EPropertyMetaData::Default)
RTTI_PROPERTY("Frame Element Connects", &nap::FlexBlockElements::mFrame, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::FlexBlockShape)
RTTI_PROPERTY("Name", &nap::FlexBlockShape::mName, nap::rtti::EPropertyMetaData::Default)
RTTI_PROPERTY("Motor Count", &nap::FlexBlockShape::mMotorCount, nap::rtti::EPropertyMetaData::Default)
RTTI_PROPERTY("Size", &nap::FlexBlockShape::mSize, nap::rtti::EPropertyMetaData::Default)
RTTI_PROPERTY("Elements", &nap::FlexBlockShape::mElements, nap::rtti::EPropertyMetaData::Required)
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
		if (!errorState.check(mValues != nullptr,
			"Value is not assigned! %s", this->mID.c_str()))
			return false;

		return true;
	}



	FlexBlockShapePoints::~FlexBlockShapePoints() { }


	bool FlexBlockShapePoints::init(utility::ErrorState& errorState)
	{
		if (!errorState.check(mObject.size() > 0,
			"No object points given %s", this->mID.c_str()))
			return false;

		if (!errorState.check(mFrame.size() > 0,
			"No frame points given %s", this->mID.c_str()))
			return false;

		return true;
	}



	FlexBlockElements::~FlexBlockElements() { }


	bool FlexBlockElements::init(utility::ErrorState& errorState)
	{
		if (!errorState.check(mObject.size() > 0,
			"No object elements given %s", this->mID.c_str()))
			return false;

		if (!errorState.check(mFrame.size() > 0,
			"No frame elements given %s", this->mID.c_str()))
			return false;

		if (!errorState.check(mFrame.size() > 0,
			"No object to frame elements given %s", this->mID.c_str()))
			return false;

		return true;
	}



	FlexBlockShape::~FlexBlockShape() { }


	bool FlexBlockShape::init(utility::ErrorState& errorState)
	{
		if (!errorState.check(mSize != nullptr,
			"No flexblockshapesize assigned %s", this->mID.c_str()))
			return false;

		if (!errorState.check(mElements != nullptr,
			"Elements not assigned %s", this->mID.c_str()))
			return false;

		if (!errorState.check(mPoints != nullptr,
			"Points not assigned %s", this->mID.c_str()))
			return false;

		return true;
	}
}
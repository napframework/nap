#include "fcurve.h"

RTTI_BEGIN_CLASS(nap::math::FloatFComplex)
		RTTI_PROPERTY("Time", &nap::math::FloatFComplex::mTime, nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("Value", &nap::math::FloatFComplex::mValue, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::math::FloatFCurvePoint)
	RTTI_PROPERTY("Position", &nap::math::FloatFCurvePoint::mPos, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("InTangent", &nap::math::FloatFCurvePoint::mInTan, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("OutTangent", &nap::math::FloatFCurvePoint::mOutTan, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("InterpolationType", &nap::math::FloatFCurvePoint::mInterp, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("AlignedTangents", &nap::math::FloatFCurvePoint::mTangentsAligned, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::math::FloatFCurve)
	RTTI_PROPERTY("Points", &nap::math::FloatFCurve::mPoints, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS
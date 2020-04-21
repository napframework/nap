#include "fcurve.h"

using namespace nap::math;

RTTI_BEGIN_ENUM(nap::math::ECurveInterp)
	RTTI_ENUM_VALUE(nap::math::ECurveInterp::Linear,   "Linear"),
	RTTI_ENUM_VALUE(nap::math::ECurveInterp::Stepped,  "Stepped"),
	RTTI_ENUM_VALUE(nap::math::ECurveInterp::Bezier,   "Bezier")
RTTI_END_ENUM

RTTI_BEGIN_STRUCT(nap::math::FloatFComplex)
		RTTI_PROPERTY("Time",			&nap::math::FloatFComplex::mTime,				nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("Value",			&nap::math::FloatFComplex::mValue,				nap::rtti::EPropertyMetaData::Default)
RTTI_END_STRUCT

RTTI_BEGIN_STRUCT(nap::math::FloatFCurvePoint)
	RTTI_PROPERTY("Position",			&nap::math::FloatFCurvePoint::mPos,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("InTangent",			&nap::math::FloatFCurvePoint::mInTan,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("OutTangent",			&nap::math::FloatFCurvePoint::mOutTan,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("InterpolationType",	&nap::math::FloatFCurvePoint::mInterp,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("AlignedTangents",	&nap::math::FloatFCurvePoint::mTangentsAligned, nap::rtti::EPropertyMetaData::Default)
RTTI_END_STRUCT

RTTI_BEGIN_CLASS(nap::math::FloatFCurve)
	RTTI_PROPERTY("Points",				&nap::math::FloatFCurve::mPoints,				nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_DEFINE_BASE(nap::math::FloatFCurvePoint);
RTTI_DEFINE_BASE(nap::math::Vec2FCurvePoint);
RTTI_DEFINE_BASE(nap::math::Vec3FCurvePoint);
RTTI_DEFINE_BASE(nap::math::Vec4FCurvePoint);

const static float defaultTanOffset = 0.1f;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DEFAULT CURVE CONSTRUCTORS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


template<>
FCurve<float, float>::FCurve() {
	mPoints.emplace_back(FCurvePoint<float, float>({0.0f, 0.0f}, {-defaultTanOffset, 0.0f}, {defaultTanOffset, 0.0f}));
	mPoints.emplace_back(FCurvePoint<float, float>({1.0f, 1.0f}, {-defaultTanOffset, 0.0f}, {defaultTanOffset, 0.0f}));
}


template<>
Vec2FCurve::FCurve() {
	glm::vec2 nil(0.0f, 0.0f);
	glm::vec2 one(1.0f, 1.0f);
	mPoints.emplace_back(FCurvePoint<float, glm::vec2>({0.0f, nil}, {-defaultTanOffset, nil}, {defaultTanOffset, nil}));
	mPoints.emplace_back(FCurvePoint<float, glm::vec2>({0.0f, one}, {-defaultTanOffset, nil}, {defaultTanOffset, nil}));
}


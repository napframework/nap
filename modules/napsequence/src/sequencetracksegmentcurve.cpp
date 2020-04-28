#include "sequencetracksegmentcurve.h"

DEFINE_VECTOR_SEQUENCETRACKSEGMENTCURVE(nap::SequenceTrackSegmentCurveFloat)
DEFINE_VECTOR_SEQUENCETRACKSEGMENTCURVE(nap::SequenceTrackSegmentCurveVec2)
DEFINE_VECTOR_SEQUENCETRACKSEGMENTCURVE(nap::SequenceTrackSegmentCurveVec3)
DEFINE_VECTOR_SEQUENCETRACKSEGMENTCURVE(nap::SequenceTrackSegmentCurveVec4)


template<>
const float nap::SequenceTrackSegmentCurveFloat::getStartValue() const
{
	assert(mCurves.size() == 1);
	assert(mCurves[0]->mPoints.size() >= 2);

	return mCurves[0]->mPoints[0].mPos.mValue;
}

template<>
const float nap::SequenceTrackSegmentCurveFloat::getEndValue() const
{
	assert(mCurves.size() == 1);
	assert(mCurves[0]->mPoints.size() >= 2);

	return mCurves[0]->mPoints[mCurves[0]->mPoints.size()-1].mPos.mValue;
}

template<>
const float nap::SequenceTrackSegmentCurveFloat::getValue(float t) const
{
	assert(mCurves.size() == 1);

	return mCurves[0]->evaluate(t);
}

template<>
void nap::SequenceTrackSegmentCurveFloat::setStartValue(float t)
{
	assert(mCurves.size() == 1);
	assert(mCurves[0]->mPoints.size() > 1);

	mCurves[0]->mPoints[0].mPos.mValue = t;
}


template<>
void nap::SequenceTrackSegmentCurveFloat::setEndValue(float t)
{
	assert(mCurves.size() == 1);
	assert(mCurves[0]->mPoints.size() > 1);

	mCurves[0]->mPoints[mCurves[0]->mPoints.size()-1].mPos.mValue = t;
}

template<>
const glm::vec2 nap::SequenceTrackSegmentCurveVec2::getStartValue() const
{
	assert(mCurves.size() == 2);
	assert(mCurves[0]->mPoints.size() > 1);

	glm::vec2 value;
	value.x = mCurves[0]->mPoints[0].mPos.mValue;
	value.y = mCurves[1]->mPoints[0].mPos.mValue;

	return value;
}

template<>
const glm::vec2 nap::SequenceTrackSegmentCurveVec2::getEndValue() const
{
	assert(mCurves.size() == 2);
	assert(mCurves[0]->mPoints.size() >= 2);

	glm::vec2 value;
	value.x = mCurves[0]->mPoints[mCurves[0]->mPoints.size() - 1].mPos.mValue;
	value.y = mCurves[1]->mPoints[mCurves[1]->mPoints.size() - 1].mPos.mValue;

	return value;
}

template<>
const glm::vec2 nap::SequenceTrackSegmentCurveVec2::getValue(float t) const
{
	assert(mCurves.size() == 2);

	return glm::vec2(
		mCurves[0]->evaluate(t),
		mCurves[1]->evaluate(t));
}

template<>
void nap::SequenceTrackSegmentCurveVec2::setStartValue(glm::vec2 t)
{
	assert(mCurves.size() == 2);
	assert(mCurves[0]->mPoints.size() > 1);

	for (int i = 0; i < mCurves.size(); i++)
	{
		mCurves[i]->mPoints[0].mPos.mValue = t[i];
	}
}

template<>
void nap::SequenceTrackSegmentCurveVec2::setEndValue(glm::vec2 t)
{
	assert(mCurves.size() == 2);
	assert(mCurves[0]->mPoints.size() > 1);

	for (int i = 0; i < mCurves.size(); i++)
	{
		mCurves[i]->mPoints[mCurves[i]->mPoints.size()-1].mPos.mValue = t[i];
	}
}

template<>
const glm::vec3 nap::SequenceTrackSegmentCurveVec3::getStartValue() const
{
	assert(mCurves.size() == 3);
	assert(mCurves[0]->mPoints.size() >= 2);

	glm::vec3 value;
	value.x = mCurves[0]->mPoints[0].mPos.mValue;
	value.y = mCurves[1]->mPoints[0].mPos.mValue;
	value.z = mCurves[2]->mPoints[0].mPos.mValue;

	return value;
}

template<>
const glm::vec3 nap::SequenceTrackSegmentCurveVec3::getValue(float t) const
{
	assert(mCurves.size() == 3);

	return glm::vec3(
		mCurves[0]->evaluate(t),
		mCurves[1]->evaluate(t),
		mCurves[2]->evaluate(t));
}

template<>
const glm::vec3  nap::SequenceTrackSegmentCurveVec3::getEndValue() const
{
	assert(mCurves.size() == 3);
	assert(mCurves[0]->mPoints.size() >= 2);

	glm::vec3 value;
	value.x = mCurves[0]->mPoints[mCurves[0]->mPoints.size() - 1].mPos.mValue;
	value.y = mCurves[1]->mPoints[mCurves[1]->mPoints.size() - 1].mPos.mValue;
	value.z = mCurves[2]->mPoints[mCurves[2]->mPoints.size() - 1].mPos.mValue;

	return value;
}

template<>
void nap::SequenceTrackSegmentCurveVec3::setStartValue(glm::vec3 t)
{
	assert(mCurves.size() == 3);
	assert(mCurves[0]->mPoints.size() > 1);

	for (int i = 0; i < mCurves.size(); i++)
	{
		mCurves[i]->mPoints[0].mPos.mValue = t[i];
	}
}

template<>
void nap::SequenceTrackSegmentCurveVec3::setEndValue(glm::vec3 t)
{
	assert(mCurves.size() == 3);
	assert(mCurves[0]->mPoints.size() > 1);

	for (int i = 0; i < mCurves.size(); i++)
	{
		mCurves[i]->mPoints[mCurves[i]->mPoints.size() - 1].mPos.mValue = t[i];
	}
}

template<>
const glm::vec4 nap::SequenceTrackSegmentCurveVec4::getStartValue() const
{
	assert(mCurves.size() == 4);
	assert(mCurves[0]->mPoints.size() >= 2);

	glm::vec4 value;
	value.x = mCurves[0]->mPoints[0].mPos.mValue;
	value.y = mCurves[1]->mPoints[0].mPos.mValue;
	value.z = mCurves[2]->mPoints[0].mPos.mValue;
	value.w = mCurves[3]->mPoints[0].mPos.mValue;

	return value;
}

template<>
const glm::vec4 nap::SequenceTrackSegmentCurveVec4::getEndValue() const
{
	assert(mCurves.size() == 4);
	assert(mCurves[0]->mPoints.size() >= 2);

	glm::vec4 value;
	value.x = mCurves[0]->mPoints[mCurves[0]->mPoints.size() - 1].mPos.mValue;
	value.y = mCurves[1]->mPoints[mCurves[1]->mPoints.size() - 1].mPos.mValue;
	value.z = mCurves[2]->mPoints[mCurves[2]->mPoints.size() - 1].mPos.mValue;
	value.w = mCurves[3]->mPoints[mCurves[3]->mPoints.size() - 1].mPos.mValue;

	return value;
}

template<>
const glm::vec4 nap::SequenceTrackSegmentCurveVec4::getValue(float t) const
{
	assert(mCurves.size() == 4);

	return glm::vec4(
		mCurves[0]->evaluate(t),
		mCurves[1]->evaluate(t),
		mCurves[2]->evaluate(t),
		mCurves[3]->evaluate(t));
}

template<>
void nap::SequenceTrackSegmentCurveVec4::setStartValue(glm::vec4 t)
{
	assert(mCurves.size() == 4);
	assert(mCurves[0]->mPoints.size() > 1);

	for (int i = 0; i < mCurves.size(); i++)
	{
		mCurves[i]->mPoints[0].mPos.mValue = t[i];
	}
}

template<>
void nap::SequenceTrackSegmentCurveVec4::setEndValue(glm::vec4 t)
{
	assert(mCurves.size() == 4);
	assert(mCurves[0]->mPoints.size() > 1);

	for (int i = 0; i < mCurves.size(); i++)
	{
		mCurves[i]->mPoints[mCurves[i]->mPoints.size() - 1].mPos.mValue = t[i];
	}
}

template<>
int nap::SequenceTrackSegmentCurveFloat::getCurveCount()		{ return 1; }

template<>
int nap::SequenceTrackSegmentCurveVec2::getCurveCount()			{ return 2; }

template<>
int nap::SequenceTrackSegmentCurveVec3::getCurveCount()			{ return 3; }

template<>
int nap::SequenceTrackSegmentCurveVec4::getCurveCount()			{ return 4; }
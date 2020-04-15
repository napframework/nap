#include "sequencetracksegmentcurve.h"

DEFINE_VECTOR_SEQUENCETRACKSEGMENTCURVE(nap::SequenceTrackSegmentCurveFloat)
DEFINE_VECTOR_SEQUENCETRACKSEGMENTCURVE(nap::SequenceTrackSegmentCurveVec2)
DEFINE_VECTOR_SEQUENCETRACKSEGMENTCURVE(nap::SequenceTrackSegmentCurveVec3)
DEFINE_VECTOR_SEQUENCETRACKSEGMENTCURVE(nap::SequenceTrackSegmentCurveVec4)

template<typename T>   // primary template
bool nap::SequenceTrackSegmentCurve<T>::init(utility::ErrorState& errorState)
{
	int curveCount = -1;
	if (RTTI_OF(glm::vec2) == RTTI_OF(T))
	{
		curveCount = 2;
	}
	else if (RTTI_OF(glm::vec3) == RTTI_OF(T))
	{
		curveCount = 3;
	}
	else if (RTTI_OF(glm::vec4) == RTTI_OF(T))
	{
		curveCount = 4;
	}
	else if (RTTI_OF(float) == RTTI_OF(T))
	{
		curveCount = 1;
	}
	else
	{
		return errorState.check(false, "SequenceTrackSegmentCurve of unknown type!");
	}


	if (SequenceTrackSegment::init(errorState))
	{
		if (!errorState.check(mCurves.size() == curveCount, "size of curves must be %i", curveCount))
		{
			return false;
		}
		else
		{
			for (int i = 0; i < mCurves.size(); i++)
			{
				if (!errorState.check(mCurves[i]->mPoints.size() >= 2, "curve %i has invalid amount of points", i))
				{
					return false;
				}
			}
		}
	}
	else
	{
		return false;
	}

	return true;
}

template<typename T>   // primary template
const T nap::SequenceTrackSegmentCurve<T>::getStartValue() const
{
	assert(false);

	T value;
	return value;
}

template<typename T>   // primary template
const T nap::SequenceTrackSegmentCurve<T>::getEndValue() const
{
	assert(false);

	T value;
	return value;
}

template<typename T>   // primary template
const T nap::SequenceTrackSegmentCurve<T>::getValue(float t) const
{
	assert(false);

	T value;
	return value;
}

template<typename T>   // primary template
void nap::SequenceTrackSegmentCurve<T>::setStartValue(T value)
{
	assert(false);
}

template<typename T>   // primary template
void nap::SequenceTrackSegmentCurve<T>::setEndValue(T value)
{
	assert(false);
}

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

	mCurves[0]->mPoints[mCurves[0]->mPoints.size()].mPos.mValue = t;
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

template class nap::SequenceTrackSegmentCurve<float>;
template class nap::SequenceTrackSegmentCurve<glm::vec2>;
template class nap::SequenceTrackSegmentCurve<glm::vec3>;
template class nap::SequenceTrackSegmentCurve<glm::vec4>;
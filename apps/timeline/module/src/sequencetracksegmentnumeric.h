#pragma once

// internal includes
#include "sequencetracksegment.h"

// external includes
#include <fcurve.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	/**
	 */
	template<typename T>
	class NAPAPI SequenceTrackSegmentNumeric : public SequenceTrackSegment
	{
		RTTI_ENABLE(SequenceTrackSegment)
	public:
		ResourcePtr<math::FCurve<float, T>>		mCurve;
		T										mStartValue;
		T										mEndValue;
	};


	//////////////////////////////////////////////////////////////////////////
	// Numeric Parameter Type Definitions
	//////////////////////////////////////////////////////////////////////////

	using SequenceTrackSegmentFloat = SequenceTrackSegmentNumeric<float>;
	using SequenceTrackSegmentDouble = SequenceTrackSegmentNumeric<double>;

	/**
	* Helper macro that can be used to define the RTTI for a numeric (vector) parameter type
	*/
	#define DEFINE_NUMERIC_SEQUENCETRACKSEGMENT(Type)																			\
		RTTI_BEGIN_CLASS(Type)																						\
			RTTI_PROPERTY("Start Value", &Type::mStartValue, nap::rtti::EPropertyMetaData::Default)				\
			RTTI_PROPERTY("End Value",	&Type::mEndValue, nap::rtti::EPropertyMetaData::Default)				\
			RTTI_PROPERTY("Curve",	&Type::mCurve, nap::rtti::EPropertyMetaData::Default)				\
		RTTI_END_CLASS
}

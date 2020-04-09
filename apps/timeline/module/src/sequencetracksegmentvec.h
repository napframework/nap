#pragma once

// Local Includes
#include "sequencetracksegment.h"

// External Includes

namespace nap
{
	/**
	 */
	template<typename T>
	class SequenceTrackSegmentVec : public SequenceTrackSegment
	{
		RTTI_ENABLE(SequenceTrackSegment)
	public:
		std::vector<ResourcePtr<math::FCurve<float, float>>>	mCurves;
		T														mStartValue;
		T														mEndValue;
	};


	//////////////////////////////////////////////////////////////////////////
	// Vector Sequence Track Segment Type Definitions
	//////////////////////////////////////////////////////////////////////////

	using SequenceTrackSegmentVec2 = SequenceTrackSegmentVec<glm::vec2>;
	using SequenceTrackSegmentVec3 = SequenceTrackSegmentVec<glm::vec3>;


	/**
	* Helper macro that can be used to define the RTTI for a numeric (vector) parameter type
	*/
#define DEFINE_VECTOR_SEQUENCETRACKSEGMENT(Type)																			\
		RTTI_BEGIN_CLASS(Type)																						\
			RTTI_PROPERTY("Start Value", &Type::mStartValue, nap::rtti::EPropertyMetaData::Default)				\
			RTTI_PROPERTY("End Value",	&Type::mEndValue, nap::rtti::EPropertyMetaData::Default)				\
			RTTI_PROPERTY("Curves",	&Type::mCurves, nap::rtti::EPropertyMetaData::Default)				\
		RTTI_END_CLASS
}

#pragma once

// Local Includes
#include "sequencetracksegment.h"

// External Includes

namespace nap
{
	/**
	 */
	template<typename T>
	class SequenceTrackSegmentCurve : public SequenceTrackSegment
	{
		RTTI_ENABLE(SequenceTrackSegment)
	public:
		std::vector<ResourcePtr<math::FCurve<float, float>>>	mCurves;

		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * 
		 */
		const T getStartValue() const;

		/**
		 * 
		 */
		const T getEndValue() const;

		/**
		 * 
		 */
		const T getValue(float pos) const;

		/**
		 * 
		 */
		void setStartValue(T value);

		/**
		*
		*/
		void setEndValue(T value);
	};


	//////////////////////////////////////////////////////////////////////////
	// Vector Sequence Track Segment Type Definitions
	//////////////////////////////////////////////////////////////////////////

	using SequenceTrackSegmentCurveFloat = SequenceTrackSegmentCurve<float>;
	using SequenceTrackSegmentCurveVec2	= SequenceTrackSegmentCurve<glm::vec2>;
	using SequenceTrackSegmentCurveVec3	= SequenceTrackSegmentCurve<glm::vec3>;
	using SequenceTrackSegmentCurveVec4	= SequenceTrackSegmentCurve<glm::vec4>;

	/**
	* Helper macro 
	*/
#define DEFINE_VECTOR_SEQUENCETRACKSEGMENTCURVE(Type)																			\
		RTTI_BEGIN_CLASS(Type)																						\
			RTTI_PROPERTY("Curves",	&Type::mCurves, nap::rtti::EPropertyMetaData::Default)				\
		RTTI_END_CLASS
}

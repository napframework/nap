#pragma once

// Local Includes
#include "sequencetracksegment.h"

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	/**
	 * SequenceTrackSegmentCurve
	 * A SequenceTrackSegment that holds an arbitrary amount of curves
	 * There are four supported types ( float, vec2, vec3, vec4 ) that can contain 1, 2 , 3 or 4 curves 
	 */
	template<typename T>
	class SequenceTrackSegmentCurve : public SequenceTrackSegment
	{
		RTTI_ENABLE(SequenceTrackSegment)
	public:
		// properties
		std::vector<ResourcePtr<math::FCurve<float, float>>> mCurves;  ///< Property: 'Curves' vector holding curves
		math::ECurveInterp mCurveType; ///< Property: 'Curve Type' curve type of this segment ( linear, bezier )

		/**
		 * init evaluates the data hold in curves and checks if its valid for this type
		 *
		 * @param errorState contains information about eventual failure of evaluation
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Gets the value of the first point of the curve, translated into the type
		 */
		const T getStartValue() const;

		/**
		 * Gets the value of the last point of the curve, translated into the type
		 */
		const T getEndValue() const;

		/**
		 * Gets the value of the evaluated point of the curve, translated in to the type
		 * @param pos must be a value between 0-1 ( 0 meaning start of curve, 1 meaning end )
		 */
		const T getValue(float pos) const;

		/**
		 * Sets the value of the first point in the curve(s)
		 */
		void setStartValue(T value);

		/**
		 * Sets the value of the last point in the curve(s)
		 */
		void setEndValue(T value);
	};


	//////////////////////////////////////////////////////////////////////////
	// Sequence Track Segment Type Definitions
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
			RTTI_PROPERTY("Curves",	&Type::mCurves, nap::rtti::EPropertyMetaData::Default)						\
			RTTI_PROPERTY("Curve Type", &Type::mCurveType, nap::rtti::EPropertyMetaData::Default) \
		RTTI_END_CLASS
}

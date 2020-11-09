/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

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
		std::vector<ResourcePtr<math::FCurve<float, float>>> 	mCurves;		///< Property: 'Curves' vector holding curves
		std::vector<math::ECurveInterp> 						mCurveTypes;	///< Property: 'Curve Types' curve types of this segment ( linear, bezier )

		/**
		 * init evaluates the data hold in curves and checks if its valid for this type
		 * @param errorState contains information about eventual failure of evaluation
		 * @return bool indicating successfull initialization
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
		 * @param pos must be a value between 0-1 ( 0 meaning start of curve, 1 meaning end ) pos is not clamped
		 */
		const T getValue(float pos) const;

		/**
		 * Sets the value of the first point in the curve(s).
		 * Needs to be specialized for every type.
		 */
		void setStartValue(T value);

		/**
		 * Sets the value of the last point in the curve(s)
		 * Needs to be specialized for every type.
		 */
		void setEndValue(T value);

		/**
		 * Returns the total number of curves associated with this track.
		 * Needs to be specialized for every type.
		 * @return total number of curves associated with this track
		 */
		int getCurveCount();
	};


	//////////////////////////////////////////////////////////////////////////
	// Sequence Track Segment Type Definitions
	//////////////////////////////////////////////////////////////////////////

	using SequenceTrackSegmentCurveFloat	= SequenceTrackSegmentCurve<float>;
	using SequenceTrackSegmentCurveVec2		= SequenceTrackSegmentCurve<glm::vec2>;
	using SequenceTrackSegmentCurveVec3		= SequenceTrackSegmentCurve<glm::vec3>;
	using SequenceTrackSegmentCurveVec4		= SequenceTrackSegmentCurve<glm::vec4>;


	//////////////////////////////////////////////////////////////////////////
	// Template Definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename T>
	bool nap::SequenceTrackSegmentCurve<T>::init(utility::ErrorState& errorState)
	{
		if (!SequenceTrackSegment::init(errorState))
			return false;

		if (!errorState.check(mCurves.size() == this->getCurveCount(), 
			"size of curves must be %i", this->getCurveCount()))
			return false;

		if (!errorState.check(mCurveTypes.size() == this->getCurveCount(),
		  	"size of curvetypes must be %i", this->getCurveCount()))
			return false;

		for (int i = 0; i < mCurves.size(); i++)
		{
			if (!errorState.check(mCurves[i]->mPoints.size() >= 2, "curve %i has invalid amount of points", i))
			{
				return false;
			}
		}

		return true;
	}


	//////////////////////////////////////////////////////////////////////////
	// Forward declarations
	//////////////////////////////////////////////////////////////////////////

	template<> 
	NAPAPI const float nap::SequenceTrackSegmentCurveFloat::getStartValue() const;

	template<>
	NAPAPI const float nap::SequenceTrackSegmentCurveFloat::getEndValue() const;

	template<>
	NAPAPI const float nap::SequenceTrackSegmentCurveFloat::getValue(float t) const;

	template<>
	NAPAPI void nap::SequenceTrackSegmentCurveFloat::setStartValue(float t);

	template<>
	NAPAPI void nap::SequenceTrackSegmentCurveFloat::setEndValue(float t);

	template<>
	NAPAPI const glm::vec2 nap::SequenceTrackSegmentCurveVec2::getStartValue() const;

	template<>
	NAPAPI const glm::vec2 nap::SequenceTrackSegmentCurveVec2::getEndValue() const;

	template<>
	NAPAPI const glm::vec2 nap::SequenceTrackSegmentCurveVec2::getValue(float t) const;

	template<>
	NAPAPI void nap::SequenceTrackSegmentCurveVec2::setStartValue(glm::vec2 t);

	template<>
	NAPAPI void nap::SequenceTrackSegmentCurveVec2::setEndValue(glm::vec2 t);

	template<>
	NAPAPI const glm::vec3 nap::SequenceTrackSegmentCurveVec3::getStartValue() const;

	template<>
	NAPAPI const glm::vec3 nap::SequenceTrackSegmentCurveVec3::getValue(float t) const;

	template<>
	NAPAPI const glm::vec3  nap::SequenceTrackSegmentCurveVec3::getEndValue() const;

	template<>
	NAPAPI void nap::SequenceTrackSegmentCurveVec3::setStartValue(glm::vec3 t);

	template<>
	NAPAPI void nap::SequenceTrackSegmentCurveVec3::setEndValue(glm::vec3 t);

	template<>
	NAPAPI const glm::vec4 nap::SequenceTrackSegmentCurveVec4::getStartValue() const;

	template<>
	NAPAPI const glm::vec4 nap::SequenceTrackSegmentCurveVec4::getEndValue() const;

	template<>
	NAPAPI const glm::vec4 nap::SequenceTrackSegmentCurveVec4::getValue(float t) const;

	template<>
	NAPAPI void nap::SequenceTrackSegmentCurveVec4::setStartValue(glm::vec4 t);

	template<>
	NAPAPI void nap::SequenceTrackSegmentCurveVec4::setEndValue(glm::vec4 t);

	template<>
	NAPAPI int nap::SequenceTrackSegmentCurve<float>::getCurveCount();

	template<>
	NAPAPI int nap::SequenceTrackSegmentCurve<glm::vec2>::getCurveCount();

	template<>
	NAPAPI int nap::SequenceTrackSegmentCurve<glm::vec3>::getCurveCount();

	template<>
	NAPAPI int nap::SequenceTrackSegmentCurve<glm::vec4>::getCurveCount();


	//////////////////////////////////////////////////////////////////////////
	// Helper Macro
	//////////////////////////////////////////////////////////////////////////
#define DEFINE_VECTOR_SEQUENCETRACKSEGMENTCURVE(Type)																			\
		RTTI_BEGIN_CLASS(Type)																						\
			RTTI_PROPERTY("Curves",	&Type::mCurves, nap::rtti::EPropertyMetaData::Default)						\
			RTTI_PROPERTY("Curve Types", &Type::mCurveTypes, nap::rtti::EPropertyMetaData::Default) \
		RTTI_END_CLASS
}

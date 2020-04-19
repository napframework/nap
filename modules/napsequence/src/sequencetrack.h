#pragma once

// internal includes
#include "sequencetracksegment.h"
#include "sequencetracktypes.h"

// external includes
#include <nap/resource.h>
#include <nap/resourceptr.h>
#include <parameternumeric.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	/**
	 * SequenceTrack
	 * SequenceTrack holds a collection of track segments
	 */
	class NAPAPI SequenceTrack : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		std::string mAssignedObjectIDs; ///< Property: 'Assigned Object ID' Assigned object to this track id
		std::vector<ResourcePtr<SequenceTrackSegment>>	mSegments; ///< Property: 'Segments' Vector holding track segments

		/*
		 * @return tracktype of this sequence track
		 */
		virtual const SequenceTrackTypes::Types getTrackType() const {
			return SequenceTrackTypes::Types::UNKOWN;
		}
	};

	/**
	 * SequenceTrackCurve
	 * SequenceTrackCurve holds a collection of curvesegments for different types
	 * There are four SequenceTrackCurve types ( float, vec2, vec3, vec4 )
	 */
	template<typename T>
	class NAPAPI SequenceTrackCurve : public SequenceTrack
	{
		RTTI_ENABLE(SequenceTrack)
	public:
		/**
		 * init
		 * initializes the curve segment and validates its data
		 * @param errorState contains any errors
		 * @return returns true on successful initialization
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * getTrackType
		 * returns this tracktype
		 */
		virtual const SequenceTrackTypes::Types getTrackType() const;

		T mMaximum; ///< Property: 'Maximum' maximum value of track
		T mMinimum; ///< Property: 'Minimum' minimum value of track
	};

	/**
	 * SequenceTrackEvent
	 * Event track, holds a collection of SequenceTrackSegmentEvents
	 */
	class NAPAPI SequenceTrackEvent : public SequenceTrack
	{
		RTTI_ENABLE(SequenceTrack)
	public:
		/**
		 * getTrackType
		 * returns this tracktype
		 */
		virtual const SequenceTrackTypes::Types getTrackType() const;
	};

#define DEFINE_SEQUENCETRACKCURVE(Type)																			\
		RTTI_BEGIN_CLASS(Type)																						\
			RTTI_PROPERTY("Minimum",	&Type::mMinimum, nap::rtti::EPropertyMetaData::Default)				\
			RTTI_PROPERTY("Maximum",	&Type::mMaximum, nap::rtti::EPropertyMetaData::Default)				\
		RTTI_END_CLASS
}

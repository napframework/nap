#pragma once

// internal includes
#include "sequencetracksegment.h"

// external includes
#include <nap/resource.h>
#include <nap/resourceptr.h>
#include <parameternumeric.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	namespace SequenceTrackTypes
	{
		enum Types : int
		{
			UNKOWN,
			FLOAT,
			VEC2,
			VEC3,
			VEC4
		};
	}

	/**
	 * SequenceTrack
	 * SequenceTrack holds a collection of track segments
	 */
	class NAPAPI SequenceTrack : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		std::string mAssignedParameterID; ///< Property: 'Parameter ID' Assigned parameter id
		std::vector<ResourcePtr<SequenceTrackSegment>>	mSegments; ///< Property: 'Segments' Vector holding track segments

		virtual const SequenceTrackTypes::Types getTrackType() const {
			return SequenceTrackTypes::Types::UNKOWN;
		}
	};

	template<typename T>
	class NAPAPI SequenceTrackCurve : public SequenceTrack
	{
		RTTI_ENABLE(SequenceTrack)
	public:
		virtual bool init(utility::ErrorState& errorState) override;

		T mMaximum;
		T mMinimum;

		virtual const SequenceTrackTypes::Types getTrackType() const;
	};

#define DEFINE_SEQUENCETRACKCURVE(Type)																			\
		RTTI_BEGIN_CLASS(Type)																						\
			RTTI_PROPERTY("Minimum",	&Type::mMinimum, nap::rtti::EPropertyMetaData::Default)				\
			RTTI_PROPERTY("Maximum",	&Type::mMaximum, nap::rtti::EPropertyMetaData::Default)				\
		RTTI_END_CLASS
}

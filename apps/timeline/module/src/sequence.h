#pragma once

// internal includes
#include "sequencetracksegment.h"
#include "sequencetrack.h"

// external includes
#include <nap/resource.h>
#include <parameter.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	/**
	 * 
	 */
	class NAPAPI SequenceTrackLink : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		std::string					mParameterID;
		ResourcePtr<SequenceTrack>	mSequenceTrack;
	protected:
	};

	/**
	 * 
	 */
	class NAPAPI Sequence : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		static Sequence* createDefaultSequence(
			const std::vector<rtti::ObjectPtr<ParameterFloat>>& parameters,
			std::vector<std::unique_ptr<rtti::Object>>& createdObjects );
	public:
		std::vector<ResourcePtr<SequenceTrackLink>>	mSequenceTrackLinks;
	protected:
	};
}

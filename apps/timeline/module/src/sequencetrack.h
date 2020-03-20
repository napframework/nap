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

	/**
	 * SequenceTrack
	 * SequenceTrack holds a collection of track segments
	 */
	class NAPAPI SequenceTrack : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		/**
		 * createDefaultSequence
		 * static method that creates a default sequence track
		 * @param createdObject a reference to a vector that will be filled with unique pointers of created objects
		 * @param objectIDs a list of unique ids, used to created unique ids for each object in this sequence track
		 * @return a raw pointer to the newly created sequence track
		 */
		static SequenceTrack* createDefaultSequenceTrack(
			std::vector<std::unique_ptr<rtti::Object>>& createdObjects,
			std::unordered_set<std::string>& objectIDs );
	public:
		std::vector<ResourcePtr<SequenceTrackSegment>>	mSegments; ///< Property: 'Segments' Vector holding track segments
	};
}

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
	 * SequenceTrackLink
	 * SequenceTrackLink links a parameter to a sequence track. A SequenceTrackLink holds the parameter unique id
	 * that points to the parameter. This parameter can be deleted, but the sequence will hold the information of
	 * the animation of this parameter, which can be assigned to another parameter of the same type later
	 */
	class NAPAPI SequenceTrackLink : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		std::string					mParameterID;	///< Property: 'Parameter ID' id of parameter the sequence track links to
		ResourcePtr<SequenceTrack>	mSequenceTrack; ///< Property: 'Sequence Track' resource ptr to sequence track, embedded in this track link
	protected:
	};

	/**
	 * Sequence
	 * Sequence contains a vector of sequence track links
	 */
	class NAPAPI Sequence : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		/**
		 * createDefaultSequence
		 * static method that creates a default sequence based on given parameters. 
		 * It will created default sequence tracks for each given parameter
		 * @param parameters vector of parameters that we want to animate
		 * @param createdObject a reference to a vector that will be filled with unique pointers of created objects
		 * @param objectIDs a list of unique ids, used to created unique ids for each object in this sequence
		 * @return a raw pointer to the newly created sequence
		 */
		static Sequence* createDefaultSequence(
			const std::vector<rtti::ObjectPtr<ParameterFloat>>& parameters,
			std::vector<std::unique_ptr<rtti::Object>>& createdObjects,
			std::unordered_set<std::string>& objectIDs);
	public:
		std::vector<ResourcePtr<SequenceTrackLink>>	mSequenceTrackLinks; ///< Property: 'Sequence Track Links' Vector holding resourceptrs to the SequenceTrackLinks
	protected:
	};
}

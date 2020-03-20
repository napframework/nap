// local includes
#include "sequencetrack.h"

// external includes
#include <nap/resourceptr.h>

RTTI_BEGIN_CLASS(nap::SequenceTrack)
	RTTI_PROPERTY("Segments", &nap::SequenceTrack::mSegments, nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	static const std::string generateUniqueID(
		std::unordered_set<std::string>& objectIDs,
		const std::string& baseID = "Generated")
	{
		std::string unique_id = baseID;

		int index = 1;
		while (objectIDs.find(unique_id) != objectIDs.end())
			unique_id = utility::stringFormat("%s_%d", baseID.c_str(), ++index);

		objectIDs.insert(unique_id);

		return unique_id;
	}


	SequenceTrack* SequenceTrack::createDefaultSequenceTrack(	std::vector<std::unique_ptr<rtti::Object>>& createdObjects,
																std::unordered_set<std::string>& objectIDs)
	{
		// create one segment
		std::unique_ptr<SequenceTrackSegment> trackSegment = std::make_unique<SequenceTrackSegment>();
		trackSegment->mID = generateUniqueID(objectIDs);
		trackSegment->mDuration = 1.0;
		trackSegment->mStartTime = 0.0;

		// create default curve
		std::unique_ptr<math::FloatFCurve> segmentCurve = std::make_unique<math::FloatFCurve>();
		segmentCurve->mID = generateUniqueID(objectIDs);

		// assign curve
		trackSegment->mCurve = nap::ResourcePtr<math::FloatFCurve>(segmentCurve.get());

		// create sequence track
		std::unique_ptr<SequenceTrack> sequenceTrack = std::make_unique<SequenceTrack>();
		sequenceTrack->mID = generateUniqueID(objectIDs);

		// assign track segment
		sequenceTrack->mSegments.emplace_back(ResourcePtr<SequenceTrackSegment>(trackSegment.get()));

		// assign return ptr
		SequenceTrack* returnPtr = sequenceTrack.get();

		// move ownership of unique ptrs
		createdObjects.emplace_back(std::move(trackSegment));
		createdObjects.emplace_back(std::move(segmentCurve));
		createdObjects.emplace_back(std::move(sequenceTrack));

		return returnPtr;
	}
}

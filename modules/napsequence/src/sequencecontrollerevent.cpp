#include "sequencecontrollerevent.h"
#include "sequenceutils.h"
#include "sequencetrackevent.h"
#include "sequenceeditor.h"

namespace nap
{
	static bool sRegistered = SequenceController::registerControllerFactory(RTTI_OF(SequenceControllerEvent), [](SequencePlayer& player)->std::unique_ptr<SequenceController> { return std::make_unique<SequenceControllerEvent>(player); });

	static bool sRegisterControllerType = SequenceEditor::registerControllerForTrackType(RTTI_OF(SequenceTrackEvent), RTTI_OF(SequenceControllerEvent));

	void SequenceControllerEvent::segmentEventStartTimeChange(const std::string& trackID, const std::string& segmentID, float amount)
	{
		// pause player thread
		std::unique_lock<std::mutex> lock = mPlayer.lock();

		auto* segment = findSegment(trackID, segmentID);
		assert(segment != nullptr); // segment not found

		if (segment != nullptr)
		{
			assert(segment->get_type().is_derived_from(RTTI_OF(SequenceTrackSegmentEventBase))); // type mismatch

			if (segment->get_type().is_derived_from(RTTI_OF(SequenceTrackSegmentEventBase)))
			{
				auto& segmentEvent = static_cast<SequenceTrackSegmentEventBase&>(*segment);
				segmentEvent.mStartTime += amount;
			}

		}

		updateTracks();
	}


	void SequenceControllerEvent::insertSegment(const std::string& trackID, double time)
	{
		nap::Logger::warn("insertSegment not used, use insertEventSegment instead");
	}


	void SequenceControllerEvent::deleteSegment(const std::string& trackID, const std::string& segmentID)
	{
		// pause player thread
		std::unique_lock<std::mutex> lock = mPlayer.lock();

		//
		Sequence& sequence = mPlayer.getSequence();

		//
		auto* track = findTrack(trackID);
		assert(track != nullptr); // track not found

		if (track != nullptr)
		{
			int segmentIndex = 0;
			for (auto& segment : track->mSegments)
			{
				if (segment->mID == segmentID)
				{
					// erase it from the list
					track->mSegments.erase(track->mSegments.begin() + segmentIndex);

					deleteObjectFromSequencePlayer(segmentID);

					break;
				}

				updateTracks();
				segmentIndex++;
			}
		}
	}


	void SequenceControllerEvent::addNewEventTrack()
	{
		std::unique_lock<std::mutex> l = mPlayer.lock();

		// create sequence track
		std::unique_ptr<SequenceTrackEvent> sequenceTrack = std::make_unique<SequenceTrackEvent>();
		sequenceTrack->mID = sequenceutils::generateUniqueID(mPlayer.mReadObjectIDs);

		//
		mPlayer.getSequence().mTracks.emplace_back(ResourcePtr<SequenceTrackEvent>(sequenceTrack.get()));

		// move ownership of unique ptrs
		mPlayer.mReadObjects.emplace_back(std::move(sequenceTrack));
	}


	void SequenceControllerEvent::insertTrack(rttr::type type)
	{
		addNewEventTrack();
	}
}
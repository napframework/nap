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
			assert(segment->get_type().is_derived_from(RTTI_OF(SequenceTrackSegmentEvent))); // type mismatch

			if (segment->get_type().is_derived_from(RTTI_OF(SequenceTrackSegmentEvent)))
			{
				auto& segmentEvent = static_cast<SequenceTrackSegmentEvent&>(*segment);
				segmentEvent.mStartTime += amount;
			}

		}

		updateTracks();
	}


	void SequenceControllerEvent::insertSegment(const std::string& trackID, double time)
	{
		insertEventSegment(trackID, time);
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

		Sequence& sequence = mPlayer.getSequence();

		SequenceTrack* newTrack = sequenceutils::createSequenceEventTrack(mPlayer.mReadObjects, mPlayer.mReadObjectIDs);

		sequence.mTracks.emplace_back(ResourcePtr<SequenceTrack>(newTrack));
	}


	void SequenceControllerEvent::insertEventSegment(const std::string& trackID, double time)
	{
		auto l = mPlayer.lock();

		// create new segment & set parameters
		std::unique_ptr<SequenceTrackSegmentEvent> newSegment = std::make_unique<SequenceTrackSegmentEvent>();
		newSegment->mStartTime = time;
		newSegment->mMessage = "Hello World!";
		newSegment->mID = sequenceutils::generateUniqueID(mPlayer.mReadObjectIDs);
		newSegment->mDuration = 0.0;

		//
		SequenceTrack* track = findTrack(trackID);
		assert(track != nullptr); // track not found

		if (track != nullptr)
		{
			track->mSegments.emplace_back(ResourcePtr<SequenceTrackSegmentEvent>(newSegment.get()));

			mPlayer.mReadObjects.emplace_back(std::move(newSegment));
		}
	}


	void SequenceControllerEvent::editEventSegment(const std::string& trackID, const std::string& segmentID, const std::string& eventMessage)
	{
		auto l = mPlayer.lock();

		SequenceTrack* track = findTrack(trackID);
		assert(track != nullptr); // track not found

		if (track != nullptr)
		{

			SequenceTrackSegment* segment = findSegment(trackID, segmentID);
			assert(segment != nullptr); // segment not found

			if (segment != nullptr)
			{
				SequenceTrackSegmentEvent* eventSegment = dynamic_cast<SequenceTrackSegmentEvent*>(segment);
				assert(eventSegment != nullptr); // type mismatch

				if (eventSegment != nullptr)
				{
					eventSegment->mMessage = eventMessage;
				}
			}
		}
	}


	void SequenceControllerEvent::insertTrack(rttr::type type)
	{
		addNewEventTrack();
	}
}
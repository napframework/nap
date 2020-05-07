#include "sequencecontroller.h"

#include <nap/logger.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	std::unordered_map<rttr::type, SequenceControllerFactoryFunc>& SequenceController::getControllerFactory()
	{
		static std::unordered_map<rttr::type, SequenceControllerFactoryFunc> factory;
		return factory;
	}

	bool SequenceController::registerControllerFactory(rttr::type type, SequenceControllerFactoryFunc func)
	{
		auto& factory = getControllerFactory();
		auto it = factory.find(type);
		assert(it == factory.end()); // duplicate entry
		if (it == factory.end())
		{
			factory.emplace(type, func);

			return true;
		}

		return false;
	}


	void SequenceController::updateTracks()
	{
		double longestTrackDuration = 0.0;
		for (auto& track : mPlayer.mSequence->mTracks)
		{
			double trackDuration = 0.0;
			double highestSegment = 0.0;

			for (const auto& segment : track->mSegments)
			{
				if (segment->mStartTime + segment->mDuration > highestSegment)
				{
					highestSegment = segment->mStartTime + segment->mDuration;
					trackDuration = highestSegment;
				}
			}

			if (trackDuration > longestTrackDuration)
			{
				longestTrackDuration = trackDuration;
			}
		}

		for (auto& track : mPlayer.mSequence->mTracks)
		{
			mPlayer.mSequence->mDuration = longestTrackDuration;
		}
	}




	SequenceTrackSegment* SequenceController::findSegment(const std::string& trackID, const std::string& segmentID)
	{
		//
		Sequence& sequence = mPlayer.getSequence();

		for (auto& track : sequence.mTracks)
		{
			if (track->mID == trackID)
			{
				for (auto& segment : track->mSegments)
				{
					if (segment->mID == segmentID)
					{
						return segment.get();
					}
				}
			}
		}

		return nullptr;
	}

	const SequenceTrackSegment* SequenceController::getSegment(const std::string& trackID, const std::string& segmentID) const
	{
		//
		const Sequence& sequence = mPlayer.getSequenceConst();

		for (const auto& track : sequence.mTracks)
		{
			if (track->mID == trackID)
			{
				for (auto& segment : track->mSegments)
				{
					if (segment->mID == segmentID)
					{
						return segment.get();
					}
				}
			}
		}

		return nullptr;
	}


	SequenceTrack* SequenceController::findTrack(const std::string& trackID)
	{
		//
		Sequence& sequence = mPlayer.getSequence();

		for (auto& track : sequence.mTracks)
		{
			if (track->mID == trackID)
			{
				return track.get();
			}
		}

		return nullptr;
	}


	void SequenceController::assignNewObjectID(const std::string& trackID, const std::string& objectID)
	{
		std::unique_lock<std::mutex> l = mPlayer.lock();

		SequenceTrack* track = findTrack(trackID);
		assert(track != nullptr); // track not found

		if (track != nullptr)
		{
			if (mPlayer.createAdapter(objectID, trackID, l))
			{
				track->mAssignedObjectIDs = objectID;
			}
		}
	}


	void SequenceController::deleteTrack(const std::string& deleteTrackID)
	{
		auto lock = mPlayer.lock();

		//
		Sequence& sequence = mPlayer.getSequence();

		int index = 0;
		for (const auto& track : sequence.mTracks)
		{
			if (track->mID == deleteTrackID)
			{
				if (mPlayer.mAdapters.find(track->mID) != mPlayer.mAdapters.end())
				{
					mPlayer.mAdapters.erase(track->mID);
				}

				sequence.mTracks.erase(sequence.mTracks.begin() + index);

				deleteObjectFromSequencePlayer(deleteTrackID);

				break;
			}
			index++;
		}
	}


	void SequenceController::deleteObjectFromSequencePlayer(const std::string& id)
	{
		if (mPlayer.mReadObjectIDs.find(id) != mPlayer.mReadObjectIDs.end())
		{
			mPlayer.mReadObjectIDs.erase(id);
		}

		for (int i = 0; i < mPlayer.mReadObjects.size(); i++)
		{
			if (mPlayer.mReadObjects[i]->mID == id)
			{
				mPlayer.mReadObjects.erase(mPlayer.mReadObjects.begin() + i);
				break;
			}
		}
	}
}
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "sequencecontroller.h"
#include "sequenceeditor.h"

#include <nap/logger.h>
#include <mathutils.h>

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
		double longest_track_duration = 0.0;
		for (auto& track : mPlayer.mSequence->mTracks)
		{
			double track_duration = 0.0;
			double longest_segment = 0.0;

			for (const auto& segment : track->mSegments)
			{
				if (segment->mStartTime + segment->mDuration > longest_segment)
				{
					longest_segment = segment->mStartTime + segment->mDuration;
					track_duration = longest_segment;
				}
			}

			if (track_duration > longest_track_duration)
			{
				longest_track_duration = track_duration;
			}
		}

		mPlayer.mSequence->mDuration = math::max<double>(longest_track_duration, mPlayer.mSequence->mDuration);
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


	const SequenceTrack* SequenceController::getTrack(const std::string& trackID) const
	{
		//
		const Sequence& sequence = mPlayer.getSequenceConst();

		for (const auto& track : sequence.mTracks)
		{
			if (track->mID == trackID)
			{
				return track.get();
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
		performEditAction([this, trackID, objectID]()
		{
			SequenceTrack* track = findTrack(trackID);
			assert(track != nullptr); // track not found

			if (track != nullptr)
			{
				track->mAssignedOutputID = objectID;
			}

			mPlayer.createAdapters();
		});
	}


	void SequenceController::deleteTrack(const std::string& deleteTrackID)
	{
		performEditAction([this, deleteTrackID]()
		{
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
		});
	}


	void SequenceController::moveTrackUp(const std::string& trackID)
	{
		performEditAction([this, trackID]()
		{
			//
			Sequence& sequence = mPlayer.getSequence();

			int index = 0;
			for (const auto& track : sequence.mTracks)
			{
				if (track->mID == trackID)
				{
					if( index > 0 )
					{
						auto track = sequence.mTracks[index];
						sequence.mTracks.erase(sequence.mTracks.begin() + index);
						sequence.mTracks.emplace(sequence.mTracks.begin() + ( index - 1 ), track);
					}

					break;
				}
				index++;
			}
		});
	}


	void SequenceController::moveTrackDown(const std::string& trackID)
	{
		performEditAction([this, trackID]()
		{
			//
			Sequence& sequence = mPlayer.getSequence();

			int index = 0;
			for (const auto& track : sequence.mTracks)
			{
				if (track->mID == trackID)
				{
					if( index < sequence.mTracks.size() - 1 )
					{
						auto track = sequence.mTracks[index];
						sequence.mTracks.erase(sequence.mTracks.begin() + index);
						sequence.mTracks.emplace(sequence.mTracks.begin() + ( index + 1 ), track);
					}

					break;
				}
				index++;
			}
		});
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


	void SequenceController::performEditAction(std::function<void()> action)
	{
		mEditor.queueEditAction(action);
	}
}
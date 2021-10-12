/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// local includes
#include "sequencecontrolleraudio.h"
#include "sequencetrackaudio.h"
#include "sequenceplayeraudiooutput.h"

// nap includes
#include <sequenceeditor.h>

namespace nap
{
	double SequenceControllerAudio::segmentAudioStartTimeChange(const std::string& trackID, const std::string& segmentID, double time)
	{
		double return_time = time;
		performEditAction([this, trackID, segmentID, time, &return_time]()
	    {
			auto* segment = findSegment(trackID, segmentID);
			assert(segment != nullptr); // segment not found
			assert(segment->get_type().is_derived_from(RTTI_OF(SequenceTrackSegmentAudio))); // type mismatch

			if (segment->get_type().is_derived_from(RTTI_OF(SequenceTrackSegmentAudio)))
			{
				auto& segment_audio = static_cast<SequenceTrackSegmentAudio&>(*segment);
				segment_audio.mStartTime = time;

				if( segment_audio.mStartTime < 0.0)
					segment_audio.mStartTime = 0.0;

				return_time = segment_audio.mStartTime;
			}

		  	alignAudioSegments(trackID);

			updateTracks();
		});

		return return_time;
	}


	double SequenceControllerAudio::segmentAudioStartTimeInSegmentChange(const std::string& trackID, const std::string& segmentID, double time)
	{
		double new_time = 0.0;
		performEditAction([this, trackID, segmentID, time, &new_time]()
		{
			auto* segment = findSegment(trackID, segmentID);
			assert(segment != nullptr); // segment not found
			assert(segment->get_type().is_derived_from(RTTI_OF(SequenceTrackSegmentAudio))); // type mismatch

			auto& segment_audio = static_cast<SequenceTrackSegmentAudio&>(*segment);
			auto* audio_buffer = findAudioBufferForTrack(trackID, segment_audio.mAudioBufferID);

			if(audio_buffer!= nullptr)
			{
				if(time >= 0.0f && time < audio_buffer->getSize() / audio_buffer->getSampleRate())
				{
					new_time = time;
					double difference = new_time - segment_audio.mStartTimeInAudioSegment;
					segment_audio.mStartTimeInAudioSegment = new_time;
					segment_audio.mDuration -= difference;
				}
			}
		});

		return new_time;
	}


	double SequenceControllerAudio::segmentAudioDurationChange(const std::string& trackID, const std::string& segmentID, double newDuration)
	{
		double adjusted_duration = 0.0;
		performEditAction([this, trackID, segmentID, newDuration, &adjusted_duration]()
		{
			auto* segment = findSegment(trackID, segmentID);
			assert(segment != nullptr); // segment not found
			assert(segment->get_type().is_derived_from(RTTI_OF(SequenceTrackSegmentAudio))); // type mismatch

			auto& segment_audio = static_cast<SequenceTrackSegmentAudio&>(*segment);
			auto* audio_buffer = findAudioBufferForTrack(trackID, segment_audio.mAudioBufferID);
		  	adjusted_duration = segment_audio.mDuration;

			if(audio_buffer!= nullptr)
			{
				if(newDuration > 0.0f && newDuration < audio_buffer->getSize() / audio_buffer->getSampleRate())
				{
					adjusted_duration = newDuration;
					segment_audio.mDuration = adjusted_duration;
				}
			}
		});

		return adjusted_duration;
	}


	const SequenceTrackSegment* SequenceControllerAudio::insertSegment(const std::string& trackID, double time)
	{
		nap::Logger::warn("insertSegment not used, use insertAudioSegment instead");
		return nullptr;
	}


	void SequenceControllerAudio::deleteSegment(const std::string& trackID, const std::string& segmentID)
	{
		performEditAction([this, trackID, segmentID]()
		{
			//
			auto* track = findTrack(trackID);
			assert(track != nullptr); // track not found

			int segment_index = 0;
			for (auto& segment : track->mSegments)
			{
				if (segment->mID == segmentID)
				{
					// erase it from the list
					track->mSegments.erase(track->mSegments.begin() + segment_index);

					deleteObjectFromSequencePlayer(segmentID);

					break;
				}

				updateTracks();
				segment_index++;
			}
		});
	}


	std::string SequenceControllerAudio::insertAudioSegment(const std::string& trackID, double time, const std::string& audioBufferID)
	{
		std::string created_segment_id = "";
		performEditAction([this, trackID, time, audioBufferID, &created_segment_id]() mutable
		{
			// create new segment & set parameters
			std::unique_ptr<SequenceTrackSegmentAudio> new_segment = std::make_unique<SequenceTrackSegmentAudio>();
			new_segment->mStartTime = time;
			new_segment->mID = mService.generateUniqueID(getPlayerReadObjectIDs());
			new_segment->mAudioBufferID = audioBufferID;
		  	created_segment_id = new_segment->mID;

			// set duration to audio buffer length
		  	auto* buffer = findAudioBufferForTrack(trackID, new_segment->mAudioBufferID);
			if(buffer!= nullptr)
			{
				new_segment->mDuration = buffer->getSize() / buffer->getSampleRate();
			}else
			{
				new_segment->mDuration = 1.0;
				nap::Logger::error("Couldn't find buffer id %s", audioBufferID.c_str());
			}

			//
			SequenceTrack* track = findTrack(trackID);
			assert(track != nullptr); // track not found

			track->mSegments.emplace_back(ResourcePtr<SequenceTrackSegmentAudio>(new_segment.get()));

			getPlayerOwnedObjects().emplace_back(std::move(new_segment));

		  	alignAudioSegments(trackID);

			updateTracks();
		});

		return created_segment_id;
	}


	std::string SequenceControllerAudio::insertAudioSegment(const std::string& trackID,
															double time,
															const std::string& audioBufferID,
															double duration,
															double startTimeInSegment)
	{
		std::string created_segment_id = "";
		performEditAction([	this,
						   	trackID,
						   	time,
						   	audioBufferID,
						   	&created_segment_id,
							duration,
							startTimeInSegment]() mutable
		{
			// create new segment & set parameters
			std::unique_ptr<SequenceTrackSegmentAudio> new_segment = std::make_unique<SequenceTrackSegmentAudio>();
			new_segment->mStartTime = time;
			new_segment->mID = mService.generateUniqueID(getPlayerReadObjectIDs());
			new_segment->mAudioBufferID = audioBufferID;
			created_segment_id = new_segment->mID;

			// set duration to audio buffer length
			new_segment->mDuration = duration;
			new_segment->mStartTimeInAudioSegment = startTimeInSegment;

			//
			SequenceTrack* track = findTrack(trackID);
			assert(track != nullptr); // track not found

			track->mSegments.emplace_back(ResourcePtr<SequenceTrackSegmentAudio>(new_segment.get()));

			getPlayerOwnedObjects().emplace_back(std::move(new_segment));

			alignAudioSegments(trackID);

			updateTracks();
		});

		return created_segment_id;
	}


	void SequenceControllerAudio::addNewAudioTrack()
	{
		performEditAction([this]()
		{
			// create sequence track
			std::unique_ptr<SequenceTrackAudio> sequence_track = std::make_unique<SequenceTrackAudio>();
			sequence_track->mID = mService.generateUniqueID(getPlayerReadObjectIDs());

			//
			getSequence().mTracks.emplace_back(ResourcePtr<SequenceTrackAudio>(sequence_track.get()));

			// move ownership of unique ptrs
			getPlayerOwnedObjects().emplace_back(std::move(sequence_track));
		});
	}


	void SequenceControllerAudio::insertTrack(rttr::type type)
	{
		assert(type==RTTI_OF(SequenceTrackAudio));
		addNewAudioTrack();
	}


	audio::AudioBufferResource* SequenceControllerAudio::findAudioBufferForTrack(const std::string& trackID, const std::string& audioBufferID)
	{
		SequenceTrack* track = findTrack(trackID);
		assert(track != nullptr); // track not found

		const SequencePlayerAudioOutput* audio_output = nullptr;
		for(const auto& output : mPlayer.mOutputs)
		{
			if(output.get()->get_type().is_derived_from<SequencePlayerAudioOutput>())
			{
				if(track->mAssignedOutputID==output->mID)
				{
					audio_output = static_cast<const SequencePlayerAudioOutput*>(output.get());
					break;
				}
			}
		}

		if(audio_output== nullptr)
			return nullptr;

		for(const auto& buffer : audio_output->getBuffers())
		{
			if(buffer->mID==audioBufferID)
			{
				return buffer.get();
			}
		}

		return nullptr;
	}


	void SequenceControllerAudio::changeAudioSegmentAudioBuffer(const std::string& trackID, const std::string& segmentID, const std::string& audioBufferID)
	{
		performEditAction([this, trackID, segmentID, audioBufferID]()
		{
			auto* buffer = findAudioBufferForTrack(trackID, audioBufferID);
			assert(buffer!= nullptr);

			auto* segment = findSegment(trackID, segmentID);
			assert(segment->get_type()==RTTI_OF(SequenceTrackSegmentAudio));

			auto* segment_audio = static_cast<SequenceTrackSegmentAudio*>(segment);
			segment_audio->mAudioBufferID = audioBufferID;
			segment_audio->mDuration = buffer->getSize() / buffer->getSampleRate();

			alignAudioSegments(trackID);

			updateTracks();
		});
	}


	void SequenceControllerAudio::alignAudioSegments(const std::string& trackID)
	{
		//
		SequenceTrack* track = findTrack(trackID);
		assert(track != nullptr); // track not found

		std::sort (track->mSegments.begin(), track->mSegments.end(),
				  [](ResourcePtr<SequenceTrackSegmentAudio> a, ResourcePtr<SequenceTrackSegmentAudio> b)->bool
		{
			return a->mStartTime < b->mStartTime;
		});

		SequenceTrackSegment* prev_segment = nullptr;
		for(auto segment_resourceptr : track->mSegments)
		{
			auto* segment = segment_resourceptr.get();
			if(prev_segment!= nullptr)
			{
				if(segment->mStartTime < prev_segment->mStartTime + prev_segment->mDuration)
				{
					segment->mStartTime = prev_segment->mStartTime + prev_segment->mDuration;
				}
			}
			prev_segment = segment;
		}
	}
}
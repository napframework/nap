/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "sequencecontroller.h"
#include "sequenceeditor.h"
#include "sequenceservice.h"
#include "sequencetracksegmentcurve.h"

#include <nap/logger.h>
#include <mathutils.h>

#include <utility>

namespace nap
{
    //////////////////////////////////////////////////////////////////////////

    void SequenceController::changeTrackName(const std::string& trackID, const std::string& name)
    {
        performEditAction([this, trackID, name]()
                          {
                              SequenceTrack *track = findTrack(trackID);
                              assert(track != nullptr); // track not found
                              track->mName = name;
                          });
    }


    void SequenceController::changeSegmentLabel(const std::string& trackID, const std::string& segmentID,
                                                const std::string& newLabel)
    {
        performEditAction([this, trackID, segmentID, newLabel]()
                          {
                              SequenceTrackSegment *segment = findSegment(trackID, segmentID);
                              assert(segment != nullptr); // segment not found
                              segment->mLabel = newLabel;
                          });
    }


    void SequenceController::changeTrackHeight(const std::string& trackID, float newHeight)
    {
        performEditAction([this, trackID, newHeight]()
                          {
                              SequenceTrack *track = findTrack(trackID);
                              assert(track != nullptr); // track not found
                              track->mTrackHeight = newHeight;
                          });
    }


    void SequenceController::updateTracks()
    {
        double longest_track_duration = 0.0;
        for(auto &track: mPlayer.mSequence->mTracks)
        {
            double track_duration = 0.0;
            double longest_segment = 0.0;

            // get the last segment on the track
            for(const auto &track_segment: track->mSegments)
            {
                if(track_segment->mStartTime > longest_segment)
                {
                    longest_segment = track_segment->mStartTime;
                    track_duration = longest_segment;

                    // if the segment has a duration add it to the track duration
                    const auto& curve_segment = static_cast<const SequenceTrackSegmentDuration&>(*track_segment.get());
                    if(track_segment->mStartTime + curve_segment.mDuration > track_duration)
                    {
                        track_duration = track_segment->mStartTime + curve_segment.mDuration;
                        longest_segment = track_duration;
                    }
                }
            }

            if(track_duration > longest_track_duration)
            {
                longest_track_duration = track_duration;
            }
        }

        mPlayer.mSequence->mDuration = math::max<double>(longest_track_duration, mPlayer.mSequence->mDuration);
    }


    SequenceTrackSegment* SequenceController::findSegment(const std::string& trackID, const std::string& segmentID)
    {
        Sequence& sequence = mPlayer.getSequence();
        const auto& tracks = sequence.mTracks;
        auto track_it = std::find_if(tracks.cbegin(),
                                     tracks.cend(),
                                     [trackID](const rtti::ObjectPtr<SequenceTrack> &other) -> bool
                                     {
                                         return trackID == other->mID;
                                     });

        if(track_it!=tracks.cend())
        {
            const auto& segments = track_it->get()->mSegments;
            auto segment_it = std::find_if(segments.cbegin(),
                                           segments.cend(),
                                           [segmentID](const rtti::ObjectPtr<SequenceTrackSegment> &other) -> bool
                                           {
                                               return segmentID == other->mID;
                                           });

            if(segment_it!=segments.cend())
            {
                return segment_it->get();
            }
        }

        return nullptr;
    }


    const SequenceTrack* SequenceController::getTrack(const std::string& trackID) const
    {
        Sequence& sequence = mPlayer.getSequence();
        const auto& tracks = sequence.mTracks;
        auto track_it = std::find_if(tracks.cbegin(),
                                     tracks.cend(),
                                     [trackID](const rtti::ObjectPtr<SequenceTrack> &other) -> bool
                                     {
                                         return trackID == other->mID;
                                     });

        if(track_it!=tracks.cend())
        {
            return track_it->get();
        }

        return nullptr;
    }


    const SequenceTrackSegment* SequenceController::getSegment(const std::string& trackID, const std::string& segmentID) const
    {
        Sequence& sequence = mPlayer.getSequence();
        const auto& tracks = sequence.mTracks;
        auto track_it = std::find_if(tracks.cbegin(),
                                     tracks.cend(),
                                     [trackID](const rtti::ObjectPtr<SequenceTrack> &other) -> bool
                                     {
                                         return trackID == other->mID;
                                     });

        if(track_it!=tracks.cend())
        {
            const auto& segments = track_it->get()->mSegments;
            auto segment_it = std::find_if(segments.cbegin(),
                                           segments.cend(),
                                           [segmentID](const rtti::ObjectPtr<SequenceTrackSegment> &other) -> bool
                                           {
                                               return segmentID == other->mID;
                                           });

            if(segment_it!=segments.cend())
            {
                return segment_it->get();
            }
        }

        return nullptr;
    }


    SequenceTrack* SequenceController::findTrack(const std::string& trackID)
    {
        Sequence& sequence = mPlayer.getSequence();
        const auto& tracks = sequence.mTracks;
        auto track_it = std::find_if(tracks.cbegin(),
                                     tracks.cend(),
                                     [trackID](const rtti::ObjectPtr<SequenceTrack> &other) -> bool
                                     {
                                         return trackID == other->mID;
                                     });

        if(track_it!=tracks.cend())
        {
            return track_it->get();
        }

        return nullptr;
    }


    void SequenceController::assignNewOutputID(const std::string& trackID, const std::string& outputID)
    {
        performEditAction([this, trackID, outputID]()
                          {
                              SequenceTrack *track = findTrack(trackID);
                              assert(track != nullptr); // track not found
                              track->mAssignedOutputID = outputID;

                              mPlayer.createAdapters();
                          });
    }


    void SequenceController::deleteTrack(const std::string& deleteTrackID)
    {
        performEditAction([this, deleteTrackID]()
                          {
                              Sequence &sequence = mPlayer.getSequence();
                              auto& tracks = sequence.mTracks;

                              auto track_it = std::find_if(tracks.cbegin(),
                                                           tracks.cend(),
                                                           [deleteTrackID](const rtti::ObjectPtr<SequenceTrack> &other) -> bool
                                                           {
                                                               return deleteTrackID == other->mID;
                                                           });

                              if(track_it!= tracks.end())
                              {
                                  tracks.erase(track_it);
                                  mPlayer.destroyAdapters();
                                  deleteObjectFromSequencePlayer(deleteTrackID);
                                  mPlayer.createAdapters();
                              }
                          });
    }


    void SequenceController::moveTrackUp(const std::string& trackID)
    {
        performEditAction([this, trackID]()
                          {
                              Sequence &sequence = mPlayer.getSequence();

                              int index = 0;
                              for(const auto &track: sequence.mTracks)
                              {
                                  if(track->mID == trackID)
                                  {
                                      if(index > 0)
                                      {
                                          auto track_to_move = sequence.mTracks[index];
                                          sequence.mTracks.erase(sequence.mTracks.begin() + index);
                                          sequence.mTracks.emplace(sequence.mTracks.begin() + (index - 1),
                                                                   track_to_move);
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
                              Sequence &sequence = mPlayer.getSequence();

                              int index = 0;
                              for(const auto &track: sequence.mTracks)
                              {
                                  if(track->mID == trackID)
                                  {
                                      if(index < sequence.mTracks.size() - 1)
                                      {
                                          auto track_to_move = sequence.mTracks[index];
                                          sequence.mTracks.erase(sequence.mTracks.begin() + index);
                                          sequence.mTracks.emplace(sequence.mTracks.begin() + (index + 1),
                                                                   track_to_move);
                                      }

                                      break;
                                  }
                                  index++;
                              }
                          });
    }


    void SequenceController::deleteObjectFromSequencePlayer(const std::string& id)
    {
        if(mPlayer.mReadObjectIDs.find(id) != mPlayer.mReadObjectIDs.end())
        {
            mPlayer.mReadObjectIDs.erase(id);
        }

        for(int i = 0; i < mPlayer.mReadObjects.size(); i++)
        {
            if(mPlayer.mReadObjects[i]->mID == id)
            {
                mPlayer.mReadObjects.erase(mPlayer.mReadObjects.begin() + i);
                break;
            }
        }
    }


    void SequenceController::performEditAction(std::function<void()> action)
    {
        mEditor.performEdit(std::move(action));
    }
}

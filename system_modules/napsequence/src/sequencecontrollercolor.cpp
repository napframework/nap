/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "sequencecontrollercolor.h"
#include "sequencetrackcolor.h"
#include "sequenceeditor.h"

namespace nap
{
    double SequenceControllerColor::segmentColorStartTimeChange(const std::string &trackID,
                                                                const std::string &segmentID, double time)
    {
        double return_time = time;
        performEditAction([this, trackID, segmentID, time, &return_time]()
                          {
                              auto *segment = findSegment(trackID, segmentID);
                              assert(segment != nullptr); // segment not found
                              assert(segment->get_type().is_derived_from(RTTI_OF(SequenceTrackSegmentColor))); // type mismatch

                              auto &segment_color = static_cast<SequenceTrackSegmentColor &>(*segment);
                              segment_color.mStartTime = time;
                              return_time = segment_color.mStartTime;

                              updateTracks();
                          });

        return return_time;
    }


    const SequenceTrackSegment *SequenceControllerColor::insertSegment(const std::string &trackID, double time)
    {
        SequenceTrackSegment *return_ptr = nullptr;

        performEditAction([this, trackID, time, &return_ptr]() mutable
                          {
                              // create new segment & set parameters
                              std::unique_ptr<SequenceTrackSegmentColor> new_segment = std::make_unique<SequenceTrackSegmentColor>();
                              new_segment->mStartTime = time;
                              new_segment->mID = mService.generateUniqueID(getPlayerReadObjectIDs());

                              // find track
                              SequenceTrack *track = findTrack(trackID);
                              assert(track != nullptr); // track not found

                              // add segment to track
                              track->mSegments.emplace_back(ResourcePtr<SequenceTrackSegmentColor>(new_segment.get()));

                              // resolve return ptr
                              return_ptr = new_segment.get();

                              // move ownership
                              getPlayerOwnedObjects().emplace_back(std::move(new_segment));

                              updateTracks();
                          });

        return return_ptr;
    }


    void SequenceControllerColor::deleteSegment(const std::string &trackID, const std::string &segmentID)
    {
        performEditAction([this, trackID, segmentID]()
                          {
                              auto *track = findTrack(trackID);
                              assert(track != nullptr); // track not found

                              int segment_index = 0;
                              for(auto &segment: track->mSegments)
                              {
                                  if(segment->mID == segmentID)
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


    void SequenceControllerColor::changeSegmentColor(const std::string &trackID, const std::string &segmentID, const nap::RGBAColorFloat &color)
    {
        performEditAction([this, trackID, segmentID, color]()
                          {
                              auto *segment = findSegment(trackID, segmentID);
                              assert(segment != nullptr); // segment not found
                              assert(segment->get_type().is_derived_from(RTTI_OF(SequenceTrackSegmentColor))); // type mismatch

                              auto &segment_color = static_cast<SequenceTrackSegmentColor &>(*segment);
                              segment_color.mColor = color;
                          });
    }


    void SequenceControllerColor::changeSegmentColorBlendMethod(const std::string &trackID, const std::string &segmentID, SequenceTrackSegmentColor::EBlendMethod blendMethod)
    {
        performEditAction([this, trackID, segmentID, blendMethod]()
                          {
                              auto *segment = findSegment(trackID, segmentID);
                              assert(segment != nullptr); // segment not found
                              assert(segment->get_type().is_derived_from(RTTI_OF(SequenceTrackSegmentColor))); // type mismatch

                              auto &segment_color = static_cast<SequenceTrackSegmentColor &>(*segment);
                              segment_color.mBlendMethod = blendMethod;
                          });
    }


    const SequenceTrackSegment* SequenceControllerColor::insertColorSegment(const std::string &trackID, double time, const nap::RGBAColorFloat &color)
    {
        SequenceTrackSegment* return_ptr = nullptr;
        performEditAction([this, return_ptr , trackID, time, color]() mutable
                          {
                              // create new segment & set parameters
                              std::unique_ptr<SequenceTrackSegmentColor> new_segment = std::make_unique<SequenceTrackSegmentColor>();
                              new_segment->mStartTime = time;
                              new_segment->mID = mService.generateUniqueID(getPlayerReadObjectIDs());
                              new_segment->mColor = color;

                              // find track
                              SequenceTrack *track = findTrack(trackID);
                              assert(track != nullptr); // track not found

                              // add segment to track
                              track->mSegments.emplace_back(ResourcePtr<SequenceTrackSegmentColor>(new_segment.get()));

                              return_ptr = new_segment.get();

                              // move ownership
                              getPlayerOwnedObjects().emplace_back(std::move(new_segment));

                              // update tracks
                              updateTracks();
                          });

        return return_ptr;
    }


    void SequenceControllerColor::addNewColorTrack()
    {
        performEditAction([this]()
                          {
                              // create sequence track
                              std::unique_ptr<SequenceTrackColor> sequence_track = std::make_unique<SequenceTrackColor>();
                              sequence_track->mID = mService.generateUniqueID(getPlayerReadObjectIDs());

                              // name is id
                              sequence_track->mName = sequence_track->mID;

                              // create resource ptr
                              getSequence().mTracks.emplace_back(ResourcePtr<SequenceTrackColor>(sequence_track.get()));

                              // move ownership of unique ptrs
                              getPlayerOwnedObjects().emplace_back(std::move(sequence_track));
                          });
    }


    void SequenceControllerColor::insertTrack(rttr::type type)
    {
        addNewColorTrack();
    }
}
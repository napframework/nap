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

                              // create curve
                              std::unique_ptr<math::FCurve<float, float>> fcurve = std::make_unique<math::FCurve<float, float>>();
                              fcurve->mPoints[1].mInTan.mTime = -0.4f;
                              fcurve->mPoints[1].mOutTan.mTime = 0.4f;
                              fcurve->mID = mService.generateUniqueID(getPlayerReadObjectIDs());

                              // set curve
                              new_segment->mCurve = ResourcePtr<math::FCurve<float, float>>(fcurve.get());
                              for(auto &point: fcurve->mPoints)
                              {
                                  point.mInterp = new_segment->mCurveType;
                              }
                              new_segment->mCurve->invalidate();

                              // move ownership
                              getPlayerOwnedObjects().emplace_back(std::move(fcurve));

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


    void SequenceControllerColor::changeSegmentColorBlendMethod(const std::string &trackID, const std::string &segmentID, SequenceTrackSegmentColor::EColorSpace blendMethod)
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

                              // create curve
                              std::unique_ptr<math::FCurve<float, float>> fcurve = std::make_unique<math::FCurve<float, float>>();
                              fcurve->mPoints[1].mInTan.mTime = -0.4f;
                              fcurve->mPoints[1].mOutTan.mTime = 0.4f;
                              fcurve->mID = mService.generateUniqueID(getPlayerReadObjectIDs());

                              // set curve
                              new_segment->mCurve = ResourcePtr<math::FCurve<float, float>>(fcurve.get());

                              // set curve interp
                              new_segment->mCurve = ResourcePtr<math::FCurve<float, float>>(fcurve.get());
                              for(auto &point: fcurve->mPoints)
                              {
                                  point.mInterp = new_segment->mCurveType;
                              }
                              new_segment->mCurve->invalidate();

                              // move ownership
                              getPlayerOwnedObjects().emplace_back(std::move(fcurve));

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


    void SequenceControllerColor::changeSegmentCurvePoint(const std::string &trackID, const std::string &segmentID, int pointIndex, const glm::vec2 &value)
    {
        performEditAction([this, trackID, segmentID, pointIndex, value]()
                          {
                              auto *segment = findSegment(trackID, segmentID);
                              assert(segment != nullptr); // segment not found
                              assert(segment->get_type().is_derived_from(RTTI_OF(SequenceTrackSegmentColor))); // type mismatch

                              auto &segment_color = static_cast<SequenceTrackSegmentColor &>(*segment);

                              // iterate trough points of curve
                              for(int i = 0; i < segment_color.mCurve->mPoints.size(); i++)
                              {
                                    // find the point the new point needs to get inserted after
                                    if(i == pointIndex)
                                    {
                                        // create point
                                        math::FCurvePoint<float, float> p = segment_color.mCurve->mPoints[i];
                                        p.mPos.mValue = value.y;
                                        p.mPos.mTime = value.x;
                                        p.mPos.mValue = glm::clamp(p.mPos.mValue, 0.0f, 1.0f);
                                        p.mPos.mTime = glm::clamp(p.mPos.mTime, 0.0f, 1.0f);

                                        // insert point
                                        segment_color.mCurve->mPoints[i] = p;
                                        segment_color.mCurve->invalidate();

                                        break;
                                    }
                              }

                              segment_color.mCurve->invalidate();
                          });
    }


    void SequenceControllerColor::insertCurvePoint(const std::string &trackID, const std::string &segmentID, float pos)
    {
        performEditAction([this, trackID, segmentID, pos]()
                          {
                              auto *segment = findSegment(trackID, segmentID);
                              assert(segment != nullptr); // segment not found
                              assert(segment->get_type().is_derived_from(RTTI_OF(SequenceTrackSegmentColor))); // type mismatch

                              auto &segment_color = static_cast<SequenceTrackSegmentColor &>(*segment);

                              // iterate trough points of curve
                              for(int i = 0; i < segment_color.mCurve->mPoints.size() - 1; i++)
                              {
                                  // find the point the new point needs to get inserted after
                                  if(segment_color.mCurve->mPoints[i].mPos.mTime <= pos &&
                                     segment_color.mCurve->mPoints[i + 1].mPos.mTime > pos)
                                  {
                                      // create point
                                      math::FCurvePoint<float, float> p;
                                      p.mPos.mTime = pos;
                                      p.mPos.mValue = segment_color.mCurve->evaluate(pos);
                                      p.mInTan.mTime = -0.1f;
                                      p.mOutTan.mTime = 0.1f;
                                      p.mInTan.mValue = 0.0f;
                                      p.mOutTan.mValue = 0.0f;
                                      p.mTangentsAligned = true;
                                      p.mInterp = segment_color.mCurveType;

                                      // insert point
                                      segment_color.mCurve->mPoints.insert(
                                              segment_color.mCurve->mPoints.begin() + i + 1, p);
                                      segment_color.mCurve->invalidate();

                                      break;
                                  }
                              }
                          });
    }


    void SequenceControllerColor::changeSegmentCurveType(const std::string &trackID, const std::string &segmentID, math::ECurveInterp curveType)
    {
        performEditAction([this, trackID, segmentID, curveType]()
                          {
                              auto *segment = findSegment(trackID, segmentID);
                              assert(segment != nullptr); // segment not found
                              assert(segment->get_type().is_derived_from(RTTI_OF(SequenceTrackSegmentColor))); // type mismatch

                              auto &segment_color = static_cast<SequenceTrackSegmentColor &>(*segment);
                              segment_color.mCurveType = curveType;

                              for(auto &point: segment_color.mCurve->mPoints)
                              {
                                  point.mInterp = curveType;
                              }

                              segment_color.mCurve->invalidate();
                          });
    }


    void SequenceControllerColor::changeSegmentCurveTanPoint(const std::string &trackID, const std::string &segmentID, int pointIndex, int tanIndex, const glm::vec2 &value)
    {
        performEditAction([this, trackID, segmentID, pointIndex, tanIndex, value]()
                          {
                              auto *segment = findSegment(trackID, segmentID);
                              assert(segment != nullptr); // segment not found
                              assert(segment->get_type().is_derived_from(RTTI_OF(SequenceTrackSegmentColor))); // type mismatch

                              auto &segment_color = static_cast<SequenceTrackSegmentColor &>(*segment);

                              // iterate trough points of curve
                              for(int i = 0; i < segment_color.mCurve->mPoints.size(); i++)
                              {
                                  // find the point the new point needs to get inserted after
                                  if(i == pointIndex)
                                  {
                                      // create point
                                      math::FCurvePoint<float, float> p = segment_color.mCurve->mPoints[i];
                                      if(tanIndex == 0)
                                      {
                                          p.mInTan.mTime = value.x;
                                          p.mInTan.mValue = value.y;
                                          p.mOutTan.mTime = -value.x;
                                          p.mOutTan.mValue = -value.y;
                                      }
                                      else
                                      {
                                          p.mOutTan.mTime = value.x;
                                          p.mOutTan.mValue = value.y;
                                          p.mInTan.mTime = -value.x;
                                          p.mInTan.mValue = -value.y;
                                      }

                                      p.mTangentsAligned = true;

                                      // insert point
                                      segment_color.mCurve->mPoints[i] = p;
                                      segment_color.mCurve->invalidate();

                                      break;
                                  }
                              }
                          });
    }


    void SequenceControllerColor::deleteCurvePoint(const std::string &trackID, const std::string &segmentID, int pointIndex)
    {
        performEditAction([this, trackID, segmentID, pointIndex]()
                          {
                              auto *segment = findSegment(trackID, segmentID);
                              assert(segment != nullptr); // segment not found
                              assert(segment->get_type().is_derived_from(RTTI_OF(SequenceTrackSegmentColor))); // type mismatch

                              auto &segment_color = static_cast<SequenceTrackSegmentColor &>(*segment);

                              // iterate trough points of curve
                              for(int i = 0; i < segment_color.mCurve->mPoints.size(); i++)
                              {
                                  // find the point the new point needs to get inserted after
                                  if(i == pointIndex)
                                  {
                                      // erase point
                                      segment_color.mCurve->mPoints.erase(segment_color.mCurve->mPoints.begin() + i);
                                      segment_color.mCurve->invalidate();

                                      break;
                                  }
                              }
                          });
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
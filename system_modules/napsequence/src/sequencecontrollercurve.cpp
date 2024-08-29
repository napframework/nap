/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "sequencecontrollercurve.h"
#include "sequenceeditor.h"
#include "sequencetracksegmentcurve.h"

namespace nap
{
    SequenceControllerCurve::SequenceControllerCurve(SequenceService& service, SequencePlayer& player, SequenceEditor& editor)
        : SequenceController(service, player, editor)
    {
        mUpdateSegmentFunctionMap = {{RTTI_OF(SequenceTrackCurveFloat), [this](SequenceTrack &track)
                                                                        {
                                                                            this->updateCurveSegments<float>(track);
                                                                        }},
                                     {RTTI_OF(SequenceTrackCurveVec2),  [this](SequenceTrack &track)
                                                                        {
                                                                            this->updateCurveSegments<glm::vec2>(track);
                                                                        }},
                                     {RTTI_OF(SequenceTrackCurveVec3),  [this](SequenceTrack &track)
                                                                        {
                                                                            this->updateCurveSegments<glm::vec3>(track);
                                                                        }},
                                     {RTTI_OF(SequenceTrackCurveVec4),  [this](SequenceTrack &track)
                                                                        {
                                                                            this->updateCurveSegments<glm::vec4>(track);
                                                                        }}};

        mInsertSegmentFunctionMap = {{RTTI_OF(SequenceTrackCurveFloat), [this](const std::string &trackID, double time) -> const SequenceTrackSegment *
                                                                        {
                                                                            return this->insertCurveSegment<float>(trackID, time);
                                                                        }},
                                     {RTTI_OF(SequenceTrackCurveVec2),  [this](const std::string &trackID, double time) -> const SequenceTrackSegment *
                                                                        {
                                                                            return this->insertCurveSegment<glm::vec2>(trackID, time);
                                                                        }},
                                     {RTTI_OF(SequenceTrackCurveVec3),  [this](const std::string &trackID, double time) -> const SequenceTrackSegment *
                                                                        {
                                                                            return this->insertCurveSegment<glm::vec3>(trackID, time);
                                                                        }},
                                     {RTTI_OF(SequenceTrackCurveVec4),  [this](const std::string &trackID, double time) -> const SequenceTrackSegment *
                                                                        {
                                                                            return this->insertCurveSegment<glm::vec4>(trackID, time);
                                                                        }},};
        mInsertTrackFunctionMap =
            {{
                 RTTI_OF(SequenceTrackCurveFloat), [](SequenceControllerCurve *controller)
                                                   {
                                                       controller->addNewCurveTrack<float>();
                                                   }},
             {
                 RTTI_OF(SequenceTrackCurveVec2),  [](SequenceControllerCurve *controller)
                                                   {
                                                       controller->addNewCurveTrack<glm::vec2>();
                                                   }},
             {
                 RTTI_OF(SequenceTrackCurveVec3),  [](SequenceControllerCurve *controller)
                                                   {
                                                       controller->addNewCurveTrack<glm::vec3>();
                                                   }},
             {
                 RTTI_OF(SequenceTrackCurveVec4),  [](SequenceControllerCurve *controller)
                                                   {
                                                       controller->addNewCurveTrack<glm::vec4>();
                                                   }
             }};

        mChangeTanPointFunctionMap =
            {
                {RTTI_OF(SequenceTrackSegmentCurveFloat),
                    [this](SequenceTrackSegment &segment, const std::string &trackID, const int pointIndex, const int curveIndex,
                           sequencecurveenums::ETanPointTypes tanPointType, float time, float value) -> bool
                    {
                        return changeTanPoint<float>(segment, trackID, pointIndex, curveIndex, tanPointType, time, value);
                    }},
                {RTTI_OF(SequenceTrackSegmentCurveVec2),
                    [this](SequenceTrackSegment &segment, const std::string &trackID, const int pointIndex, const int curveIndex,
                           sequencecurveenums::ETanPointTypes tanPointType, float time, float value) -> bool
                    {
                        return changeTanPoint<glm::vec2>(segment, trackID, pointIndex, curveIndex, tanPointType, time, value);
                    }},
                {RTTI_OF(SequenceTrackSegmentCurveVec3),
                    [this](SequenceTrackSegment &segment, const std::string &trackID, const int pointIndex, const int curveIndex,
                           sequencecurveenums::ETanPointTypes tanPointType, float time, float value) -> bool
                    {
                        return changeTanPoint<glm::vec3>(segment, trackID, pointIndex, curveIndex, tanPointType, time, value);
                    }},
                {RTTI_OF(SequenceTrackSegmentCurveVec4),
                    [this](SequenceTrackSegment &segment, const std::string &trackID, const int pointIndex, const int curveIndex,
                           sequencecurveenums::ETanPointTypes tanPointType, float time, float value) -> bool
                    {
                        return changeTanPoint<glm::vec4>(segment, trackID, pointIndex, curveIndex, tanPointType, time, value);
                    }},
            };

        mChangeLastCurvePointFunctionMap =
            {
                {RTTI_OF(SequenceTrackSegmentCurveFloat),
                    [this](SequenceTrackSegment &segment, const int curveIndex, float time, float value)
                    {
                        changeLastCurvePoint<float>(segment, curveIndex, time, value);
                    }
                },
                {RTTI_OF(SequenceTrackSegmentCurveVec2),
                    [this](SequenceTrackSegment &segment, const int curveIndex, float time, float value)
                    {
                        changeLastCurvePoint<glm::vec2>(segment, curveIndex, time, value);
                    }
                },
                {RTTI_OF(SequenceTrackSegmentCurveVec3),
                    [this](SequenceTrackSegment &segment, const int curveIndex, float time, float value)
                    {
                        changeLastCurvePoint<glm::vec3>(segment, curveIndex, time, value);
                    }
                },
                {RTTI_OF(SequenceTrackSegmentCurveVec4),
                    [this](SequenceTrackSegment &segment, const int curveIndex, float time, float value)
                    {
                        changeLastCurvePoint<glm::vec4>(segment, curveIndex, time, value);
                    }
                }
            };

        mInsertCurvePointFunctionMap =
            {
                {RTTI_OF(SequenceTrackSegmentCurveFloat), [this](SequenceTrackSegment &segment, float time, int curveIndex)
                                                          {
                                                              insertCurvePoint<float>(segment, time, curveIndex);
                                                          }},
                {RTTI_OF(SequenceTrackSegmentCurveVec2),  [this](SequenceTrackSegment &segment, float time, int curveIndex)
                                                          {
                                                              insertCurvePoint<glm::vec2>(segment, time, curveIndex);
                                                          }},
                {RTTI_OF(SequenceTrackSegmentCurveVec3),  [this](SequenceTrackSegment &segment, float time, int curveIndex)
                                                          {
                                                              insertCurvePoint<glm::vec3>(segment, time, curveIndex);
                                                          }},
                {RTTI_OF(SequenceTrackSegmentCurveVec4),  [this](SequenceTrackSegment &segment, float time, int curveIndex)
                                                          {
                                                              insertCurvePoint<glm::vec4>(segment, time, curveIndex);
                                                          }},
            };

        mChangeSegmentValueFunctionMap =
            {
                {RTTI_OF(SequenceTrackSegmentCurveFloat), [this](SequenceTrack &track, SequenceTrackSegment &segment, float value, int curveIndex, sequencecurveenums::ESegmentValueTypes valueType)
                                                          {
                                                              changeCurveSegmentValue<float>(track, segment, value, curveIndex, valueType);
                                                          }},
                {RTTI_OF(SequenceTrackSegmentCurveVec2),  [this](SequenceTrack &track, SequenceTrackSegment &segment, float value, int curveIndex, sequencecurveenums::ESegmentValueTypes valueType)
                                                          {
                                                              changeCurveSegmentValue<glm::vec2>(track, segment, value, curveIndex, valueType);
                                                          }},
                {RTTI_OF(SequenceTrackSegmentCurveVec3),  [this](SequenceTrack &track, SequenceTrackSegment &segment, float value, int curveIndex, sequencecurveenums::ESegmentValueTypes valueType)
                                                          {
                                                              changeCurveSegmentValue<glm::vec3>(track, segment, value, curveIndex, valueType);
                                                          }},
                {RTTI_OF(SequenceTrackSegmentCurveVec4),  [this](SequenceTrack &track, SequenceTrackSegment &segment, float value, int curveIndex, sequencecurveenums::ESegmentValueTypes valueType)
                                                          {
                                                              changeCurveSegmentValue<glm::vec4>(track, segment, value, curveIndex, valueType);
                                                          }},
            };

        mChangeCurveTypeFunctionMap =
            {
                {RTTI_OF(SequenceTrackSegmentCurveFloat), [this](SequenceTrackSegment &segment, math::ECurveInterp type, int curveIndex)
                                                          {
                                                              changeCurveType<float>(segment, type, curveIndex);
                                                          }},
                {RTTI_OF(SequenceTrackSegmentCurveVec2),  [this](SequenceTrackSegment &segment, math::ECurveInterp type, int curveIndex)
                                                          {
                                                              changeCurveType<glm::vec2>(segment, type, curveIndex);
                                                          }},
                {RTTI_OF(SequenceTrackSegmentCurveVec3),  [this](SequenceTrackSegment &segment, math::ECurveInterp type, int curveIndex)
                                                          {
                                                              changeCurveType<glm::vec3>(segment, type, curveIndex);
                                                          }},
                {RTTI_OF(SequenceTrackSegmentCurveVec4),  [this](SequenceTrackSegment &segment, math::ECurveInterp type, int curveIndex)
                                                          {
                                                              changeCurveType<glm::vec4>(segment, type, curveIndex);
                                                          }}
            };

        mChangeCurvePointFunctionMap =
            {
                {RTTI_OF(SequenceTrackSegmentCurveFloat), [this](SequenceTrackSegment &segment, const int pointIndex, const int curveIndex, float time, float value)
                                                          {
                                                              changeCurvePoint<float>(segment, pointIndex, curveIndex, time, value);
                                                          }},
                {RTTI_OF(SequenceTrackSegmentCurveVec2),  [this](SequenceTrackSegment &segment, const int pointIndex, const int curveIndex, float time, float value)
                                                          {
                                                              changeCurvePoint<glm::vec2>(segment, pointIndex, curveIndex, time, value);
                                                          }},
                {RTTI_OF(SequenceTrackSegmentCurveVec3),  [this](SequenceTrackSegment &segment, const int pointIndex, const int curveIndex, float time, float value)
                                                          {
                                                              changeCurvePoint<glm::vec3>(segment, pointIndex, curveIndex, time, value);
                                                          }},
                {RTTI_OF(SequenceTrackSegmentCurveVec4),  [this](SequenceTrackSegment &segment, const int pointIndex, const int curveIndex, float time, float value)
                                                          {
                                                              changeCurvePoint<glm::vec4>(segment, pointIndex, curveIndex, time, value);
                                                          }}
            };

        mDeleteCurvePointFunctionMap =
            {
                {RTTI_OF(SequenceTrackSegmentCurveFloat), [this](SequenceTrackSegment &segment, const int pointIndex, const int curveIndex)
                                                          {
                                                              deleteCurvePoint<float>(segment, pointIndex, curveIndex);
                                                          }},
                {RTTI_OF(SequenceTrackSegmentCurveVec2),  [this](SequenceTrackSegment &segment, const int pointIndex, const int curveIndex)
                                                          {
                                                              deleteCurvePoint<glm::vec2>(segment, pointIndex, curveIndex);
                                                          }},
                {RTTI_OF(SequenceTrackSegmentCurveVec3),  [this](SequenceTrackSegment &segment, const int pointIndex, const int curveIndex)
                                                          {
                                                              deleteCurvePoint<glm::vec3>(segment, pointIndex, curveIndex);
                                                          }},
                {RTTI_OF(SequenceTrackSegmentCurveVec4),  [this](SequenceTrackSegment &segment, const int pointIndex, const int curveIndex)
                                                          {
                                                              deleteCurvePoint<glm::vec4>(segment, pointIndex, curveIndex);
                                                          }}
            };
    }


    double SequenceControllerCurve::segmentDurationChange(const std::string& trackID, const std::string& segmentID, float duration, bool adjustFollowingSegments)
    {
        double return_duration = duration;

        performEditAction([this, trackID, segmentID, duration, &return_duration, adjustFollowingSegments]()
                          {
                              // Minimal duration of a segment
                              // This value is used to check if the new duration is valid
                              static constexpr float minimal_duration = 0.05f;

                              SequenceTrack *track = findTrack(trackID);
                              assert(track != nullptr); // track not found

                              // declare variables for previous, next and current segment
                              SequenceTrackSegmentCurveBase* next_segment = nullptr;
                              SequenceTrackSegmentCurveBase* track_segment = nullptr;

                              // find the segment
                              for(int i = 0 ; i < track->mSegments.size(); i++)
                              {
                                  if(track->mSegments[i]->mID == segmentID)
                                  {
                                      // get the previous, next and current segment
                                      track_segment = static_cast<SequenceTrackSegmentCurveBase*>(track->mSegments[i].get());
                                      if(i+1 < track->mSegments.size())
                                          next_segment = static_cast<SequenceTrackSegmentCurveBase*>(track->mSegments[i+1].get());

                                      // check if new duration is valid
                                      bool valid = true;
                                      double new_duration = duration;
                                      double difference = new_duration - track_segment->mDuration;

                                      if(new_duration > minimal_duration)
                                      {
                                          // iterate through the rest of the segments and check if the new duration is valid
                                          // if a segment is locked it can happen that the new duration is not valid because its
                                          // previous segment start time + duration cannot exceed the start time of the next locked segment
                                          for(int j = i+1; j < track->mSegments.size(); j++)
                                          {
                                              auto* previous_segment_local = static_cast<SequenceTrackSegmentCurveBase*>(track->mSegments[j - 1].get());
                                              auto* track_segment_local = static_cast<SequenceTrackSegmentCurveBase*>(track->mSegments[j].get());
                                              if(track_segment_local->mLocked || !adjustFollowingSegments)
                                              {
                                                  if(previous_segment_local->mStartTime + previous_segment_local->mDuration + difference >
                                                     track_segment_local->mStartTime + track_segment_local->mDuration - minimal_duration)
                                                  {
                                                      valid = false;
                                                      break;
                                                  }
                                              }
                                          }
                                      }else
                                      {
                                          valid = false;
                                      }

                                      if(valid)
                                      {
                                          track_segment->mDuration = duration;

                                          // if the drag is locked, adjust only start time of next segment to match the end time of the current segment
                                          // also, iterate through the rest of the segments and adjust their start times and durations if one is locked
                                          if(next_segment)
                                          {
                                              for(int j = i+1; j < track->mSegments.size(); j++)
                                              {
                                                  auto* previous_segment_local = static_cast<SequenceTrackSegmentCurveBase*>(track->mSegments[j - 1].get());
                                                  auto* track_segment_local = static_cast<SequenceTrackSegmentCurveBase*>(track->mSegments[j].get());
                                                  if(track_segment_local->mLocked || !adjustFollowingSegments)
                                                  {
                                                      double new_start_time = previous_segment_local->mStartTime + previous_segment_local->mDuration;
                                                      double difference = track_segment_local->mStartTime - new_start_time;
                                                      track_segment_local->mStartTime = new_start_time;
                                                      track_segment_local->mDuration += difference;
                                                      break;
                                                  }else
                                                  {
                                                      track_segment_local->mStartTime = previous_segment_local->mStartTime + previous_segment_local->mDuration;
                                                  }
                                              }
                                          }

                                          updateTracks();
                                      }

                                      return_duration = track_segment->mDuration;
                                      break;
                                  }
                              }
                          });

        return return_duration;
    }


    const SequenceTrackSegment* SequenceControllerCurve::insertSegment(const std::string& trackID, double time)
    {
        auto *track = findTrack(trackID);
        assert(track != nullptr); // track not found

        auto it = mInsertSegmentFunctionMap.find(track->get_type());
        assert(it != mInsertSegmentFunctionMap.end()); // type not found
        return it->second(trackID, time);

        return nullptr;
    }


    void SequenceControllerCurve::setSegmentLocked(const std::string& trackID, const std::string& segmentID, bool locked)
    {
        performEditAction([this, trackID, segmentID, locked]()
                          {
                              SequenceTrack *track = findTrack(trackID);
                              assert(track != nullptr); // track not found

                              auto *segment = static_cast<SequenceTrackSegmentCurveBase *>(findSegment(trackID, segmentID));
                              assert(segment != nullptr); // segment not found

                              segment->mLocked = locked;
                          });
    }


    void SequenceControllerCurve::changeSegmentColor(const std::string &trackID, const std::string &segmentID, const nap::RGBAColorFloat &newColor)
    {
        performEditAction([this, trackID, segmentID, newColor]()
                          {
                              auto *segment = findSegment(trackID, segmentID);
                              assert(segment!=nullptr); // segment not found

                              auto *curve_segment = static_cast<SequenceTrackSegmentCurveBase*>(segment);
                              assert(curve_segment!= nullptr); // cannot upcast to curve segment base

                              curve_segment->mColor = newColor;
                          });
    }


    void SequenceControllerCurve::deleteSegment(const std::string& trackID, const std::string& segmentID)
    {
        // pause player thread
        performEditAction([this, trackID, segmentID]()
                          {
                              Sequence& sequence = getSequence();
                              auto* track = findTrack(trackID);

                              if(track!= nullptr)
                              {
                                  int segment_index = 0;
                                  for(auto &segment: track->mSegments)
                                  {
                                      if(segment->mID == segmentID)
                                      {
                                          // upcast to duration segment
                                          const auto* duration_segment = static_cast<SequenceTrackSegmentDuration*>(segment.get());

                                          // store the duration of the segment that we are deleting
                                          double duration = duration_segment->mDuration;

                                          // erase it from the list
                                          track->mSegments.erase(track->mSegments.begin() + segment_index);

                                          // get the segment that is now at the previous deleted segments position
                                          if(track->mSegments.begin() + segment_index != track->mSegments.end())
                                          {
                                              // add the duration
                                              static_cast<SequenceTrackSegmentDuration*>(track->mSegments[segment_index].get())->mDuration += duration;
                                          }

                                          deleteObjectFromSequencePlayer(segmentID);

                                          // update segments
                                          auto it = mUpdateSegmentFunctionMap.find(track->get_type());
                                          assert (it != mUpdateSegmentFunctionMap.end());
                                          it->second(*track);

                                          break;
                                      }

                                      updateTracks();
                                      segment_index++;
                                  }
                              }
                          });
    }


    void SequenceControllerCurve::changeCurveType(const std::string& trackID, const std::string& segmentID,
                                                  math::ECurveInterp type, int curveIndex)
    {
        performEditAction([this, trackID, segmentID, type, curveIndex]()
                          {
                              auto *segment = findSegment(trackID, segmentID);
                              assert(segment != nullptr); // segment not found

                              auto it = mChangeCurveTypeFunctionMap.find(segment->get_type());
                              assert(it != mChangeCurveTypeFunctionMap.end()); // type not found
                              if(it != mChangeCurveTypeFunctionMap.end())
                              {
                                  it->second(*segment, type, curveIndex);
                              }
                          });
    }


    void SequenceControllerCurve::changeCurveSegmentValue(const std::string& trackID, const std::string& segmentID,
                                                          float newValue, int curveIndex,
                                                          sequencecurveenums::ESegmentValueTypes valueType)
    {
        performEditAction([this, trackID, segmentID, newValue, curveIndex, valueType]()
                          {
                              SequenceTrack *track = findTrack(trackID);
                              assert(track != nullptr); // track not found

                              SequenceTrackSegment *segment = findSegment(trackID, segmentID);
                              assert(segment != nullptr); // segment not found

                              if(segment != nullptr)
                              {
                                  auto it = mChangeSegmentValueFunctionMap.find(segment->get_type());
                                  assert(it != mChangeSegmentValueFunctionMap.end()); // type not found
                                  if(it != mChangeSegmentValueFunctionMap.end())
                                  {
                                      it->second(*track, *segment, newValue, curveIndex, valueType);
                                  }
                              }
                          });
    }


    void SequenceControllerCurve::insertCurvePoint(const std::string& trackID, const std::string& segmentID, float pos,
                                                   int curveIndex)
    {
        performEditAction([this, trackID, segmentID, pos, curveIndex]()
                          {
                              // find segment
                              SequenceTrackSegment *segment = findSegment(trackID, segmentID);
                              assert(segment != nullptr); // segment not found

                              auto it = mInsertCurvePointFunctionMap.find(segment->get_type());
                              assert(it != mInsertCurvePointFunctionMap.end()); // type not found
                              if(it != mInsertCurvePointFunctionMap.end())
                              {
                                  it->second(*segment, pos, curveIndex);
                              }
                          });
    }


    void SequenceControllerCurve::deleteCurvePoint(const std::string& trackID, const std::string& segmentID, const int index, int curveIndex)
    {
        performEditAction([this, trackID, segmentID, index, curveIndex]()
                          {
                              // find segment
                              SequenceTrackSegment *segment = findSegment(trackID, segmentID);
                              assert(segment != nullptr); // segment not found

                              auto it = mDeleteCurvePointFunctionMap.find(segment->get_type());
                              assert(it != mDeleteCurvePointFunctionMap.end()); // type not found
                              if(it != mDeleteCurvePointFunctionMap.end())
                              {
                                  it->second(*segment, index, curveIndex);
                              }
                          });
    }


    void SequenceControllerCurve::changeCurvePoint(const std::string& trackID, const std::string& segmentID,
                                                   const int pointIndex, const int curveIndex, float time, float value)
    {
        performEditAction([this, trackID, segmentID, pointIndex, curveIndex, time, value]()
                          {
                              // find segment
                              SequenceTrackSegment *segment = findSegment(trackID, segmentID);
                              assert(segment != nullptr); // segment not found

                              auto it = mChangeCurvePointFunctionMap.find(segment->get_type());
                              assert(it != mChangeCurvePointFunctionMap.end()); // type not found
                              if(it != mChangeCurvePointFunctionMap.end())
                              {
                                  it->second(*segment, pointIndex, curveIndex, time, value);

                                  // if point index = 0, check if we need to update end point of any previous segment
                                  if(pointIndex == 0)
                                  {
                                      // find corresponding track
                                      auto *track = findTrack(trackID);

                                      // iterate trough segments
                                      for(size_t i = 0; i < track->mSegments.size(); i++)
                                      {
                                          // if this segment is found, and index is bigger then 0 obtain the previous segment
                                          if(track->mSegments[i].get() == segment && i > 0)
                                          {
                                              // obtain previous segment
                                              auto *prev_segment = track->mSegments[i - 1].get();

                                              // find corresponding function call and invoke changeLastCurvePoint on the previous segment
                                              auto it2 = mChangeLastCurvePointFunctionMap.find(segment->get_type());
                                              assert(it2 != mChangeLastCurvePointFunctionMap.end()); // type not found
                                              if(it2 != mChangeLastCurvePointFunctionMap.end())
                                              {
                                                  it2->second(*prev_segment, curveIndex, 1.0f, value);
                                              }

                                              break;
                                          }
                                      }
                                  }
                              }
                          });
    }


    bool SequenceControllerCurve::changeTanPoint(const std::string& trackID, const std::string& segmentID,
                                                 const int pointIndex, const int curveIndex,
                                                 sequencecurveenums::ETanPointTypes tanType, float time, float value)
    {
        bool tangents_flipped = false;
        performEditAction([this, trackID, segmentID, pointIndex, curveIndex, tanType, time, value, &tangents_flipped]()
                          {
                              // find segment
                              SequenceTrackSegment *segment = findSegment(trackID, segmentID);
                              assert(segment != nullptr); // segment not found

                              auto it = mChangeTanPointFunctionMap.find(segment->get_type());
                              assert(it != mChangeTanPointFunctionMap.end()); // type not found
                              if(it != mChangeTanPointFunctionMap.end())
                              {
                                  // call appropriate function
                                  tangents_flipped = it->second(*segment, trackID, pointIndex, curveIndex, tanType, time, value);
                              }
                          });

        return tangents_flipped;
    }


    void SequenceControllerCurve::insertTrack(rtti::TypeInfo type)
    {
        assert(mInsertTrackFunctionMap.find(type) != mInsertTrackFunctionMap.end()); // function not found
        auto &insert_track_function = mInsertTrackFunctionMap[type];
        insert_track_function(this);
    }


    void SequenceControllerCurve::updateCurveSegments(const std::string& trackID)
    {
        auto *track = findTrack(trackID);
        auto it = mUpdateSegmentFunctionMap.find(track->get_type());
        assert(it != mUpdateSegmentFunctionMap.end());
        it->second(*track);
    }


    template<>
    void SequenceControllerCurve::changeMinMaxCurveTrack<float>(const std::string& trackID, float minimum, float maximum)
    {
        performEditAction([this, trackID, minimum, maximum]()
                          {
                              SequenceTrack *track = findTrack(trackID);
                              assert(track != nullptr); // track not found

                              auto *track_curve = static_cast<SequenceTrackCurveFloat *>(track);

                              for(auto &segment: track_curve->mSegments)
                              {
                                  auto &curve_segment = *static_cast<SequenceTrackSegmentCurveFloat *>(segment.get());
                                  int curve_count = 0;
                                  for(auto &curve: curve_segment.mCurves)
                                  {
                                      for(auto &point: curve->mPoints)
                                      {
                                          if(minimum!=maximum)
                                          {
                                              float value = point.mPos.mValue * (track_curve->mMaximum - track_curve->mMinimum) +
																				track_curve->mMinimum;

                                              point.mPos.mValue = (value - minimum) / (maximum - minimum);
                                              point.mPos.mValue = math::clamp<float>(point.mPos.mValue, 0, 1);
                                          }else
                                          {
                                              point.mPos.mValue = 0.0f;
                                          }
                                      }
                                      curve_count++;
                                  }
                              }

                              track_curve->mMinimum = minimum;
                              track_curve->mMaximum = maximum;
                          });
    }
}

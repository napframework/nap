/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "sequencecontrollercurve.h"
#include "sequenceutils.h"
#include "sequencetrackcurve.h"
#include "sequenceeditor.h"
#include "sequencetracksegmentcurve.h"

// Nap Includes
#include <mathutils.h>

namespace nap
{
	static bool sRegistered = SequenceController::registerControllerFactory(RTTI_OF(SequenceControllerCurve), [](SequencePlayer& player, SequenceEditor& editor)->std::unique_ptr<SequenceController> 
	{ 
		return std::make_unique<SequenceControllerCurve>(player, editor); 
	});


	static bool sRegisterControllerTypes[4]
	{
		SequenceEditor::registerControllerForTrackType(RTTI_OF(SequenceTrackCurveFloat), RTTI_OF(SequenceControllerCurve)),
		SequenceEditor::registerControllerForTrackType(RTTI_OF(SequenceTrackCurveVec2), RTTI_OF(SequenceControllerCurve)),
		SequenceEditor::registerControllerForTrackType(RTTI_OF(SequenceTrackCurveVec3), RTTI_OF(SequenceControllerCurve)),
		SequenceEditor::registerControllerForTrackType(RTTI_OF(SequenceTrackCurveVec4), RTTI_OF(SequenceControllerCurve))
	};


	std::unordered_map<rttr::type, void(SequenceControllerCurve::*)(SequenceTrack&)> SequenceControllerCurve::sUpdateSegmentFunctionMap
	{
		{ RTTI_OF(SequenceTrackCurveFloat), &SequenceControllerCurve::updateCurveSegments<float> },
		{ RTTI_OF(SequenceTrackCurveVec2), &SequenceControllerCurve::updateCurveSegments<glm::vec2> },
		{ RTTI_OF(SequenceTrackCurveVec3), &SequenceControllerCurve::updateCurveSegments<glm::vec3> },
		{ RTTI_OF(SequenceTrackCurveVec4), &SequenceControllerCurve::updateCurveSegments<glm::vec4> }
	};


	double SequenceControllerCurve::segmentDurationChange(const std::string& trackID, const std::string& segmentID, float duration)
	{
		double return_duration = duration;

		performEditAction([this, trackID, segmentID, duration, &return_duration]()
		{
			//
			SequenceTrack* track = findTrack(trackID);
			assert(track != nullptr); // track not found

			if (track != nullptr)
			{
			  ResourcePtr<SequenceTrackSegment> previous_segment = nullptr;
			  for (auto track_segment : track->mSegments)
			  {
				  if (track_segment->mID == segmentID)
				  {
					  // check if new duration is valid
					  bool valid = true;
					  double new_duration = duration;

					  if (new_duration > 0.0)
					  {
						  if (previous_segment != nullptr)
						  {
							  if (track_segment->mStartTime + new_duration < previous_segment->mStartTime + previous_segment->mDuration)
							  {
								  valid = false;
							  }
						  }
					  }
					  else
					  {
						  valid = false;
					  }

					  if (valid)
					  {
						  track_segment->mDuration = duration;

						  auto it = sUpdateSegmentFunctionMap.find(track->get_type());
						  if (it != sUpdateSegmentFunctionMap.end())
						  {
							  (*this.*it->second)(*track);
						  }

						  updateTracks();
					  }

					  return_duration = track_segment->mDuration;
					  break;
				  }

				  previous_segment = track_segment;
			  }
			}
		});

		return return_duration;
	}


	const SequenceTrackSegment* SequenceControllerCurve::insertSegment(const std::string& trackID, double time)
	{
		static std::unordered_map<rttr::type, const SequenceTrackSegment*(SequenceControllerCurve::*)(const std::string&, double)> insertSegmentMap
		{
			{ RTTI_OF(SequenceTrackCurveFloat), &SequenceControllerCurve::insertCurveSegment<float> },
			{ RTTI_OF(SequenceTrackCurveVec2), &SequenceControllerCurve::insertCurveSegment<glm::vec2> },
			{ RTTI_OF(SequenceTrackCurveVec3), &SequenceControllerCurve::insertCurveSegment<glm::vec3> },
			{ RTTI_OF(SequenceTrackCurveVec4), &SequenceControllerCurve::insertCurveSegment<glm::vec4> }
		};

		auto* track = findTrack(trackID);
		assert(track != nullptr); // track not found

		if (track != nullptr)
		{
			auto it = insertSegmentMap.find(track->get_type());
			assert(it != insertSegmentMap.end()); // type not found
			if (it != insertSegmentMap.end())
			{
				return (*this.*it->second)(trackID, time);
			}
		}

		return nullptr;
	}


	void SequenceControllerCurve::deleteSegment(const std::string& trackID, const std::string& segmentID)
	{
		// pause player thread
		performEditAction([this, trackID, segmentID]()
		{
			//
			Sequence& sequence = getSequence();

			for (auto& track : sequence.mTracks)
			{
			  if (track->mID == trackID)
			  {
				  int segment_index = 0;
				  for (auto& segment : track->mSegments)
				  {
					  if (segment->mID == segmentID)
					  {
						  // store the duration of the segment that we are deleting
						  double duration = segment->mDuration;

						  // erase it from the list
						  track->mSegments.erase(track->mSegments.begin() + segment_index);

						  // get the segment that is now at the previous deleted segments position
						  if (track->mSegments.begin() + segment_index != track->mSegments.end())
						  {
							  // add the duration
							  track->mSegments[segment_index]->mDuration += duration;
						  }

						  deleteObjectFromSequencePlayer(segmentID);

						  // update segments
						  auto it = sUpdateSegmentFunctionMap.find(track->get_type());
						  if (it != sUpdateSegmentFunctionMap.end())
						  {
							  (*this.*it->second)(*track);
						  }

						  break;
					  }

					  updateTracks();
					  segment_index++;
				  }

				  break;
			  }
			}
		});
	}


	template<typename T>
	void SequenceControllerCurve::addNewCurveTrack()
	{
		performEditAction([this]()
		{
			// create sequence track
			std::unique_ptr<SequenceTrackCurve<T>> sequence_track = std::make_unique<SequenceTrackCurve<T>>();
			sequence_track->mID = sequenceutils::generateUniqueID(getPlayerReadObjectIDs() );

			//
			getSequence().mTracks.emplace_back(ResourcePtr<SequenceTrackCurve<T>>(sequence_track.get()));

			getPlayerOwnedObjects().emplace_back(std::move(sequence_track));
		});
	}


	template<typename T>
	const SequenceTrackSegment* SequenceControllerCurve::insertCurveSegment(const std::string& trackID, double time)
	{
		static std::unordered_map<rttr::type, int> s_curve_count_map
		{
			{ RTTI_OF(float), 1 },
			{ RTTI_OF(glm::vec2), 2 },
			{ RTTI_OF(glm::vec3), 3 },
			{ RTTI_OF(glm::vec4), 4 }
		};

		SequenceTrackSegment* return_ptr;

		performEditAction([this, trackID, time, &return_ptr]() mutable
		{
			auto it = s_curve_count_map.find(RTTI_OF(T));
			assert(it != s_curve_count_map.end()); // type not found

			int curve_count = it->second;

			//
			assert(curve_count > 0); // invalid curvecount

			//
			Sequence& sequence = getSequence();

			// find the right track
			for (auto& track : sequence.mTracks)
			{
			  if (track->mID == trackID)
			  {
				  // track found

				  // find the segment the new segment in inserted after
				  int segment_count = 1;
				  for (auto& segment : track->mSegments)
				  {
					  if (segment->mStartTime < time &&
						  segment->mStartTime + segment->mDuration > time)
					  {
						  // segment found

						  // create new segment & set parameters
						  std::unique_ptr<SequenceTrackSegmentCurve<T>> new_segment = std::make_unique<SequenceTrackSegmentCurve<T>>();
						  new_segment->mStartTime = time;
						  new_segment->mDuration = segment->mStartTime + segment->mDuration - time;
						  new_segment->mCurves.resize(curve_count);
						  new_segment->mCurveTypes.resize(curve_count);
						  for (int i = 0; i < curve_count; i++)
						  {
							  std::unique_ptr<math::FCurve<float, float>> segment_curve = std::make_unique<math::FCurve<float, float>>();
							  segment_curve->mID = sequenceutils::generateUniqueID(getPlayerReadObjectIDs());

							  // assign curve
							  new_segment->mCurves[i] = nap::ResourcePtr<math::FCurve<float, float>>(segment_curve.get());

							  // assign curve types
							  new_segment->mCurveTypes[i] = math::ECurveInterp::Bezier;

							  // move ownership
							  getPlayerOwnedObjects().emplace_back(std::move(segment_curve));
						  }

						  //
						  SequenceTrackSegmentCurve<T>& segment_curve_2 = static_cast<SequenceTrackSegmentCurve<T>&>(*segment.get());

						  // set the value by evaluation curve
						  new_segment->setStartValue(segment_curve_2.getValue(
							  (segment->mStartTime + segment->mDuration - time) / segment->mDuration)
						  );

						  // check if there is a next segment
						  if (segment_count < track->mSegments.size())
						  {
							  // if there is a next segment, the new segments end value is the start value of the next segment ...
							  SequenceTrackSegmentCurve<T>& next_segment_curve = static_cast<SequenceTrackSegmentCurve<T>&>(*track->mSegments[segment_count].get());

							  new_segment->setEndValue(next_segment_curve.getEndValue());
						  }
						  else
						  {
							  // ... otherwise it just gets this segments end value
							  new_segment->setEndValue(segment_curve_2.getEndValue());
						  }

						  // the segment's end value gets the start value the newly inserted segment
						  segment_curve_2.setEndValue(new_segment->getStartValue());

						  // change duration of segment before inserted segment
						  segment->mDuration = new_segment->mStartTime - segment->mStartTime;

						  // generate unique id
						  new_segment->mID = sequenceutils::generateUniqueID(getPlayerReadObjectIDs());

						  // wrap it in a resource ptr and insert it into the track
						  ResourcePtr<SequenceTrackSegment> new_segment_resource_ptr(new_segment.get());
						  track->mSegments.insert(track->mSegments.begin() + segment_count, new_segment_resource_ptr);

						  return_ptr = new_segment.get();

						  // move ownership to sequence player
						  getPlayerOwnedObjects().emplace_back(std::move(new_segment));

						  //
						  updateCurveSegments<T>(*(track.get()));

						  break;
					  }
					  else if (segment_count == track->mSegments.size())
					  {
						  // insert segment at the end of the list

						  // create new segment & set parameters
						  std::unique_ptr<SequenceTrackSegmentCurve<T>> new_segment =
							  std::make_unique<SequenceTrackSegmentCurve<T>>();
						  new_segment->mStartTime = segment->mStartTime + segment->mDuration;
						  new_segment->mDuration = time - new_segment->mStartTime;
						  new_segment->mCurves.resize(curve_count);
						  new_segment->mCurveTypes.resize(curve_count);
						  for (int v = 0; v < curve_count; v++)
						  {
							  std::unique_ptr<math::FCurve<float, float>> new_curve = std::make_unique<math::FCurve<float, float>>();
							  new_curve->mID = sequenceutils::generateUniqueID(getPlayerReadObjectIDs());
							  new_segment->mCurves[v] = ResourcePtr<math::FCurve<float, float>>(new_curve.get());
							  new_segment->mCurveTypes[v] = math::ECurveInterp::Bezier;
							  getPlayerOwnedObjects().emplace_back(std::move(new_curve));
						  }

						  // generate unique id
						  new_segment->mID = sequenceutils::generateUniqueID(getPlayerReadObjectIDs());

						  // wrap it in a resource ptr and insert it into the track
						  ResourcePtr<SequenceTrackSegment> new_segment_resource_ptr(new_segment.get());
						  track->mSegments.emplace_back(new_segment_resource_ptr);

						  return_ptr = new_segment.get();

						  // move ownership to sequence player
						  getPlayerOwnedObjects().emplace_back(std::move(new_segment));

						  //
						  updateCurveSegments<T>(*(track.get()));

						  break;
					  }

					  segment_count++;
				  }

				  //
				  if (track->mSegments.size() == 0)
				  {
					  // create new segment & set parameters
					  std::unique_ptr<SequenceTrackSegmentCurve<T>> new_segment = std::make_unique<SequenceTrackSegmentCurve<T>>();
					  new_segment->mStartTime = 0.0;
					  new_segment->mDuration = time - new_segment->mStartTime;

					  // make new curve of segment
					  new_segment->mCurves.resize(curve_count);
					  new_segment->mCurveTypes.resize(curve_count);
					  for (int v = 0; v < curve_count; v++)
					  {
						  std::unique_ptr<math::FCurve<float, float>> new_curve = std::make_unique<math::FCurve<float, float>>();
						  new_curve->mID = sequenceutils::generateUniqueID(getPlayerReadObjectIDs());
						  new_segment->mCurves[v] = ResourcePtr<math::FCurve<float, float>>(new_curve.get());
						  new_segment->mCurveTypes[v] = math::ECurveInterp::Bezier;
						  getPlayerOwnedObjects().emplace_back(std::move(new_curve));
					  }

					  // generate unique id
					  new_segment->mID = sequenceutils::generateUniqueID(getPlayerReadObjectIDs());

					  // wrap it in a resource ptr and insert it into the track
					  ResourcePtr<SequenceTrackSegment> new_segment_resource_ptr(new_segment.get());
					  track->mSegments.emplace_back(new_segment_resource_ptr);

					  //
					  return_ptr = new_segment.get();

					  // move ownership to sequence player
					  getPlayerOwnedObjects().emplace_back(std::move(new_segment));

					  //
					  updateCurveSegments<T>(*(track.get()));


					  break;
				  }
				  break;
			  }
			}
		});

		return return_ptr;
	}


	void SequenceControllerCurve::changeCurveType(const std::string& trackID, const std::string& segmentID, math::ECurveInterp type, int curveIndex)
	{
		static std::unordered_map<rttr::type, void(SequenceControllerCurve::*)(SequenceTrackSegment&, math::ECurveInterp type, int curveIndex)>change_curve_type_map
		{
			{ RTTI_OF(SequenceTrackSegmentCurveFloat), &SequenceControllerCurve::changeCurveType<float> },
			{ RTTI_OF(SequenceTrackSegmentCurveVec2), &SequenceControllerCurve::changeCurveType<glm::vec2> },
			{ RTTI_OF(SequenceTrackSegmentCurveVec3), &SequenceControllerCurve::changeCurveType<glm::vec3> },
			{ RTTI_OF(SequenceTrackSegmentCurveVec4), &SequenceControllerCurve::changeCurveType<glm::vec4> },
		};

		performEditAction([this, trackID, segmentID, type, curveIndex]()
		{
			auto* segment = findSegment(trackID, segmentID);
			assert(segment != nullptr); // segment not found

			if (segment != nullptr)
			{
				auto it = change_curve_type_map.find(segment->get_type());
				assert(it != change_curve_type_map.end()); // type not found
				if (it != change_curve_type_map.end())
				{
					(*this.*it->second)(*segment, type, curveIndex);
				}
			}
		});
	}


	template<typename T>
	void SequenceControllerCurve::changeCurveType(SequenceTrackSegment& segment, math::ECurveInterp type, int curveIndex)
	{
		auto* segment_curve = dynamic_cast<SequenceTrackSegmentCurve<T>*>(&segment);
		assert(segment_curve != nullptr); // type mismatch

		if (segment_curve != nullptr)
		{
			assert(segment_curve->mCurveTypes.size() > curveIndex); // curveIndex invalid

			segment_curve->mCurveTypes[curveIndex] = type;
			for (int j = 0; j < segment_curve->mCurves[curveIndex]->mPoints.size(); j++)
			{
				segment_curve->mCurves[curveIndex]->mPoints[j].mInterp = type;
			}
		}
	}


	void SequenceControllerCurve::changeCurveSegmentValue(const std::string& trackID, const std::string& segmentID, float newValue, int curveIndex,
														  SequenceCurveEnums::SegmentValueTypes valueType)
	{
		static std::unordered_map<rttr::type, void(SequenceControllerCurve::*)(SequenceTrack&, SequenceTrackSegment& segment, float, int, SequenceCurveEnums::SegmentValueTypes)> change_segment_value_map
		{
			{ RTTI_OF(SequenceTrackSegmentCurveFloat), &SequenceControllerCurve::changeCurveSegmentValue<float> },
			{ RTTI_OF(SequenceTrackSegmentCurveVec2), &SequenceControllerCurve::changeCurveSegmentValue<glm::vec2> },
			{ RTTI_OF(SequenceTrackSegmentCurveVec3), &SequenceControllerCurve::changeCurveSegmentValue<glm::vec3> },
			{ RTTI_OF(SequenceTrackSegmentCurveVec4), &SequenceControllerCurve::changeCurveSegmentValue<glm::vec4> }
		};

		//
		performEditAction([this, trackID, segmentID, newValue, curveIndex, valueType]()
		{
			SequenceTrack* track = findTrack(trackID);
			assert(track != nullptr); // track not found

			if (track != nullptr)
			{
				SequenceTrackSegment* segment = findSegment(trackID, segmentID);
				assert(segment != nullptr); // segment not found

				if (segment != nullptr)
				{
					auto it = change_segment_value_map.find(segment->get_type());
					assert(it != change_segment_value_map.end()); // type not found
					if (it != change_segment_value_map.end())
					{
						(*this.*it->second)(*track, *segment, newValue, curveIndex, valueType);
					}
				}
			}
		});
	}


	template<typename T>
	void SequenceControllerCurve::changeCurveSegmentValue(SequenceTrack& track, SequenceTrackSegment& segment, float newValue, int curveIndex,
														  SequenceCurveEnums::SegmentValueTypes valueType)
	{
		assert(segment.get_type().is_derived_from<SequenceTrackSegmentCurve<T>>()); // type mismatch
		SequenceTrackSegmentCurve<T>& curve_segment = static_cast<SequenceTrackSegmentCurve<T>&>(segment);

		switch (valueType)
		{
		case SequenceCurveEnums::BEGIN:
		{
			curve_segment.mCurves[curveIndex]->mPoints[0].mPos.mValue = newValue;
			curve_segment.mCurves[curveIndex]->mPoints[0].mPos.mValue = math::clamp<float>(curve_segment.mCurves[curveIndex]->mPoints[0].mPos.mValue, 0.0f, 1.0f);
		}
		break;
		case SequenceCurveEnums::END:
		{
			int lastPoint = curve_segment.mCurves[curveIndex]->mPoints.size() - 1;
			curve_segment.mCurves[curveIndex]->mPoints[lastPoint].mPos.mValue = newValue;
			curve_segment.mCurves[curveIndex]->mPoints[lastPoint].mPos.mValue = math::clamp<float>(curve_segment.mCurves[curveIndex]->mPoints[lastPoint].mPos.mValue, 0.0f, 1.0f);
		}
		break;
		}

		//
		updateCurveSegments<T>(track);
	}


	template<typename T>
	void SequenceControllerCurve::updateCurveSegments(SequenceTrack& track)
	{
		// update start time and duration of all segments
		ResourcePtr<SequenceTrackSegmentCurve<T>> prev_segment = nullptr;
		for (auto track_segment : track.mSegments)
		{
			if (prev_segment == nullptr)
			{
				track_segment->mStartTime = 0.0;
			}
			else
			{
				track_segment->mStartTime = prev_segment->mStartTime + prev_segment->mDuration;
				prev_segment->mDuration = track_segment->mStartTime - prev_segment->mStartTime;
			}
			prev_segment = static_cast<ResourcePtr<SequenceTrackSegmentCurve<T>>>(track_segment);
		}

		//
		prev_segment = nullptr;
		for (auto track_segment : track.mSegments)
		{
			auto& segment_curve = static_cast<SequenceTrackSegmentCurve<T>&>(*track_segment.get());

			if (prev_segment == nullptr)
			{
			}
			else
			{
				// if we have a previous segment, the curve gets the value of the start value of the current segment
				segment_curve.setStartValue(prev_segment->getEndValue());
			}
			prev_segment = &segment_curve;
		}
	}


	void SequenceControllerCurve::insertCurvePoint(const std::string& trackID, const std::string& segmentID, float pos, int curveIndex)
	{
		static std::unordered_map<rttr::type, void(SequenceControllerCurve::*)(SequenceTrackSegment&, float, int)> insertCurvePointMap
		{
			{ RTTI_OF(SequenceTrackSegmentCurveFloat), &SequenceControllerCurve::insertCurvePoint<float> },
			{ RTTI_OF(SequenceTrackSegmentCurveVec2), &SequenceControllerCurve::insertCurvePoint<glm::vec2> },
			{ RTTI_OF(SequenceTrackSegmentCurveVec3), &SequenceControllerCurve::insertCurvePoint<glm::vec3> },
			{ RTTI_OF(SequenceTrackSegmentCurveVec4), &SequenceControllerCurve::insertCurvePoint<glm::vec4> }
		};

		//
		performEditAction([this, trackID, segmentID, pos, curveIndex]()
		{
			// find segment
			SequenceTrackSegment* segment = findSegment(trackID, segmentID);
			assert(segment != nullptr); // segment not found

			//
			if (segment != nullptr)
			{
				auto it = insertCurvePointMap.find(segment->get_type());
				assert(it != insertCurvePointMap.end()); // type not found
				if (it != insertCurvePointMap.end())
				{
					(*this.*it->second)(*segment, pos, curveIndex);
				}
			}
		});
	}


	template <typename T>
	void SequenceControllerCurve::insertCurvePoint(SequenceTrackSegment& segment, float pos, int curveIndex)
	{
		assert(segment.get_type().is_derived_from<SequenceTrackSegmentCurve<T>>()); // type mismatch

		auto& curve_segment = static_cast<SequenceTrackSegmentCurve<T>&>(segment);
		assert(curveIndex < curve_segment.mCurves.size()); // invalid curve index

		// iterate trough points of curve
		for (int i = 0; i < curve_segment.mCurves[curveIndex]->mPoints.size() - 1; i++)
		{
			// find the point the new point needs to get inserted after
			if (curve_segment.mCurves[curveIndex]->mPoints[i].mPos.mTime <= pos
				&&
				curve_segment.mCurves[curveIndex]->mPoints[i + 1].mPos.mTime > pos)
			{
				// create point
				math::FCurvePoint<float, float> p;
				p.mPos.mTime = pos;
				p.mPos.mValue = curve_segment.mCurves[curveIndex]->evaluate(pos);
				p.mInTan.mTime = -0.1f;
				p.mOutTan.mTime = 0.1f;
				p.mInTan.mValue = 0.0f;
				p.mOutTan.mValue = 0.0f;
				p.mTangentsAligned = true;
				p.mInterp = curve_segment.mCurveTypes[curveIndex];

				// insert point
				curve_segment.mCurves[curveIndex]->mPoints.insert(curve_segment.mCurves[curveIndex]->mPoints.begin() + i + 1, p);
				curve_segment.mCurves[curveIndex]->invalidate();
				break;
			}
		}
	}


	void SequenceControllerCurve::deleteCurvePoint(
		const std::string& trackID,
		const std::string& segmentID,
		const int index,
		int curveIndex)
	{
		static std::unordered_map<rttr::type, void(SequenceControllerCurve::*)(SequenceTrackSegment&, const int, int)> deleteCurvePointMap
		{
			{ RTTI_OF(SequenceTrackSegmentCurveFloat), &SequenceControllerCurve::deleteCurvePoint<float> },
			{ RTTI_OF(SequenceTrackSegmentCurveVec2), &SequenceControllerCurve::deleteCurvePoint<glm::vec2> },
			{ RTTI_OF(SequenceTrackSegmentCurveVec3), &SequenceControllerCurve::deleteCurvePoint<glm::vec3> },
			{ RTTI_OF(SequenceTrackSegmentCurveVec4), &SequenceControllerCurve::deleteCurvePoint<glm::vec4> },
		};

		performEditAction([this, trackID, segmentID, index, curveIndex]()
		{
			// find segment
			SequenceTrackSegment* segment = findSegment(trackID, segmentID);
			assert(segment != nullptr); // segment not found

			//
			if (segment != nullptr)
			{
				auto it = deleteCurvePointMap.find(segment->get_type());
				assert(it != deleteCurvePointMap.end()); // type not found
				if (it != deleteCurvePointMap.end())
				{
					(*this.*it->second)(*segment, index, curveIndex);
				}
			}
		});
	}


	template<typename T>
	void SequenceControllerCurve::deleteCurvePoint(SequenceTrackSegment& segment, const int index, int curveIndex)
	{
		assert(segment.get_type().is_derived_from<SequenceTrackSegmentCurve<T>>()); // type mismatch

		//
		auto& curve_segment = static_cast<SequenceTrackSegmentCurve<T>&>(segment);
		assert(curveIndex < curve_segment.mCurves.size()); // invalid curve index

		if (index < curve_segment.mCurves[curveIndex]->mPoints.size())
		{
			//
			curve_segment.mCurves[curveIndex]->mPoints.erase(curve_segment.mCurves[curveIndex]->mPoints.begin() + index);
			curve_segment.mCurves[curveIndex]->invalidate();
		}
	}


	void SequenceControllerCurve::changeCurvePoint(
		const std::string& trackID,
		const std::string& segmentID,
		const int pointIndex,
		const int curveIndex,
		float time,
		float value)
	{
		static std::unordered_map<rttr::type, void(SequenceControllerCurve::*)(SequenceTrackSegment&, const int, const int, float, float)> change_curve_point_map
		{
			{ RTTI_OF(SequenceTrackSegmentCurveFloat), &SequenceControllerCurve::changeCurvePoint<float> },
			{ RTTI_OF(SequenceTrackSegmentCurveVec2), &SequenceControllerCurve::changeCurvePoint<glm::vec2> },
			{ RTTI_OF(SequenceTrackSegmentCurveVec3), &SequenceControllerCurve::changeCurvePoint<glm::vec3> },
			{ RTTI_OF(SequenceTrackSegmentCurveVec4), &SequenceControllerCurve::changeCurvePoint<glm::vec4> },
		};

		performEditAction([this, trackID, segmentID, pointIndex, curveIndex, time, value]()
		{
			// find segment
			SequenceTrackSegment* segment = findSegment(trackID, segmentID);
			assert(segment != nullptr); // segment not found

			if (segment != nullptr)
			{
			  auto it = change_curve_point_map.find(segment->get_type());
			  assert(it != change_curve_point_map.end()); // type not found
			  if (it != change_curve_point_map.end())
			  {
				  (*this.*it->second)(*segment, pointIndex, curveIndex, time, value);
			  }
			}
		});
	}


	template <typename T>
	void SequenceControllerCurve::changeCurvePoint(SequenceTrackSegment& segment, const int pointIndex, const int curveIndex, float time, float value)
	{
		//
		assert(segment.get_type().is_derived_from<SequenceTrackSegmentCurve<T>>()); // type mismatch
		auto& curve_segment = static_cast<SequenceTrackSegmentCurve<T>&>(segment);;
		assert(curveIndex < curve_segment.mCurves.size()); // invalid curve index
		assert(pointIndex < curve_segment.mCurves[curveIndex]->mPoints.size()); // invalid point index

		//
		math::FCurvePoint<float, float>& curve_point = curve_segment.mCurves[curveIndex]->mPoints[pointIndex];
		curve_point.mPos.mTime = time;
		curve_point.mPos.mValue = value;
		curve_point.mPos.mValue = math::clamp<float>(curve_point.mPos.mValue, 0.0f, 1.0f);
		curve_segment.mCurves[curveIndex]->invalidate();
	}


	void SequenceControllerCurve::changeTanPoint(const std::string& trackID, const std::string& segmentID, const int pointIndex, const int curveIndex,
												 SequenceCurveEnums::TanPointTypes tanType, float time, float value)
	{
		//
		static std::unordered_map<rttr::type, void(SequenceControllerCurve::*)(SequenceTrackSegment&, const std::string&, const int, const int,
												  SequenceCurveEnums::TanPointTypes, float, float)> change_curve_point_map
		{
			{ RTTI_OF(SequenceTrackSegmentCurveFloat), &SequenceControllerCurve::changeTanPoint<float> },
			{ RTTI_OF(SequenceTrackSegmentCurveVec2), &SequenceControllerCurve::changeTanPoint<glm::vec2> },
			{ RTTI_OF(SequenceTrackSegmentCurveVec3), &SequenceControllerCurve::changeTanPoint<glm::vec3> },
			{ RTTI_OF(SequenceTrackSegmentCurveVec4), &SequenceControllerCurve::changeTanPoint<glm::vec4> },
		};

		performEditAction([this, trackID, segmentID, pointIndex, curveIndex, tanType, time, value]()
		{
			// find segment
			SequenceTrackSegment* segment = findSegment(trackID, segmentID);
			assert(segment != nullptr); // segment not found

			if (segment != nullptr)
			{
				auto it = change_curve_point_map.find(segment->get_type());
				assert(it != change_curve_point_map.end()); // type not found
				if (it != change_curve_point_map.end())
				{
					// call appropriate function
					(*this.*it->second)(*segment, trackID, pointIndex, curveIndex, tanType, time, value);
				}
			}
		});
	}


	template<typename T>
	void SequenceControllerCurve::changeTanPoint(SequenceTrackSegment& segment, const std::string& trackID, const int pointIndex, const int curveIndex,
												 SequenceCurveEnums::TanPointTypes tanType, float time, float value)
	{
		assert(segment.get_type().is_derived_from<SequenceTrackSegmentCurve<T>>()); // type mismatch
		auto& curve_segment = static_cast<SequenceTrackSegmentCurve<T>&>(segment);;
		assert(curveIndex < curve_segment.mCurves.size()); // invalid curve index
		assert(pointIndex < curve_segment.mCurves[curveIndex]->mPoints.size()); // invalid point index

		//
		auto& curve_point = curve_segment.mCurves[curveIndex]->mPoints[pointIndex];
		switch (tanType)
		{
		case SequenceCurveEnums::TanPointTypes::IN:
		{
			if (time < curve_point.mOutTan.mTime)
			{
				curve_point.mInTan.mTime = time;
				curve_point.mInTan.mValue = value;

				if (curve_point.mTangentsAligned)
				{
					curve_point.mOutTan.mTime = -curve_point.mInTan.mTime;
					curve_point.mOutTan.mValue = -curve_point.mInTan.mValue;
				}
			}
		}
		break;
		case SequenceCurveEnums::TanPointTypes::OUT:
		{
			if (time > curve_point.mInTan.mTime)
			{
				curve_point.mOutTan.mTime = time;
				curve_point.mOutTan.mValue = value;

				if (curve_point.mTangentsAligned)
				{
					curve_point.mInTan.mTime = -curve_point.mOutTan.mTime;
					curve_point.mInTan.mValue = -curve_point.mOutTan.mValue;
				}
			}

		}
		break;
		}

		// is this the last control point ?
		// then also change the first control point of the next segment accordinly
		if (pointIndex == curve_segment.mCurves[curveIndex]->mPoints.size() - 1)
		{
			SequenceTrack* track = findTrack(trackID);
			assert(track != nullptr); // track not found

			for (int i = 0; i < track->mSegments.size(); i++)
			{
				if (track->mSegments[i].get() == &segment &&
					i + 1 < track->mSegments.size())
				{
					auto& next_segment_curve_point = static_cast<SequenceTrackSegmentCurve<T>&>(*track->mSegments[i + 1]).mCurves[curveIndex]->mPoints[0];

					next_segment_curve_point.mInTan.mTime = curve_point.mInTan.mTime;
					next_segment_curve_point.mInTan.mValue = curve_point.mInTan.mValue;
					next_segment_curve_point.mOutTan.mTime = curve_point.mOutTan.mTime;
					next_segment_curve_point.mOutTan.mValue = curve_point.mOutTan.mValue;

				}
			}
		}

		curve_segment.mCurves[curveIndex]->invalidate();
	}


	void SequenceControllerCurve::insertTrack(rttr::type type)
	{
		if( type == RTTI_OF(SequenceTrackCurveFloat))
			addNewCurveTrack<float>();
		if (type == RTTI_OF(SequenceTrackCurveVec2))
			addNewCurveTrack<glm::vec2>();
		if (type == RTTI_OF(SequenceTrackCurveVec3))
			addNewCurveTrack<glm::vec3>();
		if (type == RTTI_OF(SequenceTrackCurveVec4))
			addNewCurveTrack<glm::vec4>();
	}


	template<>
	void SequenceControllerCurve::changeMinMaxCurveTrack<float>(const std::string& trackID, float minimum, float maximum)
	{
		performEditAction([this, trackID, minimum, maximum]()
		{
			SequenceTrack* track = findTrack(trackID);
			assert(track != nullptr); // track not found

			SequenceTrackCurveFloat* track_curve = static_cast<SequenceTrackCurveFloat*>(track);

			for(auto& segment : track_curve->mSegments)
			{
				SequenceTrackSegmentCurveFloat& curve_segment = static_cast<SequenceTrackSegmentCurveFloat&>(*segment.get());
				int curve_count = 0;
				for(auto& curve : curve_segment.mCurves)
				{
					for(auto& point : curve->mPoints)
					{
						float value = point.mPos.mValue * ( track_curve->mMaximum - track_curve->mMinimum) + track_curve->mMinimum;

						point.mPos.mValue = ( value - minimum ) / ( maximum - minimum ) ;
						point.mPos.mValue = math::clamp<float>(point.mPos.mValue, 0, 1);
					}
					curve_count++;
				}
			}

			track_curve->mMinimum = minimum;
			track_curve->mMaximum = maximum;
		});
	}

	template<typename T>
	void SequenceControllerCurve::changeMinMaxCurveTrack(const std::string& trackID, T minimum, T maximum)
	{
		performEditAction([this, trackID, minimum, maximum]()
		{
			SequenceTrack* track = findTrack(trackID);
			assert(track != nullptr); // track not found

			SequenceTrackCurve<T>* track_curve = static_cast<SequenceTrackCurve<T>*>(track);

			for(auto& segment : track_curve->mSegments)
			{
				SequenceTrackSegmentCurve<T>& curve_segment = static_cast<SequenceTrackSegmentCurve<T>&>(*segment.get());
				int curve_count = 0;
				for(auto& curve : curve_segment.mCurves)
				{
					for(auto& point : curve->mPoints)
					{
						float value = point.mPos.mValue * ( track_curve->mMaximum[curve_count] - track_curve->mMinimum[curve_count] ) + track_curve->mMinimum[curve_count] ;

						point.mPos.mValue = ( value - minimum[curve_count] ) / ( maximum[curve_count] - minimum[curve_count] ) ;
						point.mPos.mValue = math::clamp<float>(point.mPos.mValue, 0, 1);
					}
					curve_count++;
				}
			}

			track_curve->mMinimum = minimum;
			track_curve->mMaximum = maximum;
		});
	}


	// explicit template declarations
	template NAPAPI void SequenceControllerCurve::addNewCurveTrack<float>();
	template NAPAPI void SequenceControllerCurve::addNewCurveTrack<glm::vec2>();
	template NAPAPI void SequenceControllerCurve::addNewCurveTrack<glm::vec3>();
	template NAPAPI void SequenceControllerCurve::addNewCurveTrack<glm::vec4>();

	template NAPAPI const SequenceTrackSegment* SequenceControllerCurve::insertCurveSegment<float>(const std::string& trackID, double time);
	template NAPAPI const SequenceTrackSegment* SequenceControllerCurve::insertCurveSegment<glm::vec2>(const std::string& trackID, double time);
	template NAPAPI const SequenceTrackSegment* SequenceControllerCurve::insertCurveSegment<glm::vec3>(const std::string& trackID, double time);
	template NAPAPI const SequenceTrackSegment* SequenceControllerCurve::insertCurveSegment<glm::vec4>(const std::string& trackID, double time);

	template NAPAPI void SequenceControllerCurve::insertCurvePoint<float>(SequenceTrackSegment& segment, float pos, int curveIndex);
	template NAPAPI void SequenceControllerCurve::insertCurvePoint<glm::vec2>(SequenceTrackSegment& segment, float pos, int curveIndex);
	template NAPAPI void SequenceControllerCurve::insertCurvePoint<glm::vec3>(SequenceTrackSegment& segment, float pos, int curveIndex);
	template NAPAPI void SequenceControllerCurve::insertCurvePoint<glm::vec4>(SequenceTrackSegment& segment, float pos, int curveIndex);

	template NAPAPI void SequenceControllerCurve::changeCurvePoint<float>(SequenceTrackSegment& segment, const int pointIndex, const int curveIndex, float time, float value);
	template NAPAPI void SequenceControllerCurve::changeCurvePoint<glm::vec2>(SequenceTrackSegment& segment, const int pointIndex, const int curveIndex, float time, float value);
	template NAPAPI void SequenceControllerCurve::changeCurvePoint<glm::vec3>(SequenceTrackSegment& segment, const int pointIndex, const int curveIndex, float time, float value);
	template NAPAPI void SequenceControllerCurve::changeCurvePoint<glm::vec4>(SequenceTrackSegment& segment, const int pointIndex, const int curveIndex, float time, float value);

	template NAPAPI void SequenceControllerCurve::changeCurveSegmentValue<float>(SequenceTrack& track, SequenceTrackSegment& segment, float amount, int curveIndex,
															SequenceCurveEnums::SegmentValueTypes valueType);
	template NAPAPI void SequenceControllerCurve::changeCurveSegmentValue<glm::vec2>(SequenceTrack& track, SequenceTrackSegment& segment, float amount, int curveIndex,
																SequenceCurveEnums::SegmentValueTypes valueType);
	template NAPAPI void SequenceControllerCurve::changeCurveSegmentValue<glm::vec3>(SequenceTrack& track, SequenceTrackSegment& segment, float amount, int curveIndex,
																SequenceCurveEnums::SegmentValueTypes valueType);
	template NAPAPI void SequenceControllerCurve::changeCurveSegmentValue<glm::vec4>(SequenceTrack& track, SequenceTrackSegment& segment, float amount, int curveIndex,
																SequenceCurveEnums::SegmentValueTypes valueType);

	template NAPAPI void SequenceControllerCurve::changeTanPoint<float>(SequenceTrackSegment& segment, const std::string& trackID, const int pointIndex, const int curveIndex,
																		SequenceCurveEnums::TanPointTypes tanType, float time, float value);
	template NAPAPI void SequenceControllerCurve::changeTanPoint<glm::vec2>(SequenceTrackSegment& segment, const std::string& trackID, const int pointIndex, const int curveIndex,
																			SequenceCurveEnums::TanPointTypes tanType, float time, float value);
	template NAPAPI void SequenceControllerCurve::changeTanPoint<glm::vec3>(SequenceTrackSegment& segment, const std::string& trackID, const int pointIndex, const int curveIndex,
																			SequenceCurveEnums::TanPointTypes tanType, float time, float value);
	template NAPAPI void SequenceControllerCurve::changeTanPoint<glm::vec4>(SequenceTrackSegment& segment, const std::string& trackID, const int pointIndex, const int curveIndex,
																			SequenceCurveEnums::TanPointTypes tanType, float time, float value);

	template NAPAPI void SequenceControllerCurve::deleteCurvePoint<float>(SequenceTrackSegment& segment, const int index, int curveIndex);
	template NAPAPI void SequenceControllerCurve::deleteCurvePoint<glm::vec2>(SequenceTrackSegment& segment, const int index, int curveIndex);
	template NAPAPI void SequenceControllerCurve::deleteCurvePoint<glm::vec3>(SequenceTrackSegment& segment, const int index, int curveIndex);
	template NAPAPI void SequenceControllerCurve::deleteCurvePoint<glm::vec4>(SequenceTrackSegment& segment, const int index, int curveIndex);

	template NAPAPI void SequenceControllerCurve::changeMinMaxCurveTrack<float>(const std::string& trackID, float minimum, float maximum);
	template NAPAPI void SequenceControllerCurve::changeMinMaxCurveTrack<glm::vec2>(const std::string& trackID, glm::vec2 minimum, glm::vec2 maximum);
	template NAPAPI void SequenceControllerCurve::changeMinMaxCurveTrack<glm::vec3>(const std::string& trackID, glm::vec3 minimum, glm::vec3 maximum);
	template NAPAPI void SequenceControllerCurve::changeMinMaxCurveTrack<glm::vec4>(const std::string& trackID, glm::vec4 minimum, glm::vec4 maximum);

	template NAPAPI void SequenceControllerCurve::changeCurveType<float>(SequenceTrackSegment& segment, math::ECurveInterp type, int curveIndex);
	template NAPAPI void SequenceControllerCurve::changeCurveType<glm::vec2>(SequenceTrackSegment& segment, math::ECurveInterp type, int curveIndex);
	template NAPAPI void SequenceControllerCurve::changeCurveType<glm::vec3>(SequenceTrackSegment& segment, math::ECurveInterp type, int curveIndex);
	template NAPAPI void SequenceControllerCurve::changeCurveType<glm::vec4>(SequenceTrackSegment& segment, math::ECurveInterp type, int curveIndex);
}

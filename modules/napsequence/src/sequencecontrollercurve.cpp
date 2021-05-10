/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "sequencecontrollercurve.h"
#include "sequenceeditor.h"
#include "sequencetracksegmentcurve.h"

namespace nap
{
	// map for updating segments

	// map for inserting segments
	static std::unordered_map<rttr::type, void(SequenceControllerCurve::*)(SequenceTrack&)> update_segment_functions
		;

	SequenceControllerCurve::SequenceControllerCurve(SequencePlayer& player, SequenceEditor& editor) :
		  SequenceController(player, editor)
	{
		mUpdateSegmentFunctionMap =
		{
			{RTTI_OF(SequenceTrackCurveFloat), 	[this](SequenceTrack& track){this->updateCurveSegments<float>(track); }},
			{RTTI_OF(SequenceTrackCurveVec2), 	[this](SequenceTrack& track){this->updateCurveSegments<glm::vec2>(track); }},
			{RTTI_OF(SequenceTrackCurveVec3), 	[this](SequenceTrack& track){this->updateCurveSegments<glm::vec3>(track); }},
			{RTTI_OF(SequenceTrackCurveVec4), 	[this](SequenceTrack& track){this->updateCurveSegments<glm::vec4>(track); }}
		};

		mInsertSegmentFunctionMap =
		{
			{RTTI_OF(SequenceTrackCurveFloat), 	[this](const std::string& trackID, double time){return this->insertCurveSegment<float>(trackID, time); }},
			{RTTI_OF(SequenceTrackCurveVec2), 	[this](const std::string& trackID, double time){return this->insertCurveSegment<glm::vec2>(trackID, time); }},
			{RTTI_OF(SequenceTrackCurveVec3), 	[this](const std::string& trackID, double time){return this->insertCurveSegment<glm::vec3>(trackID, time); }},
			{RTTI_OF(SequenceTrackCurveVec4), 	[this](const std::string& trackID, double time){return this->insertCurveSegment<glm::vec4>(trackID, time); }},
		};
	}

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

							auto it = mUpdateSegmentFunctionMap.find(track->get_type());
							assert (it!=mUpdateSegmentFunctionMap.end());
							it->second(*track);

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
		auto* track = findTrack(trackID);
		assert(track != nullptr); // track not found

		if (track != nullptr)
		{
			auto it = mInsertSegmentFunctionMap.find(track->get_type());
			assert(it != mInsertSegmentFunctionMap.end()); // type not found
			it->second(trackID, time);
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
							auto it = mUpdateSegmentFunctionMap.find(track->get_type());
							assert (it != mUpdateSegmentFunctionMap.end());
							it->second(*track);

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

		static std::unordered_map<rttr::type, void(SequenceControllerCurve::*)(SequenceTrackSegment&, const int, float, float)> change_last_curve_point_map
			{
				{ RTTI_OF(SequenceTrackSegmentCurveFloat), &SequenceControllerCurve::changeLastCurvePoint<float> },
				{ RTTI_OF(SequenceTrackSegmentCurveVec2), &SequenceControllerCurve::changeLastCurvePoint<glm::vec2> },
				{ RTTI_OF(SequenceTrackSegmentCurveVec3), &SequenceControllerCurve::changeLastCurvePoint<glm::vec3> },
				{ RTTI_OF(SequenceTrackSegmentCurveVec4), &SequenceControllerCurve::changeLastCurvePoint<glm::vec4> },
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

					// if point index = 0, check if we need to update end point of any previous segment
					if( pointIndex == 0 )
					{
						// find corresponding track
						auto* track = findTrack(trackID);

						// iterate trough segments
						for(size_t i = 0; i < track->mSegments.size(); i++)
						{
							// if this segment is found, and index is bigger then 0 obtain the previous segment
							if( track->mSegments[i].get() == segment && i > 0 )
							{
								// obtain previous segment
								auto* prev_segment = track->mSegments[i-1].get();

								// find corresponding function call and invoke changeLastCurvePoint on the previous segment
								auto it2 = change_last_curve_point_map.find(segment->get_type());
								assert(it2 != change_last_curve_point_map.end()); // type not found
								if (it2 != change_last_curve_point_map.end())
								{
									(*this.*it2->second)(*prev_segment, curveIndex, 1.0f, value);
								}

								break;
							}
						}
					}
				}
			}
		});
	}


	bool SequenceControllerCurve::changeTanPoint(const std::string& trackID, const std::string& segmentID, const int pointIndex, const int curveIndex,
												 SequenceCurveEnums::ETanPointTypes tanType, float time, float value)
	{
		//
		static std::unordered_map<rttr::type, bool(SequenceControllerCurve::*)(SequenceTrackSegment&, const std::string&, const int, const int,
																			   SequenceCurveEnums::ETanPointTypes, float, float)> change_curve_point_map
			{
				{ RTTI_OF(SequenceTrackSegmentCurveFloat), &SequenceControllerCurve::changeTanPoint<float> },
				{ RTTI_OF(SequenceTrackSegmentCurveVec2), &SequenceControllerCurve::changeTanPoint<glm::vec2> },
				{ RTTI_OF(SequenceTrackSegmentCurveVec3), &SequenceControllerCurve::changeTanPoint<glm::vec3> },
				{ RTTI_OF(SequenceTrackSegmentCurveVec4), &SequenceControllerCurve::changeTanPoint<glm::vec4> },
			};

		bool tangents_flipped = false;
		performEditAction([this, trackID, segmentID, pointIndex, curveIndex, tanType, time, value, &tangents_flipped]()
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
					tangents_flipped = (*this.*it->second)(*segment, trackID, pointIndex, curveIndex, tanType, time, value);
				}
			}
		});

		return tangents_flipped;
	}


	void SequenceControllerCurve::insertTrack(rtti::TypeInfo type)
	{
		static std::unordered_map<rtti::TypeInfo, std::function<void(SequenceControllerCurve*)>> insert_track_function_map
		{
			{RTTI_OF(SequenceTrackCurveFloat), [](SequenceControllerCurve* controller){controller->addNewCurveTrack<float>();}},
			{RTTI_OF(SequenceTrackCurveVec2), [](SequenceControllerCurve* controller){controller->addNewCurveTrack<glm::vec2>();}},
			{RTTI_OF(SequenceTrackCurveVec3), [](SequenceControllerCurve* controller){controller->addNewCurveTrack<glm::vec3>();}},
			{RTTI_OF(SequenceTrackCurveVec4), [](SequenceControllerCurve* controller){controller->addNewCurveTrack<glm::vec4>();}}
		};

		assert(insert_track_function_map.find(type)!=insert_track_function_map.end()); // function not found
		auto& insert_track_function = insert_track_function_map[type];
		insert_track_function(this);
	}


	void SequenceControllerCurve::updateCurveSegments(const std::string& trackID)
	{
		auto* track = findTrack(trackID);
		if(track!= nullptr)
		{
			auto it = mUpdateSegmentFunctionMap.find(track->get_type());
			assert(it != mUpdateSegmentFunctionMap.end());
			it->second(*track);
		}
	}


	template<>
	void SequenceControllerCurve::changeMinMaxCurveTrack<float>(const std::string& trackID, float minimum, float maximum)
	{
		performEditAction([this, trackID, minimum, maximum]()
		{
			SequenceTrack* track = findTrack(trackID);
			assert(track != nullptr); // track not found

			auto* track_curve = static_cast<SequenceTrackCurveFloat*>(track);

			for(auto& segment : track_curve->mSegments)
			{
				auto& curve_segment = *static_cast<SequenceTrackSegmentCurveFloat*>(segment.get());
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
}
// Local Includes
#include "sequencecontrollercurve.h"
#include "sequenceutils.h"
#include "sequencetrackcurve.h"
#include "sequenceeditor.h"

// Nap Includes
#include <mathutils.h>

namespace nap
{
	static bool sRegistered = SequenceController::registerControllerFactory(RTTI_OF(SequenceControllerCurve), [](SequencePlayer& player)->std::unique_ptr<SequenceController> { return std::make_unique<SequenceControllerCurve>(player); });


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


	void SequenceControllerCurve::segmentDurationChange(const std::string& trackID, const std::string& segmentID, float amount)
	{
		// lock
		std::unique_lock<std::mutex> l = lock();

		//
		Sequence& sequence = getSequence();

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
					double new_duration = track_segment->mDuration + amount;

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
						track_segment->mDuration += amount;

						auto it = sUpdateSegmentFunctionMap.find(track->get_type());
						if (it != sUpdateSegmentFunctionMap.end())
						{
							(*this.*it->second)(*track);
						}

						updateTracks();
					}
					break;
				}

				previous_segment = track_segment;
			}
		}
	}


	void SequenceControllerCurve::insertSegment(const std::string& trackID, double time)
	{
		static std::unordered_map<rttr::type, void(SequenceControllerCurve::*)(const std::string&, double)> insertSegmentMap
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
				(*this.*it->second)(trackID, time);
			}
		}
	}


	void SequenceControllerCurve::deleteSegment(const std::string& trackID, const std::string& segmentID)
	{
		// pause player thread
		std::unique_lock<std::mutex> l = lock();

		//
		Sequence& sequence = getSequence();

		for (auto& track : sequence.mTracks)
		{
			if (track->mID == trackID)
			{
				int segmentIndex = 0;
				for (auto& segment : track->mSegments)
				{
					if (segment->mID == segmentID)
					{
						// store the duration of the segment that we are deleting
						double duration = segment->mDuration;

						// erase it from the list
						track->mSegments.erase(track->mSegments.begin() + segmentIndex);

						// get the segment that is now at the previous deleted segments position
						if (track->mSegments.begin() + segmentIndex != track->mSegments.end())
						{
							// add the duration
							track->mSegments[segmentIndex]->mDuration += duration;
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
					segmentIndex++;
				}

				break;
			}
		}
	}


	template<typename T>
	void SequenceControllerCurve::addNewCurveTrack()
	{
		std::unique_lock<std::mutex> l = lock();

		// create sequence track
		std::unique_ptr<SequenceTrackCurve<T>> sequenceTrack = std::make_unique<SequenceTrackCurve<T>>();
		sequenceTrack->mID = sequenceutils::generateUniqueID(getPlayerReadObjectIDs() );

		//
		getSequence().mTracks.emplace_back(ResourcePtr<SequenceTrackCurve<T>>(sequenceTrack.get()));

		getPlayerOwnedObjects().emplace_back(std::move(sequenceTrack));
	}


	template<typename T>
	void SequenceControllerCurve::insertCurveSegment(const std::string& trackID, double time)
	{
		auto l = lock();

		static std::unordered_map<rttr::type, int> curveCountMap
		{
			{ RTTI_OF(float), 1 },
			{ RTTI_OF(glm::vec2), 2 },
			{ RTTI_OF(glm::vec3), 3 },
			{ RTTI_OF(glm::vec4), 4 }
		};

		auto it = curveCountMap.find(RTTI_OF(T));
		assert(it != curveCountMap.end()); // type not found

		int curveCount = it->second;

		//
		assert(curveCount > 0); // invalid curvecount

		//
		Sequence& sequence = getSequence();

		// find the right track
		for (auto& track : sequence.mTracks)
		{
			if (track->mID == trackID)
			{
				// track found

				// find the segment the new segment in inserted after
				int segmentCount = 1;
				for (auto& segment : track->mSegments)
				{
					if (segment->mStartTime < time &&
						segment->mStartTime + segment->mDuration > time)
					{
						// segment found

						// create new segment & set parameters
						std::unique_ptr<SequenceTrackSegmentCurve<T>> newSegment = std::make_unique<SequenceTrackSegmentCurve<T>>();
						newSegment->mStartTime = time;
						newSegment->mDuration = segment->mStartTime + segment->mDuration - time;
						newSegment->mCurves.resize(curveCount);
						for (int i = 0; i < curveCount; i++)
						{
							std::unique_ptr<math::FCurve<float, float>> segmentCurve = std::make_unique<math::FCurve<float, float>>();
							segmentCurve->mID = sequenceutils::generateUniqueID(getPlayerReadObjectIDs());

							// assign curve
							newSegment->mCurves[i] = nap::ResourcePtr<math::FCurve<float, float>>(segmentCurve.get());

							// move ownership
							getPlayerOwnedObjects().emplace_back(std::move(segmentCurve));
						}

						//
						SequenceTrackSegmentCurve<T>& segmentVec = static_cast<SequenceTrackSegmentCurve<T>&>(*segment.get());

						// set the value by evaluation curve
						newSegment->setStartValue(
							segmentVec.getValue(
							(segment->mStartTime + segment->mDuration - time) / segment->mDuration)
						);

						// check if there is a next segment
						if (segmentCount < track->mSegments.size())
						{
							// if there is a next segment, the new segments end value is the start value of the next segment ...
							SequenceTrackSegmentCurve<T>& nextSegmentFloat = static_cast<SequenceTrackSegmentCurve<T>&>(*track->mSegments[segmentCount].get());

							newSegment->setEndValue(nextSegmentFloat.getEndValue());
						}
						else
						{
							// ... otherwise it just gets this segments end value
							newSegment->setEndValue(segmentVec.getEndValue());
						}

						// the segment's end value gets the start value the newly inserted segment 
						segmentVec.setEndValue(newSegment->getStartValue());

						// change duration of segment before inserted segment
						segment->mDuration = newSegment->mStartTime - segment->mStartTime;

						// generate unique id
						newSegment->mID = sequenceutils::generateUniqueID(getPlayerReadObjectIDs());

						// wrap it in a resource ptr and insert it into the track
						ResourcePtr<SequenceTrackSegment> newSegmentResourcePtr(newSegment.get());
						track->mSegments.insert(track->mSegments.begin() + segmentCount, newSegmentResourcePtr);

						// move ownership to sequence player
						getPlayerOwnedObjects().emplace_back(std::move(newSegment));

						//
						updateCurveSegments<T>(*(track.get()));

						break;
					}
					else if (segmentCount == track->mSegments.size())
					{
						// insert segment at the end of the list

						// create new segment & set parameters
						std::unique_ptr<SequenceTrackSegmentCurve<T>> newSegment =
							std::make_unique<SequenceTrackSegmentCurve<T>>();
						newSegment->mStartTime = segment->mStartTime + segment->mDuration;
						newSegment->mDuration = time - newSegment->mStartTime;
						newSegment->mCurves.resize(curveCount);
						for (int v = 0; v < curveCount; v++)
						{
							std::unique_ptr<math::FCurve<float, float>> newCurve = std::make_unique<math::FCurve<float, float>>();
							newCurve->mID = sequenceutils::generateUniqueID(getPlayerReadObjectIDs());
							newSegment->mCurves[v] = ResourcePtr<math::FCurve<float, float>>(newCurve.get());
							getPlayerOwnedObjects().emplace_back(std::move(newCurve));
						}

						// generate unique id
						newSegment->mID = sequenceutils::generateUniqueID(getPlayerReadObjectIDs());

						// wrap it in a resource ptr and insert it into the track
						ResourcePtr<SequenceTrackSegment> newSegmentResourcePtr(newSegment.get());
						track->mSegments.emplace_back(newSegmentResourcePtr);

						// move ownership to sequence player
						getPlayerOwnedObjects().emplace_back(std::move(newSegment));

						//
						updateCurveSegments<T>(*(track.get()));

						break;
					}


					segmentCount++;
				}

				//
				if (track->mSegments.size() == 0)
				{
					// create new segment & set parameters
					std::unique_ptr<SequenceTrackSegmentCurve<T>> newSegment = std::make_unique<SequenceTrackSegmentCurve<T>>();
					newSegment->mStartTime = 0.0;
					newSegment->mDuration = time - newSegment->mStartTime;

					// make new curve of segment
					newSegment->mCurves.resize(curveCount);
					for (int v = 0; v < curveCount; v++)
					{
						std::unique_ptr<math::FCurve<float, float>> newCurve = std::make_unique<math::FCurve<float, float>>();
						newCurve->mID = sequenceutils::generateUniqueID(getPlayerReadObjectIDs());
						newSegment->mCurves[v] = ResourcePtr<math::FCurve<float, float>>(newCurve.get());
						getPlayerOwnedObjects().emplace_back(std::move(newCurve));
					}

					// generate unique id
					newSegment->mID = sequenceutils::generateUniqueID(getPlayerReadObjectIDs());

					// wrap it in a resource ptr and insert it into the track
					ResourcePtr<SequenceTrackSegment> newSegmentResourcePtr(newSegment.get());
					track->mSegments.emplace_back(newSegmentResourcePtr);

					// move ownership to sequence player
					getPlayerOwnedObjects().emplace_back(std::move(newSegment));

					//
					updateCurveSegments<T>(*(track.get()));
				}
				break;
			}
		}
	}


	void SequenceControllerCurve::changeCurveType(const std::string& trackID, const std::string& segmentID, math::ECurveInterp type)
	{
		static std::unordered_map<rttr::type, void(SequenceControllerCurve::*)(SequenceTrackSegment&, math::ECurveInterp type)> changeCurveTypeMap
		{
			{ RTTI_OF(SequenceTrackSegmentCurveFloat), &SequenceControllerCurve::changeCurveType<float> },
			{ RTTI_OF(SequenceTrackSegmentCurveVec2), &SequenceControllerCurve::changeCurveType<glm::vec2> },
			{ RTTI_OF(SequenceTrackSegmentCurveVec3), &SequenceControllerCurve::changeCurveType<glm::vec3> },
			{ RTTI_OF(SequenceTrackSegmentCurveVec4), &SequenceControllerCurve::changeCurveType<glm::vec4> },
		};

		auto l = lock();

		auto* segment = findSegment(trackID, segmentID);
		assert(segment != nullptr); // segment not found

		if (segment != nullptr)
		{
			auto it = changeCurveTypeMap.find(segment->get_type());
			assert(it != changeCurveTypeMap.end()); // type not found
			if (it != changeCurveTypeMap.end())
			{
				(*this.*it->second)(*segment, type);
			}
		}
	}


	template<typename T>
	void SequenceControllerCurve::changeCurveType(SequenceTrackSegment& segment, math::ECurveInterp type)
	{
		auto* segmentCurve = dynamic_cast<SequenceTrackSegmentCurve<T>*>(&segment);
		assert(segmentCurve != nullptr); // type mismatch

		if (segmentCurve != nullptr)
		{
			segmentCurve->mCurveType = type;
			for (int i = 0; i < segmentCurve->mCurves.size(); i++)
			{
				for (int j = 0; j < segmentCurve->mCurves[i]->mPoints.size(); j++)
				{
					segmentCurve->mCurves[i]->mPoints[j].mInterp = type;
				}
			}
		}
	}


	void SequenceControllerCurve::changeCurveSegmentValue(const std::string& trackID, const std::string& segmentID, float amount, int curveIndex,
														  SequenceCurveEnums::SegmentValueTypes valueType)
	{
		static std::unordered_map<rttr::type, void(SequenceControllerCurve::*)(SequenceTrack&, SequenceTrackSegment& segment, float, int, SequenceCurveEnums::SegmentValueTypes)> changeSegmentValueMap
		{
			{ RTTI_OF(SequenceTrackSegmentCurveFloat), &SequenceControllerCurve::changeCurveSegmentValue<float> },
			{ RTTI_OF(SequenceTrackSegmentCurveVec2), &SequenceControllerCurve::changeCurveSegmentValue<glm::vec2> },
			{ RTTI_OF(SequenceTrackSegmentCurveVec3), &SequenceControllerCurve::changeCurveSegmentValue<glm::vec3> },
			{ RTTI_OF(SequenceTrackSegmentCurveVec4), &SequenceControllerCurve::changeCurveSegmentValue<glm::vec4> }
		};

		//
		std::unique_lock<std::mutex> l = lock();

		SequenceTrack* track = findTrack(trackID);
		assert(track != nullptr); // track not found

		if (track != nullptr)
		{
			SequenceTrackSegment* segment = findSegment(trackID, segmentID);
			assert(segment != nullptr); // segment not found

			if (segment != nullptr)
			{
				auto it = changeSegmentValueMap.find(segment->get_type());
				assert(it != changeSegmentValueMap.end()); // type not found
				if (it != changeSegmentValueMap.end())
				{
					(*this.*it->second)(*track, *segment, amount, curveIndex, valueType);
				}
			}
		}
	}


	template<typename T>
	void SequenceControllerCurve::changeCurveSegmentValue(SequenceTrack& track, SequenceTrackSegment& segment, float amount, int curveIndex,
														  SequenceCurveEnums::SegmentValueTypes valueType)
	{
		assert(segment.get_type().is_derived_from<SequenceTrackSegmentCurve<T>>()); // type mismatch
		SequenceTrackSegmentCurve<T>& curveSegment = static_cast<SequenceTrackSegmentCurve<T>&>(segment);

		switch (valueType)
		{
		case SequenceCurveEnums::BEGIN:
		{
			curveSegment.mCurves[curveIndex]->mPoints[0].mPos.mValue += amount;
			curveSegment.mCurves[curveIndex]->mPoints[0].mPos.mValue = math::clamp<float>(curveSegment.mCurves[curveIndex]->mPoints[0].mPos.mValue, 0.0f, 1.0f);
		}
		break;
		case SequenceCurveEnums::END:
		{
			int lastPoint = curveSegment.mCurves[curveIndex]->mPoints.size() - 1;
			curveSegment.mCurves[curveIndex]->mPoints[lastPoint].mPos.mValue += amount;
			curveSegment.mCurves[curveIndex]->mPoints[lastPoint].mPos.mValue = math::clamp<float>(curveSegment.mCurves[curveIndex]->mPoints[lastPoint].mPos.mValue, 0.0f, 1.0f);
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
		ResourcePtr<SequenceTrackSegmentCurve<T>> previousSegment = nullptr;
		for (auto trackSegment : track.mSegments)
		{
			if (previousSegment == nullptr)
			{
				trackSegment->mStartTime = 0.0;
			}
			else
			{
				trackSegment->mStartTime = previousSegment->mStartTime + previousSegment->mDuration;
				previousSegment->mDuration = trackSegment->mStartTime - previousSegment->mStartTime;
			}
			previousSegment = static_cast<ResourcePtr<SequenceTrackSegmentCurve<T>>>(trackSegment);
		}

		// 
		previousSegment = nullptr;
		for (auto trackSeg : track.mSegments)
		{
			auto& curveSegment = static_cast<SequenceTrackSegmentCurve<T>&>(*trackSeg.get());

			if (previousSegment == nullptr)
			{
			}
			else
			{
				// if we have a previous segment, the curve gets the value of the start value of the current segment
				curveSegment.setStartValue(previousSegment->getEndValue());
			}
			previousSegment = &curveSegment;
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
		std::unique_lock<std::mutex> l = lock();

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
	}


	template <typename T>
	void SequenceControllerCurve::insertCurvePoint(SequenceTrackSegment& segment, float pos, int curveIndex)
	{
		assert(segment.get_type().is_derived_from<SequenceTrackSegmentCurve<T>>()); // type mismatch

		auto& curveSegment = static_cast<SequenceTrackSegmentCurve<T>&>(segment);
		assert(curveIndex < curveSegment.mCurves.size()); // invalid curve index

		// iterate trough points of curve
		for (int i = 0; i < curveSegment.mCurves[curveIndex]->mPoints.size() - 1; i++)
		{
			// find the point the new point needs to get inserted after
			if (curveSegment.mCurves[curveIndex]->mPoints[i].mPos.mTime <= pos
				&& curveSegment.mCurves[curveIndex]->mPoints[i + 1].mPos.mTime > pos)
			{
				// create point
				math::FCurvePoint<float, float> p;
				p.mPos.mTime = pos;
				p.mPos.mValue = curveSegment.mCurves[curveIndex]->evaluate(pos);
				p.mInTan.mTime = -0.1f;
				p.mOutTan.mTime = 0.1f;
				p.mInTan.mValue = 0.0f;
				p.mOutTan.mValue = 0.0f;
				p.mTangentsAligned = true;
				p.mInterp = curveSegment.mCurveType;

				// insert point
				curveSegment.mCurves[curveIndex]->mPoints.insert(curveSegment.mCurves[curveIndex]->mPoints.begin() + i + 1, p);
				curveSegment.mCurves[curveIndex]->invalidate();
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

		std::unique_lock<std::mutex> l = lock();

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
	}


	template<typename T>
	void SequenceControllerCurve::deleteCurvePoint(SequenceTrackSegment& segment, const int index, int curveIndex)
	{
		assert(segment.get_type().is_derived_from<SequenceTrackSegmentCurve<T>>()); // type mismatch

		//
		auto& curveSegment = static_cast<SequenceTrackSegmentCurve<T>&>(segment);
		assert(curveIndex < curveSegment.mCurves.size()); // invalid curve index

		if (index < curveSegment.mCurves[curveIndex]->mPoints.size())
		{
			//
			curveSegment.mCurves[curveIndex]->mPoints.erase(curveSegment.mCurves[curveIndex]->mPoints.begin() + index);
			curveSegment.mCurves[curveIndex]->invalidate();
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
		static std::unordered_map<rttr::type, void(SequenceControllerCurve::*)(SequenceTrackSegment&, const int, const int, float, float)> changeCurvePointMap
		{
			{ RTTI_OF(SequenceTrackSegmentCurveFloat), &SequenceControllerCurve::changeCurvePoint<float> },
			{ RTTI_OF(SequenceTrackSegmentCurveVec2), &SequenceControllerCurve::changeCurvePoint<glm::vec2> },
			{ RTTI_OF(SequenceTrackSegmentCurveVec3), &SequenceControllerCurve::changeCurvePoint<glm::vec3> },
			{ RTTI_OF(SequenceTrackSegmentCurveVec4), &SequenceControllerCurve::changeCurvePoint<glm::vec4> },
		};

		std::unique_lock<std::mutex> l = lock();

		// find segment
		SequenceTrackSegment* segment = findSegment(trackID, segmentID);
		assert(segment != nullptr); // segment not found

		if (segment != nullptr)
		{
			auto it = changeCurvePointMap.find(segment->get_type());
			assert(it != changeCurvePointMap.end()); // type not found
			if (it != changeCurvePointMap.end())
			{
				(*this.*it->second)(*segment, pointIndex, curveIndex, time, value);
			}
		}
	}


	template <typename T>
	void SequenceControllerCurve::changeCurvePoint(SequenceTrackSegment& segment, const int pointIndex, const int curveIndex, float time, float value)
	{
		//
		assert(segment.get_type().is_derived_from<SequenceTrackSegmentCurve<T>>()); // type mismatch
		auto& curveSegment = static_cast<SequenceTrackSegmentCurve<T>&>(segment);;
		assert(curveIndex < curveSegment.mCurves.size()); // invalid curve index
		assert(pointIndex < curveSegment.mCurves[curveIndex]->mPoints.size()); // invalid point index

		//
		math::FCurvePoint<float, float>& curvePoint = curveSegment.mCurves[curveIndex]->mPoints[pointIndex];
		curvePoint.mPos.mTime += time;
		curvePoint.mPos.mValue += value;
		curvePoint.mPos.mValue = math::clamp<float>(curvePoint.mPos.mValue, 0.0f, 1.0f);
		curveSegment.mCurves[curveIndex]->invalidate();
	}


	void SequenceControllerCurve::changeTanPoint(const std::string& trackID, const std::string& segmentID, const int pointIndex, const int curveIndex,
												 SequenceCurveEnums::TanPointTypes tanType, float time, float value)
	{
		//
		static std::unordered_map<rttr::type, void(SequenceControllerCurve::*)(SequenceTrackSegment&, const std::string&, const int, const int,
												  SequenceCurveEnums::TanPointTypes, float, float)> changeCurvePointMap
		{
			{ RTTI_OF(SequenceTrackSegmentCurveFloat), &SequenceControllerCurve::changeTanPoint<float> },
			{ RTTI_OF(SequenceTrackSegmentCurveVec2), &SequenceControllerCurve::changeTanPoint<glm::vec2> },
			{ RTTI_OF(SequenceTrackSegmentCurveVec3), &SequenceControllerCurve::changeTanPoint<glm::vec3> },
			{ RTTI_OF(SequenceTrackSegmentCurveVec4), &SequenceControllerCurve::changeTanPoint<glm::vec4> },
		};


		//
		std::unique_lock<std::mutex> l = lock();

		// find segment
		SequenceTrackSegment* segment = findSegment(trackID, segmentID);
		assert(segment != nullptr); // segment not found

		if (segment != nullptr)
		{
			auto it = changeCurvePointMap.find(segment->get_type());
			assert(it != changeCurvePointMap.end()); // type not found
			if (it != changeCurvePointMap.end())
			{
				(*this.*it->second)(*segment, trackID, pointIndex, curveIndex, tanType, time, value);
			}
		}
	}


	template<typename T>
	void SequenceControllerCurve::changeTanPoint(SequenceTrackSegment& segment, const std::string& trackID, const int pointIndex, const int curveIndex,
												 SequenceCurveEnums::TanPointTypes tanType, float time, float value)
	{
		assert(segment.get_type().is_derived_from<SequenceTrackSegmentCurve<T>>()); // type mismatch
		auto& curveSegment = static_cast<SequenceTrackSegmentCurve<T>&>(segment);;
		assert(curveIndex < curveSegment.mCurves.size()); // invalid curve index
		assert(pointIndex < curveSegment.mCurves[curveIndex]->mPoints.size()); // invalid point index

		//
		auto& curvePoint = curveSegment.mCurves[curveIndex]->mPoints[pointIndex];
		switch (tanType)
		{
		case SequenceCurveEnums::TanPointTypes::IN:
		{
			if (curvePoint.mInTan.mTime + time < curvePoint.mOutTan.mTime)
			{
				curvePoint.mInTan.mTime += time;
				curvePoint.mInTan.mValue += value;

				if (curvePoint.mTangentsAligned)
				{
					curvePoint.mOutTan.mTime = -curvePoint.mInTan.mTime;
					curvePoint.mOutTan.mValue = -curvePoint.mInTan.mValue;
				}
			}
		}
		break;
		case SequenceCurveEnums::TanPointTypes::OUT:
		{
			if (curvePoint.mOutTan.mTime + time > curvePoint.mInTan.mTime)
			{
				curvePoint.mOutTan.mTime += time;
				curvePoint.mOutTan.mValue += value;

				if (curvePoint.mTangentsAligned)
				{
					curvePoint.mInTan.mTime = -curvePoint.mOutTan.mTime;
					curvePoint.mInTan.mValue = -curvePoint.mOutTan.mValue;
				}
			}

		}
		break;
		}

		// is this the last control point ?
		// then also change the first control point of the next segment accordinly
		if (pointIndex == curveSegment.mCurves[curveIndex]->mPoints.size() - 1)
		{
			SequenceTrack* track = findTrack(trackID);
			assert(track != nullptr); // track not found

			for (int i = 0; i < track->mSegments.size(); i++)
			{
				if (track->mSegments[i].get() == &segment &&
					i + 1 < track->mSegments.size())
				{
					auto& nextSegmentCurvePoint = static_cast<SequenceTrackSegmentCurve<T>&>(*track->mSegments[i + 1]).mCurves[curveIndex]->mPoints[0];

					nextSegmentCurvePoint.mInTan.mTime = curvePoint.mInTan.mTime;
					nextSegmentCurvePoint.mInTan.mValue = curvePoint.mInTan.mValue;
					nextSegmentCurvePoint.mOutTan.mTime = curvePoint.mOutTan.mTime;
					nextSegmentCurvePoint.mOutTan.mValue = curvePoint.mOutTan.mValue;

				}
			}
		}

		curveSegment.mCurves[curveIndex]->invalidate();
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


	template<typename T>
	void SequenceControllerCurve::changeMinMaxCurveTrack(const std::string& trackID, T minimum, T maximum)
	{
		auto l = lock();

		SequenceTrack* track = findTrack(trackID);
		assert(track != nullptr); // track not found

		SequenceTrackCurve<T>* trackCurve = static_cast<SequenceTrackCurve<T>*>(track);
		trackCurve->mMinimum = minimum;
		trackCurve->mMaximum = maximum;
	}


	// explicit template declarations
	template NAPAPI void SequenceControllerCurve::addNewCurveTrack<float>();
	template NAPAPI void SequenceControllerCurve::addNewCurveTrack<glm::vec2>();
	template NAPAPI void SequenceControllerCurve::addNewCurveTrack<glm::vec3>();
	template NAPAPI void SequenceControllerCurve::addNewCurveTrack<glm::vec4>();

	template NAPAPI void SequenceControllerCurve::insertCurveSegment<float>(const std::string& trackID, double time);
	template NAPAPI void SequenceControllerCurve::insertCurveSegment<glm::vec2>(const std::string& trackID, double time);
	template NAPAPI void SequenceControllerCurve::insertCurveSegment<glm::vec3>(const std::string& trackID, double time);
	template NAPAPI void SequenceControllerCurve::insertCurveSegment<glm::vec4>(const std::string& trackID, double time);

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

	template NAPAPI void SequenceControllerCurve::changeCurveType<float>(SequenceTrackSegment& segment, math::ECurveInterp type);
	template NAPAPI void SequenceControllerCurve::changeCurveType<glm::vec2>(SequenceTrackSegment& segment, math::ECurveInterp type);
	template NAPAPI void SequenceControllerCurve::changeCurveType<glm::vec3>(SequenceTrackSegment& segment, math::ECurveInterp type);
	template NAPAPI void SequenceControllerCurve::changeCurveType<glm::vec4>(SequenceTrackSegment& segment, math::ECurveInterp type);
}
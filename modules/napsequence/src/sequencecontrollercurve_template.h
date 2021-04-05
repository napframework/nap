/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

namespace nap
{
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

								new_segment->mCurveTypes[i] = math::ECurveInterp::Bezier;

								// move ownership
								getPlayerOwnedObjects().emplace_back(std::move(segment_curve));
							}

							//
							auto& segment_curve_2 = *rtti_cast<SequenceTrackSegmentCurve<T>>(segment.get());

							// set the value by evaluation curve
							new_segment->setStartValue(segment_curve_2.getValue(
								(segment->mStartTime + segment->mDuration - time) / segment->mDuration)
							);

							// check if there is a next segment
							if (segment_count < track->mSegments.size())
							{
								// if there is a next segment, the new segments end value is the start value of the next segment ...
								auto& next_segment_curve = *rtti_cast<SequenceTrackSegmentCurve<T>>(track->mSegments[segment_count].get());

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
					if (track->mSegments.empty())
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


	template<typename T>
	void SequenceControllerCurve::changeCurveSegmentValue(SequenceTrack& track, SequenceTrackSegment& segment, float newValue, int curveIndex,
														  SequenceCurveEnums::SegmentValueTypes valueType)
	{
		assert(segment.get_type().is_derived_from<SequenceTrackSegmentCurve<T>>()); // type mismatch
		auto& curve_segment = *rtti_cast<SequenceTrackSegmentCurve<T>>(&segment);

		switch (valueType)
		{
		case SequenceCurveEnums::BEGIN:
		{
			curve_segment.mCurves[curveIndex]->mPoints[0].mPos.mValue = newValue;
			curve_segment.mCurves[curveIndex]->mPoints[0].mPos.mValue = math::clamp(curve_segment.mCurves[curveIndex]->mPoints[0].mPos.mValue, 0.0f, 1.0f);
		}
			break;
		case SequenceCurveEnums::END:
		{
			int lastPoint = curve_segment.mCurves[curveIndex]->mPoints.size() - 1;
			curve_segment.mCurves[curveIndex]->mPoints[lastPoint].mPos.mValue = newValue;
			curve_segment.mCurves[curveIndex]->mPoints[lastPoint].mPos.mValue = math::clamp(curve_segment.mCurves[curveIndex]->mPoints[lastPoint].mPos.mValue, 0.0f, 1.0f);
		}
			break;
		}

		//
		updateCurveSegments<T>(track);
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


	template <typename T>
	void SequenceControllerCurve::changeCurvePoint(SequenceTrackSegment& segment, const int pointIndex, const int curveIndex, float time, float value)
	{
		// obtain curve segment
		assert(segment.get_type().is_derived_from<SequenceTrackSegmentCurve<T>>()); // type mismatch
		auto& curve_segment = static_cast<SequenceTrackSegmentCurve<T>&>(segment);;
		assert(curveIndex < curve_segment.mCurves.size()); // invalid curve index
		assert(pointIndex < curve_segment.mCurves[curveIndex]->mPoints.size()); // invalid point index

		// obtain curve point and set values
		math::FCurvePoint<float, float>& curve_point = curve_segment.mCurves[curveIndex]->mPoints[pointIndex];
		curve_point.mPos.mTime = time;
		curve_point.mPos.mValue = value;
		curve_point.mPos.mValue = math::clamp(curve_point.mPos.mValue, 0.0f, 1.0f);
		curve_segment.mCurves[curveIndex]->invalidate();
	}


	template <typename T>
	void SequenceControllerCurve::changeLastCurvePoint(SequenceTrackSegment& segment, const int curveIndex, float time, float value)
	{

		// obtain curve segment
		assert(segment.get_type().is_derived_from<SequenceTrackSegmentCurve<T>>()); // type mismatch
		auto& curve_segment = static_cast<SequenceTrackSegmentCurve<T>&>(segment);;
		assert(curveIndex < curve_segment.mCurves.size()); // invalid curve index

		//
		int point_index = curve_segment.mCurves[curveIndex]->mPoints.size() - 1;

		// obtain curve point and set values
		math::FCurvePoint<float, float>& curve_point = curve_segment.mCurves[curveIndex]->mPoints[point_index];
		curve_point.mPos.mTime = time;
		curve_point.mPos.mValue = value;
		curve_point.mPos.mValue = math::clamp(curve_point.mPos.mValue, 0.0f, 1.0f);
		curve_segment.mCurves[curveIndex]->invalidate();
	}


	template <typename  T>
	void SequenceControllerCurve::changeTanPoint(SequenceTrackSegment& segment, const std::string& trackID, const int pointIndex, const int curveIndex, SequenceCurveEnums::ETanPointTypes tanType, float time, float value)
	{
		assert(segment.get_type().is_derived_from<SequenceTrackSegmentCurve<T>>()); // type mismatch
		auto& curve_segment = static_cast<SequenceTrackSegmentCurve<T>&>(segment);;
		assert(curveIndex < curve_segment.mCurves.size()); // invalid curve index
		assert(pointIndex < curve_segment.mCurves[curveIndex]->mPoints.size()); // invalid point index

		//
		auto& curve_point = curve_segment.mCurves[curveIndex]->mPoints[pointIndex];
		switch (tanType)
		{
		case SequenceCurveEnums::ETanPointTypes::IN:
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
		case SequenceCurveEnums::ETanPointTypes::OUT:
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
			auto& segment_curve = *rtti_cast<SequenceTrackSegmentCurve<T>>(track_segment.get());

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


	template<typename T>
	void SequenceControllerCurve::changeMinMaxCurveTrack(const std::string& trackID, T minimum, T maximum)
	{
		performEditAction([this, trackID, minimum, maximum]()
		{
			SequenceTrack* track = findTrack(trackID);
			assert(track != nullptr); // track not found

			auto* track_curve = static_cast<SequenceTrackCurve<T>*>(track);

			for(auto& segment : track_curve->mSegments)
			{
				auto& curve_segment = static_cast<SequenceTrackSegmentCurve<T>&>(*segment.get());
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
}
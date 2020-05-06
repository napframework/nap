#pragma once

#include "sequencecontroller.h"
#include "sequenceeditortypes.h"

namespace nap
{
	class NAPAPI SequenceControllerCurve : public SequenceController
	{
	public:
		SequenceControllerCurve(SequencePlayer & player) : SequenceController(player) { }

		/**
		 * segmentDurationChange
		 * @param trackID the id of the track
		 * @param segmentID the id of the segment we need to edit
		 * @param amount the amount that the duration of this segment should change
	 	 */
		void segmentDurationChange(const std::string& trackID, const std::string& segmentID, float amount);

		/**
		 * segmentEventStartTimeChange
		 * changes start time of segment
		 * @param trackID the trackID of track containing segment
		 * @param segmentID the segmentID
		 * @param amount the amount the starttime needs to change
		 */
		void segmentEventStartTimeChange(const std::string& trackID, const std::string& segmentID, float amount);
	
		/**
		 * addCurveTrack
		 * adds a new curve track of type T ( float, vec2, vec3, vec4 )
		 */
		template<typename T>
		void addNewCurveTrack();

		/**
		 * changeCurveSegmentValue
		 * changes start or end value of segment of type T
		 * @param trackID the track id
		 * @param segmentID id of segment
		 * @param amount the amount that the value needs to change
		 * @param curveIndex the curve index of the value
		 * @param valueType the segment value type ( first or last value )
		 */
		void changeCurveSegmentValue(const std::string& trackID, const std::string& segmentID, float amount, int curveIndex, SequenceEditorTypes::SegmentValueTypes valueType);
	
		/**
		 * insertCurvePoint
		 * insert point in curve of segment
		 * @param trackID the track id
		 * @param segmentID the segment id
		 * @param pos the position at which to insert the curvepoint in curve ( range 0-1 )
		 * @param curveIndex the index of the curve
		 */
		void insertCurvePoint(const std::string& trackID, const std::string& segmentID, float pos, int curveIndex);
	

		/**
		* deleteCurvePoint
		* deletes point from curve
		* @param trackID the trackID
		* @param segmentID the segmentID
		* @param index the point index
		* @param curveIndex the curveIndex
		*/
		void deleteCurvePoint(const std::string& trackID, const std::string& segmentID, const int index, int curveIndex);

		/**
		* changeCurvePoint
		* changes a curvepoint value and time / position
		* @param trackID the trackID
		* @param segmentID the segmentID
		* @param pointIndex the point index
		* @param curveIndex the curve index
		* @param time offset for new time
		* @param value offset for new value
		*/
		void changeCurvePoint(const std::string& trackID, const std::string& segmentID, const int pointIndex, const int curveIndex, float time, float value);

		/**
		* changeTanPoint
		* changes tangent of curve point. Tangents are always aligned
		* @param trackID the trackID
		* @param segmentID the segmentID
		* @param pointIndex the point index
		* @param curveIndex the curve index
		* @param tanType in or out tangent
		* @param time offset for new time
		* @param value offset for new value
		*/
		void changeTanPoint(const std::string& trackID, const std::string& segmentID, const int pointIndex, const int curveIndex, SequenceEditorTypes::TanPointTypes tanType, float time, float value);

		/**
		 * changeMinMaxCurveTrack
		 * changes minimum and maximum value of track
		 * @param trackID the trackID
		 * @param minimum new minimum
		 * @param maximum new maximum
		 */
		template<typename T>
		void changeMinMaxCurveTrack(const std::string& trackID, T minimum, T maximum);

		/**
		 * changeCurveType
		 * changes curvetype ( linear or bezier )
		 * @param trackID the trackID
		 * @param segmentID the segmentID
		 * @param curveType the new curve type
		 */
		void changeCurveType(const std::string& trackID, const std::string& segmentID, math::ECurveInterp type);

		virtual void insertSegment(const std::string& trackID, double time) override;

		virtual void deleteSegment(const std::string& trackID, const std::string& segmentID) override;
	
		virtual void insertTrack(rttr::type type) override;
	private:
		/**
		 * updateCurveSegments
		 * updates curve segments values to be continuous ( segment 1 end value == segment 2 start value etc )
		 * @param track reference to sequence track
		 */
		template<typename T>
		void updateCurveSegments(SequenceTrack& track);

		/**
		 * insertCurveSegment
		 * inserts a new curvesegment of type T ( vec2, vec3, vec4, float )
		 * @param trackID the track in which to insert the segment
		 * @param time the time at when to insert segment
		 */
		template <typename T>
		void insertCurveSegment(const std::string& trackID, double time);

		template <typename  T>
		void changeTanPoint(SequenceTrackSegment& segment, const std::string& trackID, const int pointIndex, const int curveIndex, SequenceEditorTypes::TanPointTypes tanType, float time, float value);

		template<typename T>
		void changeCurveType(SequenceTrackSegment& segment, math::ECurveInterp type);

		template <typename  T>
		void changeCurvePoint(SequenceTrackSegment& segment, const int pointIndex, const int curveIndex, float time, float value);

		template<typename T>
		void deleteCurvePoint(SequenceTrackSegment& segment, const int index, int curveIndex);

		template<typename T>
		void insertCurvePoint(SequenceTrackSegment& segment, float pos, int curveIndex);

		template<typename T>
		void changeCurveSegmentValue(SequenceTrack& track, SequenceTrackSegment& segment, float amount, int curveIndex, SequenceEditorTypes::SegmentValueTypes valueType);
	
		static std::unordered_map<rttr::type, void(SequenceControllerCurve::*)(SequenceTrack&)> sUpdateSegmentFunctionMap;
	};
}
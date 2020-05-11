#pragma once

#include "sequencetrackview.h"
#include "sequencetracksegmentcurve.h"

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	class SequenceCurveTrackView : public SequenceTrackView
	{
	public:
		SequenceCurveTrackView(SequenceEditorGUIView& view, SequenceEditorGUIState& state);

		virtual void drawTrack(const SequenceTrack& track) override;

		virtual void handlePopups() override;
	private:
		/**
		 * drawCurveTrack
		 * draws a track containing curves
		 * @tparam T the curve type ( float, glm::vec2, glm::vec3, glm::vec4 )
		 * @param track reference to track
		 * @param cursorPos the current IMGUI cursorposition
		 * @param marginBetweenTracks y margin between tracks
		 * @param sequencePlayer reference to player
		 * @param deleteTrack set to true when delete track button is pressed
		 * @param deleteTrackID the id of track that needs to be deleted
		 */
		template<typename T>
		void drawCurveTrack(const SequenceTrack &track, ImVec2 &cursorPos, const float marginBetweenTracks, const SequencePlayer &sequencePlayer, bool &deleteTrack, std::string &deleteTrackID);


		/**
		 * drawSegmentContent
		 * draws the contents of a segment
		 * @tparam T the type of this segment
		 * @param track reference to track
		 * @param segment reference to segment
		 * @param trackTopLeft topleft position of track
		 * @param previousSegmentX the x position of the previous segment
		 * @param segmentWidth the width of this segment
		 * @param segmentX the x position of this segment
		 * @param drawList pointer to drawlist of this track window
		 * @param drawStartValue should we draw the start value ? only used in first segment of track
		 */
		template<typename T>
		void drawSegmentContent(const SequenceTrack &track, const SequenceTrackSegment &segment, const ImVec2& trackTopLeft, float previousSegmentX, float segmentWidth, float segmentX, ImDrawList* drawList, bool drawStartValue);
	
		/**
		 * drawSegmentValue
		 * draws a segments value
		 * @tparam T type of segment
		 * @param track reference to track
		 * @param segment reference to segment
		 * @param trackTopLeft tracks topleft position
		 * @param segmentX segment x position
		 * @param segmentWidth width of segment
		 * @param segmentType type of segment
		 * @param drawList pointer to window drawlist
		 */
		template<typename T>
		void drawSegmentValue(const SequenceTrack& track, const SequenceTrackSegment& segment, const ImVec2 &trackTopLeft, const float segmentX, const float segmentWidth, const SequenceCurveEnums::SegmentValueTypes segmentType, ImDrawList* drawList);
	
		/**
		 * drawSegmentHandler
		 * draws segment handler
		 * @param track reference to track
		 * @param segment reference to segment
		 * @param trackTopLeft tracks topleft position
		 * @param segmentX segment x position
		 * @param segmentWidth width of segment
		 * @param drawList pointer to window drawlist
		 */
		void drawSegmentHandler(const SequenceTrack& track, const SequenceTrackSegment& segment, const ImVec2 &trackTopLeft, const float segmentX, const float segmentWidth, ImDrawList* drawList);
	
		/**
		 * drawControlPoints
		 * draws control points of curve segment
		 * @param track reference to track
		 * @param segment reference to segment
		 * @param trackTopLeft tracks topleft position
		 * @param segmentX segment x position
		 * @param segmentWidth width of segment
		 * @param drawList pointer to window drawlist
		 */
		template<typename T>
		void drawControlPoints(const SequenceTrack& track, const SequenceTrackSegment& segment, const ImVec2 &trackTopLeft, const float segmentX, const float segmentWidth, ImDrawList* drawList);

		/**
		 * drawCurves
		 * draws curves of segment
		 * @tparam T the type of this segment
		 * @param track reference to track
		 * @param segment reference to segment
		 * @param trackTopLeft topleft position of track
		 * @param previousSegmentX the x position of the previous segment
		 * @param segmentWidth the width of this segment
		 * @param segmentX the x position of this segment
		 * @param drawList pointer to drawlist of this track window
		 */
		template<typename T>
		void drawCurves(const SequenceTrack& track, const SequenceTrackSegment& segment, const ImVec2 &trackTopLeft, const float previousSegmentX, const float segmentWidth, const float segmentX, ImDrawList* drawList);

		/**
		 * drawTanHandler
		 * draws handlers of curve point
		 * @tparam T type of segment
		 * @param track reference to track
		 * @param segment reference to segment
		 * @param stringStream stringstream, used to keep track of object id
		 * @param segmentWidth width of segment
		 * @param curvePoint reference to curvePoint
		 * @param circlePoint circlePoint position
		 * @param controlPointIndex index of control point
		 * @param curveIndex index of curve
		 * @param type tangent type ( in or out )
		 * @param drawList pointer to window drawlist
		 */
		template<typename T>
		void drawTanHandler(const SequenceTrack &track, const SequenceTrackSegment &segment, std::ostringstream &stringStream, const float segmentWidth, const math::FCurvePoint<float, float> &curvePoint, const ImVec2 &circlePoint, const int controlPointIndex, const int curveIndex, const SequenceCurveEnums::TanPointTypes type, ImDrawList* drawList);
	
		/**
		 * handleInsertSegmentPopup
		 * handles insert segment popup
		 */
		void handleInsertSegmentPopup();

		/**
		 * handleDeleteSegmentPopup
		 * handles delete segment popup
		 */
		void handleDeleteSegmentPopup();

		/**
		 * handleInsertCurvePointPopup
		 * handles insert curve point popup
		 */
		void handleInsertCurvePointPopup();

		/**
		 * handleCurvePointActionPopup
		 * handles curvepoint action popup
		 */
		void handleCurvePointActionPopup();

		/**
		 * handleCurveTypePopup
		 * handles curve type popup
		 */
		void handleCurveTypePopup();

		/**
		 * drawInspectorRange
		 * Draws min/max range of inspector
		 * @tparam T type
		 * @param track reference to track
		 */
		template<typename T>
		void drawInspectorRange(const SequenceTrack& track);

		/**
		 * showValue
		 * @tparam T type of value
		 * @param track reference to track
		 * @param segment reference to segment
		 * @param x value to display
		 * @param time time in segment
		 * @param curveIndex curve index
		 */
		template<typename T>
		void showValue(const SequenceTrack& track, const SequenceTrackSegmentCurve<T>& segment, float x, double time, int curveIndex);
	
		/**
		 * inputFloat
		 * input float that takes type T as input
		 * @tparam T type of inputFloat
		 * @param precision decimal precision
		 * @return true if dragged
		 */
		template<typename T>
		bool inputFloat(T &, int precision);

		std::unordered_map<std::string, std::vector<std::vector<ImVec2>>> mCurveCache;

		using DrawTrackMemFunPtr = void(SequenceCurveTrackView::*)(const SequenceTrack &track, ImVec2 &cursorPos, const float marginBetweenTracks, const SequencePlayer &sequencePlayer, bool &deleteTrack, std::string &deleteTrackID);
		using DrawSegmentMemFunPtr = void(SequenceCurveTrackView::*)(const SequenceTrack &track, const SequenceTrackSegment &segment, const ImVec2& trackTopLeft, float previousSegmentX, float segmentWidth, float segmentX, ImDrawList* drawList, bool drawStartValue);
	
		static std::unordered_map<rttr::type, DrawTrackMemFunPtr> sDrawTracksMap;

		static std::unordered_map<rttr::type, DrawSegmentMemFunPtr> sDrawCurveSegmentsMap;
	};

	namespace SequenceGUIActions
	{
		class HoveringControlPoint : public Action
		{
			RTTI_ENABLE(Action)
		public:
			HoveringControlPoint(std::string trackID, std::string segmentID, int controlPointIndex, int curveIndex)
				: mTrackID(trackID), mSegmentID(segmentID), mControlPointIndex(controlPointIndex), mCurveIndex(curveIndex)
			{

			}

			std::string mTrackID;
			std::string mSegmentID;
			int mControlPointIndex;
			int mCurveIndex;
		};

		class DraggingControlPoint : public Action
		{
			RTTI_ENABLE(Action)
		public:
			DraggingControlPoint(std::string trackID, std::string segmentID, int controlPointIndex, int curveIndex)
				: mTrackID(trackID), mSegmentID(segmentID), mControlPointIndex(controlPointIndex), mCurveIndex(curveIndex)
			{

			}

			std::string mTrackID;
			std::string mSegmentID;
			int mControlPointIndex;
			int mCurveIndex;
		};

		class HoveringTanPoint : public Action
		{
			RTTI_ENABLE(Action)
		public:
			HoveringTanPoint(std::string tanPointID) : mTanPointID(tanPointID) {}
			std::string mTanPointID;
		};

		class HoveringCurve : public Action
		{
			RTTI_ENABLE(Action)
		public:
			HoveringCurve(std::string trackId, std::string segmentID, int curveIndex)
				: mTrackID(trackId), mSegmentID(segmentID), mCurveIndex(curveIndex) {}
			std::string mTrackID;
			std::string mSegmentID;
			int mCurveIndex;
		};

		class OpenInsertCurvePointPopup : public Action
		{
			RTTI_ENABLE(Action)
		public:
			OpenInsertCurvePointPopup(std::string trackID, std::string segmentID, int selectedCurve, float pos)
				: mTrackID(trackID), mSegmentID(segmentID), mSelectedIndex(selectedCurve), mPos(pos) {}

			std::string mTrackID;
			std::string mSegmentID;
			int mSelectedIndex;
			float mPos;
		};

		class InsertingCurvePoint : public Action
		{
			RTTI_ENABLE(Action)
		public:
			InsertingCurvePoint(std::string trackID, std::string segmentID, int selectedCurve, float pos)
				: mTrackID(trackID), mSegmentID(segmentID), mSelectedIndex(selectedCurve), mPos(pos) {}

			std::string mTrackID;
			std::string mSegmentID;
			int mSelectedIndex;
			float mPos;
		};

		class OpenCurvePointActionPopup : public Action
		{
			RTTI_ENABLE(Action)
		public:
			OpenCurvePointActionPopup(std::string trackID, std::string segmentID, int controlPointIndex, int curveIndex)
				: mTrackID(trackID), mSegmentID(segmentID), mControlPointIndex(controlPointIndex), mCurveIndex(curveIndex)
			{

			}

			std::string mTrackID;
			std::string mSegmentID;
			int mControlPointIndex;
			int mCurveIndex;
		};

		class CurvePointActionPopup : public Action
		{
			RTTI_ENABLE(Action)
		public:
			CurvePointActionPopup(std::string trackID, std::string segmentID, int controlPointIndex, int curveIndex)
				: mTrackID(trackID), mSegmentID(segmentID), mControlPointIndex(controlPointIndex), mCurveIndex(curveIndex)
			{

			}

			std::string mTrackID;
			std::string mSegmentID;
			int mControlPointIndex;
			int mCurveIndex;
		};

		class OpenCurveTypePopup : public Action
		{
			RTTI_ENABLE(Action)
		public:
			OpenCurveTypePopup(std::string trackID, std::string segmentID, int index, float pos, ImVec2 windowPos) :
				mTrackID(trackID),
				mSegmentID(segmentID),
				mCurveIndex(index),
				mPos(pos),
				mWindowPos(windowPos) {}

			std::string mTrackID;
			std::string mSegmentID;
			int mCurveIndex;
			float mPos;
			ImVec2 mWindowPos;
		};

		class CurveTypePopup : public Action
		{
			RTTI_ENABLE(Action)
		public:
			CurveTypePopup(std::string trackID, std::string segmentID, int index, float pos, ImVec2 windowPos) :
				mTrackID(trackID), mSegmentID(segmentID), mCurveIndex(index), mPos(pos), mWindowPos(windowPos) {}

			std::string mTrackID;
			std::string mSegmentID;
			int mCurveIndex;
			float mPos;
			ImVec2 mWindowPos;
		};

		class DraggingTanPoint : public Action
		{
			RTTI_ENABLE(Action)
		public:
			DraggingTanPoint(std::string trackId, std::string segmentID, int controlPointIndex, int curveIndex,
							 SequenceCurveEnums::TanPointTypes type)
				: mTrackID(trackId), mSegmentID(segmentID), mControlPointIndex(controlPointIndex), mCurveIndex(curveIndex), mType(type) {}

			std::string mTrackID;
			std::string mSegmentID;
			int mControlPointIndex;
			SequenceCurveEnums::TanPointTypes mType;
			int mCurveIndex;
		};

		class OpenEditCurveSegmentPopup : public Action
		{
			RTTI_ENABLE(Action)
		public:
			OpenEditCurveSegmentPopup(std::string trackID, std::string segmentID, rttr::type segmentType)
				: mTrackID(trackID), mSegmentID(segmentID), mSegmentType(segmentType)
			{

			}

			std::string mTrackID;
			std::string mSegmentID;
			rttr::type mSegmentType;
		};

		class EditingCurveSegment : public Action {
			RTTI_ENABLE(Action)
		public:
			EditingCurveSegment(std::string trackID, std::string segmentID, rttr::type segmentType)
				: mTrackID(trackID), mSegmentID(segmentID), mSegmentType(segmentType)
			{

			}

			std::string mTrackID;
			std::string mSegmentID;
			rttr::type mSegmentType;
		};
	}

}

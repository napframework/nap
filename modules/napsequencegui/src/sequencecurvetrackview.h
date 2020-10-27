/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "sequencetrackview.h"
#include "sequencetracksegmentcurve.h"

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	/**
	 * SequenceCurveTrackView shows and handles contents for curve tracks
	 */
	class SequenceCurveTrackView : public SequenceTrackView
	{
	public:
		/**
		 * Constructor
		 * @param view reference to editor view
		 * @param state reference to editor state
		 */
		SequenceCurveTrackView(SequenceEditorGUIView& view, SequenceEditorGUIState& state);

		/**
		 * Handles popups
		 */
		virtual void handlePopups() override;

		/**
		 * Handles any actions
		 */
		virtual void handleActions() override;
	private:
		/**
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
		 * handles insert segment popup
		 */
		void handleInsertSegmentPopup();

		/**
		 * handles delete segment popup
		 */
		void handleDeleteSegmentPopup();

		/**
		 * handles insert curve point popup
		 */
		void handleInsertCurvePointPopup();

		/**
		 * handles curvepoint action popup
		 */
		template<typename T>
		void handleCurvePointActionPopup();

		/**
		 * handles segment value actions
		 */
		template<typename T>
		void handleSegmentValueActionPopup();

		/**
		 * handles curve type popup
		 */
		void handleCurveTypePopup();

		/**
		 * handles tanpoint action popup
		 */
		void handleTanPointActionPopup();

		/**
		 * Draws min/max range of inspector
		 * @tparam T type
		 * @param track reference to track
		 */
		template<typename T>
		void drawInspectorRange(const SequenceTrack& track);

		/**
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
		 * input float that takes type T as input
		 * @tparam T type of inputFloat
		 * @param precision decimal precision
		 * @return true if dragged
		 */
		template<typename T>
		bool inputFloat(T &, int precision);

		/**
		 * show inspector content
		 * @param track reference to track
		 */
		virtual void showInspectorContent(const SequenceTrack &track) override;

		/**
		 * shows track content
		 * @param track reference to track
		 * @param trackTopLeft orientation
		 */
		virtual void showTrackContent(const SequenceTrack &track, const ImVec2& trackTopLeft) override;

		// curve cache holds evaluated curves, needs to be cleared when view changes and curves need to be redrawn
		std::unordered_map<std::string, std::vector<std::vector<ImVec2>>> mCurveCache;

		// short curt to member function drawSegmentContent
		using DrawSegmentMemFunPtr = void(SequenceCurveTrackView::*)(const SequenceTrack &track, const SequenceTrackSegment &segment, const ImVec2& trackTopLeft, float previousSegmentX, float segmentWidth, float segmentX, ImDrawList* drawList, bool drawStartValue);

		// static map of member function pointers
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

		template<typename T>
		class OpenCurvePointActionPopup : public Action
		{
			RTTI_ENABLE(Action)
		public:
			OpenCurvePointActionPopup(std::string trackID, std::string segmentID, int controlPointIndex, int curveIndex, float value, float time, T minimum, T maximum)
				: mTrackID(trackID), mSegmentID(segmentID), mControlPointIndex(controlPointIndex), mCurveIndex(curveIndex), mValue(value), mTime(time), mMinimum(minimum), mMaximum(maximum)
			{

			}

			std::string mTrackID;
			std::string mSegmentID;
			int mControlPointIndex;
			int mCurveIndex;
			float mValue;
			float mTime;
            T mMinimum;
			T mMaximum;
		};

		template<typename T>
		class CurvePointActionPopup : public Action
		{
			RTTI_ENABLE(Action)
		public:
			CurvePointActionPopup(std::string trackID, std::string segmentID, int controlPointIndex, int curveIndex, float value, float time, T minimum, T maximum)
				: mTrackID(trackID), mSegmentID(segmentID), 
				mControlPointIndex(controlPointIndex), mCurveIndex(curveIndex),
				mValue(value), mMinimum(minimum), mMaximum(maximum), mTime(time)
			{

			}

			std::string mTrackID;
			std::string mSegmentID;
			int mControlPointIndex;
			int mCurveIndex;
			float mValue;
            T mMinimum;
			T mMaximum;
            float mTime;
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
			DraggingTanPoint(std::string trackID, std::string segmentID, int controlPointIndex, int curveIndex, SequenceCurveEnums::TanPointTypes type)
				: mTrackID(trackID), mSegmentID(segmentID), mControlPointIndex(controlPointIndex), mCurveIndex(curveIndex), mType(type) {}

			std::string mTrackID;
			std::string mSegmentID;
			int mControlPointIndex;
            int mCurveIndex;
			SequenceCurveEnums::TanPointTypes mType;
		};

		class OpenEditTanPointPopup : public Action
		{
			RTTI_ENABLE(Action)
		public:
			OpenEditTanPointPopup(std::string trackID, std::string segmentID, int controlPointIndex, int curveIndex, SequenceCurveEnums::TanPointTypes type, float value, float time)
				: mTrackID(trackID), mSegmentID(segmentID), mControlPointIndex(controlPointIndex), mCurveIndex(curveIndex), mType(type), mValue(value), mTime(time) {}

			std::string mTrackID;
			std::string mSegmentID;
			int mControlPointIndex;
			int mCurveIndex;
			SequenceCurveEnums::TanPointTypes mType;
			float mValue;
			float mTime;
		};

		class EditingTanPointPopup : public Action
		{
			RTTI_ENABLE(Action)
		public:
			EditingTanPointPopup(std::string trackID, std::string segmentID, int controlPointIndex, int curveIndex, SequenceCurveEnums::TanPointTypes type, float value, float time)
				: mTrackID(trackID), mSegmentID(segmentID), mControlPointIndex(controlPointIndex), mCurveIndex(curveIndex), mType(type), mValue(value), mTime(time){}

			std::string mTrackID;
			std::string mSegmentID;
			int mControlPointIndex;
			int mCurveIndex;
			SequenceCurveEnums::TanPointTypes mType;
			float mValue;
			float mTime;
		};

		class OpenEditCurveSegmentPopup : public Action
		{
			RTTI_ENABLE(Action)
		public:
			OpenEditCurveSegmentPopup(std::string trackID, std::string segmentID, rttr::type segmentType, double startTime, double duration)
				: mTrackID(trackID), mSegmentID(segmentID), mSegmentType(segmentType), mStartTime(startTime), mDuration(duration)
			{

			}

			std::string mTrackID;
			std::string mSegmentID;
			rttr::type mSegmentType;
			double mStartTime;
			double mDuration;
		};

		class EditingCurveSegment : public Action {
			RTTI_ENABLE(Action)
		public:
			EditingCurveSegment(std::string trackID, std::string segmentID, rttr::type segmentType, double startTime, double duration)
				: mTrackID(trackID), mSegmentID(segmentID), mSegmentType(segmentType), mStartTime(startTime), mDuration(duration)
			{

			}

			std::string mTrackID;
			std::string mSegmentID;
			rttr::type mSegmentType;
			double mStartTime;
			double mDuration;
		};

		template<typename T>
		class OpenEditSegmentCurveValuePopup :
			public Action
		{
			RTTI_ENABLE(Action)
		public:
			OpenEditSegmentCurveValuePopup(
				std::string trackId,
				std::string segmentID,
				SequenceCurveEnums::SegmentValueTypes type,
				int curveIndex,
				T value,
				T minimum,
				T maximum) :
				mTrackID(trackId),
				mSegmentID(segmentID),
				mType(type),
				mCurveIndex(curveIndex),
				mValue(value),
				mMinimum(minimum),
				mMaximum(maximum) {}

			std::string mTrackID;
			std::string mSegmentID;
			SequenceCurveEnums::SegmentValueTypes mType;
			int mCurveIndex;
			T mValue;
			T mMinimum;
			T mMaximum;
		};

		template<typename T>
		class EditingSegmentCurveValue :
			public Action
		{
			RTTI_ENABLE(Action)
		public:
			EditingSegmentCurveValue(std::string trackId, std::string segmentID, SequenceCurveEnums::SegmentValueTypes type, int curveIndex, T value, T minimum, T maximum)
				: mTrackID(trackId), mSegmentID(segmentID), mType(type), mCurveIndex(curveIndex), mValue(value), mMinimum(minimum), mMaximum(maximum) {}

			std::string mTrackID;
			std::string mSegmentID;
			SequenceCurveEnums::SegmentValueTypes mType;
			int mCurveIndex;
			T mValue;
			T mMinimum;
			T mMaximum;
		};
	}

	namespace SequenceGUIClipboards
	{
		class CurveSegmentClipboard :
			public Clipboard
		{
			RTTI_ENABLE(Clipboard)
		public:
			CurveSegmentClipboard(rttr::type segmentType) : mSegmentType(segmentType){}
			rttr::type mSegmentType;
		};
	}

	template<>
	void SequenceCurveTrackView::handleCurvePointActionPopup<float>();

	template<>
	void SequenceCurveTrackView::handleSegmentValueActionPopup<float>();
}

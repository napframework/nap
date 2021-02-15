/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "sequencetrackview.h"
#include "sequencetracksegmentcurve.h"
#include "sequencecontrollercurve.h"
#include "napcolors.h"

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	/**
	 * SequenceCurveTrackView shows and handles contents for curve tracks
	 */
	class NAPAPI SequenceCurveTrackView : public SequenceTrackView
	{
		RTTI_ENABLE(SequenceTrackView)
	public:
		/**
		 * Constructor
		 * @param view reference to editor view
		 * @param state reference to editor state
		 */
		SequenceCurveTrackView(SequenceEditorGUIView& view, SequenceEditorGUIState& state);

		/**
		 * Handles any actions
		 */
		virtual void handleActions() override;

	protected:
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
		virtual void drawSegmentHandler(const SequenceTrack& track, const SequenceTrackSegment& segment, const ImVec2 &trackTopLeft, const float segmentX, const float segmentWidth, ImDrawList* drawList);
	
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
		virtual void handleInsertSegmentPopup();

		/**
		 * handles delete segment popup
		 */
		virtual void handleEditSegmentPopup();

		/**
		 * handles insert curve point popup
		 */
		virtual void handleInsertCurvePointPopup();

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
		virtual void handleCurveTypePopup();

		/**
		 * handles tanpoint action popup
		 */
		virtual void handleTanPointActionPopup();

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

		/**
		 * pastes current clipboard as new segments at given time
		 * @tparam T the segment type
		 * @param trackId the track id of the track to insert into
		 * @param time the time at which to create new segment
		 */
		template<typename T>
		void pasteClipboardSegments(const std::string& trackId, double time);

		/**
		 * pastes content of clipboard segment into another segment
		 * @tparam T the segment type
		 * @param trackId the track id of the track to insert into
		 * @param segmentID the segment id of the segment to replace
		 */
		template<typename T>
		void pasteClipboardSegmentInto(const std::string& trackID, const std::string& segmentID);

		/**
		 * updates segment in clipboard with contents of segment in track
		 * @param trackID the track id of the segment in clipboard
		 * @param segmentID the segment id of the segment in clipboard
		 */
		void updateSegmentInClipboard(const std::string& trackID, const std::string& segmentID);

		/**
		 * iterates of all segments in clipboard of specified track and updates them with the ones in the specified track
		 * @param trackID the track id containing segments in clipboard
		 */
		void updateSegmentsInClipboard(const std::string& trackID);

		// curve cache holds evaluated curves, needs to be cleared when view changes and curves need to be redrawn
		std::unordered_map<std::string, std::vector<std::vector<ImVec2>>> mCurveCache;

		// short curt to member function drawSegmentContent
		using DrawSegmentMemFunPtr = void(SequenceCurveTrackView::*)(const SequenceTrack &track, const SequenceTrackSegment &segment, const ImVec2& trackTopLeft, float previousSegmentX, float segmentWidth, float segmentX, ImDrawList* drawList, bool drawStartValue);

		// static map of member function pointers
		static std::unordered_map<rttr::type, DrawSegmentMemFunPtr> sDrawCurveSegmentsMap;
	};

	//////////////////////////////////////////////////////////////////////////
	// Sequence Curve Track Actions
	//////////////////////////////////////////////////////////////////////////

	namespace SequenceGUIActions
	{
		class HoveringControlPoint : public TrackAction
		{
			RTTI_ENABLE(TrackAction)
		public:
			HoveringControlPoint(std::string trackID, std::string segmentID, int controlPointIndex, int curveIndex)
				: TrackAction(trackID), mSegmentID(segmentID), mControlPointIndex(controlPointIndex), mCurveIndex(curveIndex)
			{

			}

			std::string mSegmentID;
			int mControlPointIndex;
			int mCurveIndex;
		};

		class DraggingControlPoint : public TrackAction
		{
			RTTI_ENABLE(TrackAction)
		public:
			DraggingControlPoint(std::string trackID, std::string segmentID, int controlPointIndex, int curveIndex)
				: TrackAction(trackID), mSegmentID(segmentID), mControlPointIndex(controlPointIndex), mCurveIndex(curveIndex)
			{

			}

			std::string mSegmentID;
			int mControlPointIndex;
			int mCurveIndex;
		};

		class HoveringTanPoint : public TrackAction
		{
			RTTI_ENABLE(TrackAction)
		public:
			HoveringTanPoint(std::string trackID, std::string tanPointID) : TrackAction(trackID), mTanPointID(tanPointID) {}

			std::string mTanPointID;
		};

		class HoveringCurve : public TrackAction
		{
			RTTI_ENABLE(TrackAction)
		public:
			HoveringCurve(std::string trackId, std::string segmentID, int curveIndex)
				: TrackAction(trackId), mSegmentID(segmentID), mCurveIndex(curveIndex) {}

			std::string mSegmentID;
			int mCurveIndex;
		};

		class OpenInsertCurvePointPopup : public TrackAction
		{
			RTTI_ENABLE(TrackAction)
		public:
			OpenInsertCurvePointPopup(std::string trackID, std::string segmentID, int selectedCurve, float pos)
				: TrackAction(trackID), mSegmentID(segmentID), mSelectedIndex(selectedCurve), mPos(pos) {}

			std::string mSegmentID;
			int mSelectedIndex;
			float mPos;
		};

		class InsertingCurvePoint : public TrackAction
		{
			RTTI_ENABLE(TrackAction)
		public:
			InsertingCurvePoint(std::string trackID, std::string segmentID, int selectedCurve, float pos)
				: TrackAction(trackID), mSegmentID(segmentID), mSelectedIndex(selectedCurve), mPos(pos) {}

			std::string mSegmentID;
			int mSelectedIndex;
			float mPos;
		};

		template<typename T>
		class OpenCurvePointActionPopup : public TrackAction
		{
			RTTI_ENABLE(TrackAction)
		public:
			OpenCurvePointActionPopup(std::string trackID, std::string segmentID, int controlPointIndex, int curveIndex, float value, float time, T minimum, T maximum)
				: TrackAction(trackID), mSegmentID(segmentID), mControlPointIndex(controlPointIndex), mCurveIndex(curveIndex), mValue(value), mTime(time), mMinimum(minimum), mMaximum(maximum)
			{

			}

			std::string mSegmentID;
			int mControlPointIndex;
			int mCurveIndex;
			float mValue;
			float mTime;
            T mMinimum;
			T mMaximum;
		};

		template<typename T>
		class CurvePointActionPopup : public TrackAction
		{
			RTTI_ENABLE(TrackAction)
		public:
			CurvePointActionPopup(std::string trackID, std::string segmentID, int controlPointIndex, int curveIndex, float value, float time, T minimum, T maximum)
				: TrackAction(trackID), mSegmentID(segmentID),
				mControlPointIndex(controlPointIndex), mCurveIndex(curveIndex),
				mValue(value), mMinimum(minimum), mMaximum(maximum), mTime(time)
			{

			}

			std::string mSegmentID;
			int mControlPointIndex;
			int mCurveIndex;
			float mValue;
            T mMinimum;
			T mMaximum;
            float mTime;
		};

		class OpenCurveTypePopup : public TrackAction
		{
			RTTI_ENABLE(TrackAction)
		public:
			OpenCurveTypePopup(std::string trackID, std::string segmentID, int index, float pos, ImVec2 windowPos) :
				TrackAction(trackID),
				mSegmentID(segmentID),
				mCurveIndex(index),
				mPos(pos),
				mWindowPos(windowPos) {}

			std::string mSegmentID;
			int mCurveIndex;
			float mPos;
			ImVec2 mWindowPos;
		};

		class CurveTypePopup : public TrackAction
		{
			RTTI_ENABLE(TrackAction)
		public:
			CurveTypePopup(std::string trackID, std::string segmentID, int index, float pos, ImVec2 windowPos) :
				TrackAction(trackID), mSegmentID(segmentID), mCurveIndex(index), mPos(pos), mWindowPos(windowPos) {}

			std::string mSegmentID;
			int mCurveIndex;
			float mPos;
			ImVec2 mWindowPos;
		};

		class DraggingTanPoint : public TrackAction
		{
			RTTI_ENABLE(TrackAction)
		public:
			DraggingTanPoint(std::string trackID, std::string segmentID, int controlPointIndex, int curveIndex, SequenceCurveEnums::TanPointTypes type)
				: TrackAction(trackID), mSegmentID(segmentID), mControlPointIndex(controlPointIndex), mCurveIndex(curveIndex), mType(type) {}

			std::string mSegmentID;
			int mControlPointIndex;
            int mCurveIndex;
			SequenceCurveEnums::TanPointTypes mType;
		};

		class OpenEditTanPointPopup : public TrackAction
		{
			RTTI_ENABLE(TrackAction)
		public:
			OpenEditTanPointPopup(std::string trackID, std::string segmentID, int controlPointIndex, int curveIndex, SequenceCurveEnums::TanPointTypes type, float value, float time)
				: TrackAction(trackID), mSegmentID(segmentID), mControlPointIndex(controlPointIndex), mCurveIndex(curveIndex), mType(type), mValue(value), mTime(time) {}

			std::string mSegmentID;
			int mControlPointIndex;
			int mCurveIndex;
			SequenceCurveEnums::TanPointTypes mType;
			float mValue;
			float mTime;
		};

		class EditingTanPointPopup : public TrackAction
		{
			RTTI_ENABLE(TrackAction)
		public:
			EditingTanPointPopup(std::string trackID, std::string segmentID, int controlPointIndex, int curveIndex, SequenceCurveEnums::TanPointTypes type, float value, float time)
				: TrackAction(trackID), mSegmentID(segmentID), mControlPointIndex(controlPointIndex), mCurveIndex(curveIndex), mType(type), mValue(value), mTime(time){}

			std::string mSegmentID;
			int mControlPointIndex;
			int mCurveIndex;
			SequenceCurveEnums::TanPointTypes mType;
			float mValue;
			float mTime;
		};

		class OpenEditCurveSegmentPopup : public TrackAction
		{
			RTTI_ENABLE(TrackAction)
		public:
			OpenEditCurveSegmentPopup(std::string trackID, std::string segmentID, rttr::type segmentType, double startTime, double duration)
				: TrackAction(trackID), mSegmentID(segmentID), mSegmentType(segmentType), mStartTime(startTime), mDuration(duration)
			{

			}

			std::string mSegmentID;
			rttr::type mSegmentType;
			double mStartTime;
			double mDuration;
		};

		class EditingCurveSegment : public TrackAction {
			RTTI_ENABLE(TrackAction)
		public:
			EditingCurveSegment(std::string trackID, std::string segmentID, rttr::type segmentType, double startTime, double duration)
				: TrackAction(trackID), mSegmentID(segmentID), mSegmentType(segmentType), mStartTime(startTime), mDuration(duration)
			{

			}

			std::string mSegmentID;
			rttr::type mSegmentType;
			double mStartTime;
			double mDuration;
		};

		template<typename T>
		class OpenEditSegmentCurveValuePopup :
			public TrackAction
		{
			RTTI_ENABLE(TrackAction)
		public:
			OpenEditSegmentCurveValuePopup(
				std::string trackId,
				std::string segmentID,
				SequenceCurveEnums::SegmentValueTypes type,
				int curveIndex,
				T value,
				T minimum,
				T maximum) :
				TrackAction(trackId),
				mSegmentID(segmentID),
				mType(type),
				mCurveIndex(curveIndex),
				mValue(value),
				mMinimum(minimum),
				mMaximum(maximum) {}

			std::string mSegmentID;
			SequenceCurveEnums::SegmentValueTypes mType;
			int mCurveIndex;
			T mValue;
			T mMinimum;
			T mMaximum;
		};

		template<typename T>
		class EditingSegmentCurveValue :
			public TrackAction
		{
			RTTI_ENABLE(TrackAction)
		public:
			EditingSegmentCurveValue(std::string trackId, std::string segmentID, SequenceCurveEnums::SegmentValueTypes type, int curveIndex, T value, T minimum, T maximum)
				: TrackAction(trackId), mSegmentID(segmentID), mType(type), mCurveIndex(curveIndex), mValue(value), mMinimum(minimum), mMaximum(maximum) {}

			std::string mSegmentID;
			SequenceCurveEnums::SegmentValueTypes mType;
			int mCurveIndex;
			T mValue;
			T mMinimum;
			T mMaximum;
		};
	}


	//////////////////////////////////////////////////////////////////////////
	// Curve Clipboards
	//////////////////////////////////////////////////////////////////////////

	namespace SequenceGUIClipboards
	{
		/**
		 * CurveSegmentClipboard contains serialized curve segments
		 */
		class CurveSegmentClipboard :
			public Clipboard
		{
			RTTI_ENABLE(Clipboard)
		public:
			/**
			 * Constructor
			 * @param segmentType the segment type that needs to be serialized
			 * @param trackID the track id of track that contains the segment
			 * @param sequenceName the name of the current loaded sequence
			 */
			CurveSegmentClipboard(const rttr::type& segmentType, const std::string& trackID, const std::string& sequenceName) : Clipboard(segmentType), mTrackID(trackID), mSequenceName(sequenceName){}

			/**
			 * returns track id
			 * @return the track id that contains the segment
			 */
			const std::string& getTrackID() const{ return mTrackID; }

			/**
			 * returns sequence name that contains the segment
			 * @return the sequence name that contains the segment
			 */
			const std::string& getSequenceName() const{ return mSequenceName; }
		private:
			std::string mTrackID;
			std::string mSequenceName;
		};
	}

	//////////////////////////////////////////////////////////////////////////
	// Forward declarations
	//////////////////////////////////////////////////////////////////////////

	template<>
	void NAPAPI SequenceCurveTrackView::handleCurvePointActionPopup<float>();

	template<>
	void NAPAPI SequenceCurveTrackView::handleSegmentValueActionPopup<float>();

	template<>
	void NAPAPI SequenceCurveTrackView::showValue<float>(
		const SequenceTrack& track,
		const SequenceTrackSegmentCurve<float>& segment,
		float x,
		double time,
		int curveIndex);

	template<>
	void NAPAPI SequenceCurveTrackView::showValue<glm::vec2>(
		const SequenceTrack& track,
		const SequenceTrackSegmentCurve<glm::vec2>& segment,
		float x,
		double time,
		int curveIndex);

	template<>
	void NAPAPI SequenceCurveTrackView::showValue<glm::vec3>(
		const SequenceTrack& track,
		const SequenceTrackSegmentCurve<glm::vec3>& segment,
		float x,
		double time,
		int curveIndex);

	template<>
	void NAPAPI SequenceCurveTrackView::showValue<glm::vec4>(
		const SequenceTrack& track,
		const SequenceTrackSegmentCurve<glm::vec4>& segment,
		float x,
		double time,
		int curveIndex);

	template<>
	bool NAPAPI SequenceCurveTrackView::inputFloat<float>(float &v, int precision);

	template<>
	bool NAPAPI SequenceCurveTrackView::inputFloat<glm::vec2>(glm::vec2 &v, int precision);

	template<>
	bool NAPAPI SequenceCurveTrackView::inputFloat<glm::vec3>(glm::vec3 &v, int precision);

	template<>
	bool NAPAPI SequenceCurveTrackView::inputFloat<glm::vec4>(glm::vec4 &v, int precision);
}

// Include all template definitions
#include "sequencecurvetrackview_template.h"
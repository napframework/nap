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
	class NAPAPI SequenceCurveTrackView : public SequenceTrackView
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
		virtual bool handlePopups() override;

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
		void NAPAPI drawSegmentContent(const SequenceTrack &track, const SequenceTrackSegment &segment, const ImVec2& trackTopLeft, float previousSegmentX, float segmentWidth, float segmentX, ImDrawList* drawList, bool drawStartValue);
	
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
		void NAPAPI drawSegmentValue(const SequenceTrack& track, const SequenceTrackSegment& segment, const ImVec2 &trackTopLeft, const float segmentX, const float segmentWidth, const SequenceCurveEnums::SegmentValueTypes segmentType, ImDrawList* drawList);
	
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
		void NAPAPI drawControlPoints(const SequenceTrack& track, const SequenceTrackSegment& segment, const ImVec2 &trackTopLeft, const float segmentX, const float segmentWidth, ImDrawList* drawList);

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
		void NAPAPI drawCurves(const SequenceTrack& track, const SequenceTrackSegment& segment, const ImVec2 &trackTopLeft, const float previousSegmentX, const float segmentWidth, const float segmentX, ImDrawList* drawList);

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
		void NAPAPI drawTanHandler(const SequenceTrack &track, const SequenceTrackSegment &segment, std::ostringstream &stringStream, const float segmentWidth, const math::FCurvePoint<float, float> &curvePoint, const ImVec2 &circlePoint, const int controlPointIndex, const int curveIndex, const SequenceCurveEnums::TanPointTypes type, ImDrawList* drawList);
	
		/**
		 * handles insert segment popup
		 */
		virtual bool handleInsertSegmentPopup();

		/**
		 * handles delete segment popup
		 */
		virtual bool handleDeleteSegmentPopup();

		/**
		 * handles insert curve point popup
		 */
		virtual bool handleInsertCurvePointPopup();

		/**
		 * handles curvepoint action popup
		 */
		template<typename T>
		bool NAPAPI handleCurvePointActionPopup();

		/**
		 * handles segment value actions
		 */
		template<typename T>
		bool NAPAPI handleSegmentValueActionPopup();

		/**
		 * handles curve type popup
		 */
		virtual bool handleCurveTypePopup();

		/**
		 * handles tanpoint action popup
		 */
		virtual bool handleTanPointActionPopup();

		/**
		 * Draws min/max range of inspector
		 * @tparam T type
		 * @param track reference to track
		 */
		template<typename T>
		void NAPAPI drawInspectorRange(const SequenceTrack& track);

		/**
		 * @tparam T type of value
		 * @param track reference to track
		 * @param segment reference to segment
		 * @param x value to display
		 * @param time time in segment
		 * @param curveIndex curve index
		 */
		template<typename T>
		void NAPAPI showValue(const SequenceTrack& track, const SequenceTrackSegmentCurve<T>& segment, float x, double time, int curveIndex);
	
		/**
		 * input float that takes type T as input
		 * @tparam T type of inputFloat
		 * @param precision decimal precision
		 * @return true if dragged
		 */
		template<typename T>
		bool NAPAPI inputFloat(T &, int precision);

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
		 * pastes current clipboard as a new segment at given time
		 * @tparam T the segment type
		 * @param trackId the track id of the track to insert into
		 * @param time the time at which to create new segment
		 */
		template<typename T>
		void NAPAPI pasteClipboardSegment(const std::string& trackId, double time);

		/**
		 * pastes content of clipboard segment into another segment
		 * @tparam T the segment type
		 * @param trackId the track id of the track to insert into
		 * @param segmentID the segment id of the segment to replace
		 */
		template<typename T>
		void NAPAPI pasteClipboardSegmentInto(const std::string& trackId, const std::string& segmentID);

		// curve cache holds evaluated curves, needs to be cleared when view changes and curves need to be redrawn
		std::unordered_map<std::string, std::vector<std::vector<ImVec2>>> mCurveCache;

		//
		std::vector<std::function<bool()>> mPopupHandlers;

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
	bool NAPAPI SequenceCurveTrackView::handleCurvePointActionPopup<float>();

	template<>
	bool NAPAPI SequenceCurveTrackView::handleSegmentValueActionPopup<float>();

	template<typename T>
	void SequenceCurveTrackView::pasteClipboardSegment(const std::string& trackId, double time)
	{
		// get clipboard action
		auto* curve_segment_clipboard = mState.mClipboard->getDerived<CurveSegmentClipboard>();

		// create vector & object ptr to be filled by de-serialization
		std::vector<std::unique_ptr<rtti::Object>> read_objects;
		rtti::ObjectPtr<T> curve_segment;

		// continue upon succesfull de-serialization
		if( curve_segment_clipboard->deserialize<T>(read_objects, curve_segment) )
		{
			// obtain controller
			auto& curve_controller = getEditor().getController<SequenceControllerCurve>();

			// insert new segment
			const auto* new_segment = curve_controller.insertSegment(trackId, time);

			// change duration
			curve_controller.segmentDurationChange(trackId, new_segment->mID, curve_segment->mDuration);

			// copy curve points
			for(int c = 0; c < curve_segment->mCurves.size(); c++)
			{
				for(int i = 1; i < curve_segment->mCurves[c]->mPoints.size() - 1; i++)
				{
					curve_controller.insertCurvePoint(trackId, new_segment->mID, curve_segment->mCurves[c]->mPoints[i].mPos.mTime, c);
				}
			}

			// change all curvepoints to match the copied clipboard curve segment
			// note that the first point is always determined by the previous segment
			for(int c = 0; c < curve_segment->mCurves.size(); c++)
			{
				for (int i = 0; i < curve_segment->mCurves[c]->mPoints.size(); i++)
				{
					curve_controller.changeCurvePoint(trackId, new_segment->mID, i, c,
													  curve_segment->mCurves[c]->mPoints[i].mPos.mTime,
													  curve_segment->mCurves[c]->mPoints[i].mPos.mValue);

					curve_controller.changeTanPoint(trackId, new_segment->mID, i, c, SequenceCurveEnums::IN,
													curve_segment->mCurves[c]->mPoints[i].mInTan.mTime,
													curve_segment->mCurves[c]->mPoints[i].mInTan.mValue);
				}
			}

			// make the controller re-align start & end points of segments
			curve_controller.updateCurveSegments(trackId);
		}else
		{
			nap::Logger::error("Error trying to paste clipboard");
		}
	}


	template<typename T>
	void SequenceCurveTrackView::pasteClipboardSegmentInto(const std::string& trackId, const std::string& segmentId)
	{
		// get clipboard action
		auto* curve_segment_clipboard = mState.mClipboard->getDerived<CurveSegmentClipboard>();

		// create vector & object ptr to be filled by de-serialization
		std::vector<std::unique_ptr<rtti::Object>> read_objects;
		rtti::ObjectPtr<T> curve_segment;

		// continue upon successful de-serialization
		if( curve_segment_clipboard->deserialize<T>(read_objects, curve_segment) )
		{
			// obtain controller
			auto& curve_controller = getEditor().getController<SequenceControllerCurve>();

			// insert new segment
			const auto* target_segment = curve_controller.getSegment(trackId, segmentId);

			// upcast target segment to type of T
			const T* target_segment_upcast = dynamic_cast<const T*>(target_segment);
			assert(target_segment_upcast != nullptr); // error in upcast

			// proceed upon successful cast
			if( target_segment_upcast != nullptr )
			{
				// delete all points except the first and last one
				for(size_t c = 0; c < target_segment_upcast->mCurves.size(); c++)
				{
					for(size_t p = 1; p < target_segment_upcast->mCurves[c]->mPoints.size() - 1; p++)
					{
						curve_controller.deleteCurvePoint(trackId, segmentId, p, c);
					}
				}

				// change duration
				curve_controller.segmentDurationChange(trackId, target_segment_upcast->mID, curve_segment->mDuration);

				// copy curve points
				for(int c = 0; c < curve_segment->mCurves.size(); c++)
				{
					for(int i = 1; i < curve_segment->mCurves[c]->mPoints.size() - 1; i++)
					{
						curve_controller.insertCurvePoint(trackId, target_segment_upcast->mID, curve_segment->mCurves[c]->mPoints[i].mPos.mTime, c);
					}
				}

				// change all curvepoints to match the copied clipboard curve segment
				// note that the first point is always determined by the previous segment
				for(int c = 0; c < curve_segment->mCurves.size(); c++)
				{
					for (int i = 0; i < curve_segment->mCurves[c]->mPoints.size(); i++)
					{
						curve_controller.changeCurvePoint(trackId, target_segment_upcast->mID, i, c,
														  curve_segment->mCurves[c]->mPoints[i].mPos.mTime,
														  curve_segment->mCurves[c]->mPoints[i].mPos.mValue);

						curve_controller.changeTanPoint(trackId, target_segment_upcast->mID, i, c, SequenceCurveEnums::IN,
														curve_segment->mCurves[c]->mPoints[i].mInTan.mTime,
														curve_segment->mCurves[c]->mPoints[i].mInTan.mValue);
					}
				}

				// make the controller re-align start & end points of segments
				curve_controller.updateCurveSegments(trackId);
			}else
			{
				nap::Logger::error("Error casting target segment");
			}
		}else
		{
			nap::Logger::error("Error trying to paste clipboard");
		}
	}
}

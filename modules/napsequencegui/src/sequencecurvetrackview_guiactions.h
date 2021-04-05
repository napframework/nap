#pragma once

namespace nap
{
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
				: TrackAction(std::move(trackID)), mSegmentID(std::move(segmentID)), mControlPointIndex(controlPointIndex), mCurveIndex(curveIndex){}

			std::string mSegmentID;
			int mControlPointIndex;
			int mCurveIndex;
		};

		class DraggingControlPoint : public TrackAction
		{
			RTTI_ENABLE(TrackAction)
		public:
			DraggingControlPoint(std::string trackID, std::string segmentID, int controlPointIndex, int curveIndex, float newTime, float newValue)
				: TrackAction(std::move(trackID)), mSegmentID(std::move(segmentID)), mControlPointIndex(controlPointIndex),
				  mCurveIndex(curveIndex), mNewTime(newTime), mNewValue(newValue){}

			std::string mSegmentID;
			int mControlPointIndex;
			int mCurveIndex;
			float mNewTime;
			float mNewValue;
		};

		class HoveringTanPoint : public TrackAction
		{
			RTTI_ENABLE(TrackAction)
		public:
			HoveringTanPoint(std::string trackID, std::string tanPointID) : TrackAction(std::move(trackID)), mTanPointID(std::move(tanPointID)) {}

			std::string mTanPointID;
		};

		class HoveringCurve : public TrackAction
		{
			RTTI_ENABLE(TrackAction)
		public:
			HoveringCurve(std::string trackId, std::string segmentID, int curveIndex)
				: TrackAction(std::move(trackId)), mSegmentID(std::move(segmentID)), mCurveIndex(curveIndex) {}

			std::string mSegmentID;
			int mCurveIndex;
		};

		class OpenInsertCurvePointPopup : public TrackAction
		{
			RTTI_ENABLE(TrackAction)
		public:
			OpenInsertCurvePointPopup(std::string trackID, std::string segmentID, int selectedCurve, float pos)
				: TrackAction(std::move(trackID)), mSegmentID(std::move(segmentID)), mSelectedIndex(selectedCurve), mPos(pos) {}

			std::string mSegmentID;
			int mSelectedIndex;
			float mPos;
		};

		class InsertingCurvePoint : public TrackAction
		{
			RTTI_ENABLE(TrackAction)
		public:
			InsertingCurvePoint(std::string trackID, std::string segmentID, int selectedCurve, float pos)
				: TrackAction(std::move(trackID)), mSegmentID(std::move(segmentID)), mSelectedIndex(selectedCurve), mPos(pos) {}

			std::string mSegmentID;
			int mSelectedIndex;
			float mPos;
		};

		template<typename T>
		class OpenCurvePointActionPopup : public TrackAction
		{
			RTTI_ENABLE(TrackAction)
		public:
			OpenCurvePointActionPopup(const std::string& trackID, std::string segmentID, int controlPointIndex, int curveIndex, float value, float time, T minimum, T maximum)
				: TrackAction(trackID), mSegmentID(std::move(segmentID)), mControlPointIndex(controlPointIndex), mCurveIndex(curveIndex), mValue(value), mTime(time), mMinimum(minimum), mMaximum(maximum){}

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
			CurvePointActionPopup(const std::string& trackID, std::string segmentID, int controlPointIndex, int curveIndex, float value, float time, T minimum, T maximum)
				: TrackAction(trackID), mSegmentID(std::move(segmentID)),
				  mControlPointIndex(controlPointIndex), mCurveIndex(curveIndex),
				  mValue(value), mMinimum(minimum), mMaximum(maximum), mTime(time){}

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
				TrackAction(std::move(trackID)),
				mSegmentID(std::move(segmentID)),
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
				TrackAction(std::move(trackID)), mSegmentID(std::move(segmentID)), mCurveIndex(index), mPos(pos), mWindowPos(windowPos) {}

			std::string mSegmentID;
			int mCurveIndex;
			float mPos;
			ImVec2 mWindowPos;
		};

		class DraggingTanPoint : public TrackAction
		{
			RTTI_ENABLE(TrackAction)
		public:
			DraggingTanPoint(std::string trackID, std::string segmentID, int controlPointIndex, int curveIndex, SequenceCurveEnums::ETanPointTypes type)
				: TrackAction(std::move(trackID)), mSegmentID(std::move(segmentID)), mControlPointIndex(controlPointIndex), mCurveIndex(curveIndex), mType(type) {}

			std::string mSegmentID;
			int mControlPointIndex;
			int mCurveIndex;
			SequenceCurveEnums::ETanPointTypes mType;

			float mNewValue = 0.0f;
			float mNewTime = 0.0f;
		};

		class OpenEditTanPointPopup : public TrackAction
		{
			RTTI_ENABLE(TrackAction)
		public:
			OpenEditTanPointPopup(std::string trackID, std::string segmentID, int controlPointIndex, int curveIndex, SequenceCurveEnums::ETanPointTypes type, float value, float time)
				: TrackAction(std::move(trackID)), mSegmentID(std::move(segmentID)), mControlPointIndex(controlPointIndex), mCurveIndex(curveIndex), mType(type), mValue(value), mTime(time) {}

			std::string mSegmentID;
			int mControlPointIndex;
			int mCurveIndex;
			SequenceCurveEnums::ETanPointTypes mType;
			float mValue;
			float mTime;
		};

		class EditingTanPointPopup : public TrackAction
		{
			RTTI_ENABLE(TrackAction)
		public:
			EditingTanPointPopup(std::string trackID, std::string segmentID, int controlPointIndex, int curveIndex, SequenceCurveEnums::ETanPointTypes type, float value, float time)
				: TrackAction(std::move(trackID)), mSegmentID(std::move(segmentID)), mControlPointIndex(controlPointIndex), mCurveIndex(curveIndex), mType(type), mValue(value), mTime(time){}

			std::string mSegmentID;
			int mControlPointIndex;
			int mCurveIndex;
			SequenceCurveEnums::ETanPointTypes mType;
			float mValue;
			float mTime;
		};

		class OpenEditCurveSegmentPopup : public TrackAction
		{
			RTTI_ENABLE(TrackAction)
		public:
			OpenEditCurveSegmentPopup(std::string trackID, std::string segmentID, const rttr::type& segmentType, double startTime, double duration)
				: TrackAction(std::move(trackID)), mSegmentID(std::move(segmentID)), mSegmentType(segmentType), mStartTime(startTime), mDuration(duration){}

			std::string mSegmentID;
			rttr::type mSegmentType;
			double mStartTime;
			double mDuration;
		};

		class EditingCurveSegment : public TrackAction
		{
			RTTI_ENABLE(TrackAction)
		public:
			EditingCurveSegment(std::string trackID, std::string segmentID, const rttr::type& segmentType, double startTime, double duration)
				: TrackAction(std::move(trackID)), mSegmentID(std::move(segmentID)), mSegmentType(segmentType), mStartTime(startTime), mDuration(duration){}

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
			OpenEditSegmentCurveValuePopup(const std::string& trackId, std::string segmentID,
										   SequenceCurveEnums::SegmentValueTypes type, int curveIndex, T value,
										   T minimum, T maximum) :
				TrackAction(trackId), mSegmentID(std::move(segmentID)),
				mType(type), mCurveIndex(curveIndex), mValue(value),
				mMinimum(minimum), mMaximum(maximum) {}

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
			EditingSegmentCurveValue(const std::string& trackId, std::string segmentID, SequenceCurveEnums::SegmentValueTypes type, int curveIndex, T value, T minimum, T maximum)
				: TrackAction(trackId), mSegmentID(std::move(segmentID)), mType(type), mCurveIndex(curveIndex), mValue(value), mMinimum(minimum), mMaximum(maximum) {}

			std::string mSegmentID;
			SequenceCurveEnums::SegmentValueTypes mType;
			int mCurveIndex;
			T mValue;
			T mMinimum;
			T mMaximum;
		};

		template<typename T>
		class ChangeMinMaxCurve :
			public TrackAction
		{
			RTTI_ENABLE(TrackAction)
		public:
			ChangeMinMaxCurve(const std::string& trackID, T newMin, T newMax)
				: TrackAction(trackID), mNewMin(newMin), mNewMax(newMax){}

			T mNewMin;
			T mNewMax;
		};

		class AssignNewObjectIDToTrack :
			public TrackAction
		{
			RTTI_ENABLE(TrackAction)
		public:
			AssignNewObjectIDToTrack(const std::string& trackID, std::string objectID)
				: TrackAction(trackID), mObjectID(std::move(objectID)){}

			std::string mObjectID;
		};

		class HoveringSegmentValue : public TrackAction
		{
			RTTI_ENABLE(TrackAction)
		public:
			HoveringSegmentValue(std::string trackId, std::string segmentID, SequenceCurveEnums::SegmentValueTypes type, int curveIndex)
				: TrackAction(std::move(trackId)), mSegmentID(std::move(segmentID)), mType(type), mCurveIndex(curveIndex) {}

			std::string mSegmentID;
			SequenceCurveEnums::SegmentValueTypes mType;
			int mCurveIndex;
		};

		class DraggingSegmentValue :
			public TrackAction
		{
			RTTI_ENABLE(TrackAction)
		public:
			DraggingSegmentValue(std::string trackId, std::string segmentID, SequenceCurveEnums::SegmentValueTypes type, int curveIndex, float newValue)
				: TrackAction(std::move(trackId)), mSegmentID(std::move(segmentID)), mType(type), mCurveIndex(curveIndex), mNewValue(newValue) {}

			std::string mSegmentID;
			SequenceCurveEnums::SegmentValueTypes mType;
			int mCurveIndex;
			float mNewValue;
		};
	}
}
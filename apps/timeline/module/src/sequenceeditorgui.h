#pragma once

// internal includes
#include "sequenceeditor.h"

// external includes
#include <nap/resource.h>
#include <nap/resourceptr.h>
#include <rtti/objectptr.h>
#include <imgui/imgui.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	// forward declares
	class SequenceEditorGUIView;
	class SequenceEditorView;
	class SequenceGUIActionData;

	/**
	 * SequenceEditorGUI
	 * A GUI resource that can be instantiated to draw a GUI (view) for the sequence editor
	 */
	class NAPAPI SequenceEditorGUI : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		/**
		 * init
		 * @param errorState contains any errors
		 * @return true on success
		 */
		virtual bool init(utility::ErrorState& errorState);

		/**
		 * onDestroy
		 * called before deconstruction
		 */
		virtual void onDestroy();

		/**
		 * draw
		 * Call this method to draw the GUI
		 */
		void draw();
	public:
		// properties
		ResourcePtr<SequenceEditor> mSequenceEditor = nullptr; ///< Property: 'Sequence Editor' link to editor resource
	protected:
		// instantiated view
		std::unique_ptr<SequenceEditorGUIView> mView = nullptr; 
	};

	/**
	 * Types of possible interactions with GUI
	 * Used by the gui state to handle mouse input / popups / actions
	 */
	namespace SequenceGUIMouseActions
	{
		enum Types
		{
			// ACTIONS
			DRAGGING_SEGMENT,
			INSERTING_SEGMENT,
			OPEN_INSERT_SEGMENT_POPUP,
			EDITING_SEGMENT,
			OPEN_EDIT_SEGMENT_POPUP,
			HOVERING_SEGMENT,
			HOVERING_SEGMENT_VALUE,
			DRAGGING_SEGMENT_VALUE,
			HOVERING_CONTROL_POINT,
			DRAGGING_CONTROL_POINT,
			HOVERING_TAN_POINT,
			DRAGGING_TAN_POINT,
			HOVERING_CURVE,
			HOVERING_PLAYER_TIME,
			DRAGGING_PLAYER_TIME,
			OPEN_INSERT_TRACK_POPUP,
			INSERTING_TRACK,
			OPEN_INSERT_EVENT_SEGMENT_POPUP,
			INSERTING_EVENT_SEGMENT,
			OPEN_EDIT_EVENT_SEGMENT_POPUP,
			EDITING_EVENT_SEGMENT,
			OPEN_INSERT_CURVE_POINT_POPUP,
			INSERTING_CURVE_POINT,
			OPEN_CURVE_POINT_ACTION_POPUP,
			CURVE_POINT_ACTION_POPUP,
			OPEN_CURVE_TYPE_POPUP,
			CURVE_TYPE_POPUP,
			LOAD,
			SAVE_AS,
			NONE
		};
	}

	/**
	 * SequenceEditorGUIState
	 * Class holding state information for current Editor GUI
	 */
	class SequenceEditorGUIState
	{
	public:
		/**
		 * default constructor
		 */
		SequenceEditorGUIState() = default;

		/**
		 * deconstructor
		 */
		~SequenceEditorGUIState() {}

		// current action
		SequenceGUIMouseActions::Types currentAction
			= SequenceGUIMouseActions::Types::NONE;

		// current object id, used to identify object with actions
		std::string currentObjectID = "";

		// current action data
		std::unique_ptr<SequenceGUIActionData> currentActionData;
	};


	/**
	 * SequenceEditorGUIView
	 * Responsible for draw the GUI for the sequence editor
	 * Needs reference to controller
	 */
	class SequenceEditorGUIView
	{
	public:

		SequenceEditorGUIView(SequenceEditorController& controller, std::string id);

		virtual void draw();
	private:
		void drawTracks(
			const SequencePlayer& sequencePlayer,
			const Sequence &sequence);

		template<typename T>
		void drawCurveTrack(
			const SequenceTrack &track,
			ImVec2 &cursorPos,
			const float marginBetweenTracks,
			const SequencePlayer &sequencePlayer,
			bool &deleteTrack,
			std::string &deleteTrackID);

		void drawEventTrack(
			const SequenceTrack &track,
			ImVec2 &cursorPos,
			const float marginBetweenTracks,
			const SequencePlayer &sequencePlayer,
			bool &deleteTrack,
			std::string &deleteTrackID);

		template<typename T>
		void drawSegmentContent(
			const SequenceTrack &track,
			const SequenceTrackSegment &segment,
			const ImVec2& trackTopLeft,
			float previousSegmentX,
			float segmentWidth,
			float segmentX,
			ImDrawList* drawList,
			bool drawStartValue);

		template<typename T>
		void drawSegmentValue(
			const SequenceTrack& track,
			const SequenceTrackSegment& segment,
			const ImVec2 &trackTopLeft,
			const float segmentX,
			const float segmentWidth,
			const SequenceEditorTypes::SegmentValueTypes segmentType,
			ImDrawList* drawList);

		void drawSegmentHandler(
			const SequenceTrack& track,
			const SequenceTrackSegment& segment,
			const ImVec2 &trackTopLeft,
			const float segmentX,
			const float segmentWidth,
			ImDrawList* drawList
		);

		template<typename T>
		void drawControlPoints(
			const SequenceTrack& track,
			const SequenceTrackSegment& segment,
			const ImVec2 &trackTopLeft,
			const float segmentX,
			const float segmentWidth,
			ImDrawList* drawList);

		template<typename T>
		void drawCurves(
			const SequenceTrack& track,
			const SequenceTrackSegment& segment,
			const ImVec2 &trackTopLeft,
			const float previousSegmentX,
			const float segmentWidth,
			const float segmentX,
			ImDrawList* drawList);

		template<typename T>
		void drawTanHandler(
			const SequenceTrack &track,
			const SequenceTrackSegment &segment,
			std::ostringstream &stringStream,
			const float segmentWidth,
			const math::FCurvePoint<float, float> &curvePoint,
			const ImVec2 &circlePoint,
			const int controlPointIndex,
			const int curveIndex,
			const SequenceEditorTypes::TanPointTypes type,
			ImDrawList* drawList);

		void drawPlayerController(SequencePlayer& playerm);

		void drawTimelinePlayerPosition(const Sequence& sequence, SequencePlayer& player);

		void handleInsertSegmentPopup();

		void handleDeleteSegmentPopup();

		void handleInsertTrackPopup();

		void handleInsertEventSegmentPopup();

		void handleInsertCurvePointPopup();

		void handleLoadPopup();

		void handleSaveAsPopup();

		void handleCurvePointActionPopup();

		void handleCurveTypePopup();

		void handleEditEventSegmentPopup();
	protected:
		// ImGUI tools
		bool Combo(const char* label, int* currIndex, std::vector<std::string>& values);

		bool ListBox(const char* label, int* currIndex, std::vector<std::string>& values);

		template<typename T>
		bool inputFloat(T &, int precision);

		std::string formatTimeString(double time);

		template<typename T>
		void drawInspectorRange(const SequenceTrack& track);

		template<typename T>
		void showValue(
			const SequenceTrack& track,
			const SequenceTrackSegmentCurve<T>& segment, 
			float x, 
			double time,
			int curveIndex);
	protected:

		std::unordered_map<std::string, std::vector<ImVec2>> mCurveCache;

		SequenceEditorController& mController;
	protected:
		SequenceEditorGUIState mEditorAction;

		std::string mID;
		ImVec2 mPreviousMousePos;

		bool mIsWindowFocused = false;
		ImVec2 mMouseDelta;
		ImVec2 mMousePos;
		ImVec2 mWindowPos;
		ImVec2 mTimelineControllerPos;
		float mTimelineWidth;
		float mStepSize;
		float mTrackHeight;
		float mInspectorWidth;
		ImVec2 mPrevWindowPos;
		ImVec2 mPrevScroll;
		float mVerticalResolution = 150.0f;
		float mHorizontalResolution = 100.0f;
		double mMouseCursorTime;
	};

	class SequenceGUIActionData
	{
	public:
		SequenceGUIActionData() {}
		virtual ~SequenceGUIActionData() {}
	};

	class SequenceGUIEditSegmentData : public SequenceGUIActionData
	{
	public:
		SequenceGUIEditSegmentData(
			std::string trackId_,
			std::string segmentID_,
			SequenceTrackTypes::Types trackType) :
				mTrackID(trackId_),
				mSegmentID(segmentID_),
				mTrackType(trackType){}

		std::string mTrackID;
		std::string mSegmentID;
		SequenceTrackTypes::Types mTrackType;
	};

	class SequenceGUIInsertSegmentData : public SequenceGUIActionData
	{
	public:
		SequenceGUIInsertSegmentData(std::string id, double t, SequenceTrackTypes::Types type) : trackID(id), time(t), trackType(type) {}

		double time = 0.0;
		std::string trackID;
		SequenceTrackTypes::Types trackType;
	};

	class SequenceGUIDragPlayerData : public SequenceGUIActionData
	{
	public:
		SequenceGUIDragPlayerData(
			bool playerWasPlaying_,
			bool playerWasPaused_)
			:	playerWasPlaying(playerWasPlaying_),
				playerWasPaused(playerWasPaused_){}

		bool playerWasPlaying;
		bool playerWasPaused;
	};

	class SequenceGUIDragSegmentData : public SequenceGUIActionData
	{
	public:
		SequenceGUIDragSegmentData(
			std::string trackId_,
			std::string segmentID_,
			SequenceEditorTypes::SegmentValueTypes type_,
			int curveIndex_)
			: trackID(trackId_),
			segmentID(segmentID_),
			type(type_),
			curveIndex(curveIndex_){}

		std::string			trackID;
		std::string			segmentID;
		SequenceEditorTypes::SegmentValueTypes	type;
		int					curveIndex;
	};

	class SequenceGUIDragTanPointData : public SequenceGUIActionData
	{
	public:
		SequenceGUIDragTanPointData(
			std::string trackId_,
			std::string segmentID_,
			int controlPointIndex_,
			int curveIndex_,
			SequenceEditorTypes::TanPointTypes type_)
			: trackID(trackId_),
			segmentID(segmentID_),
			controlPointIndex(controlPointIndex_),
			curveIndex(curveIndex_),
			type(type_) {}

		std::string		trackID;
		std::string		segmentID;
		int				controlPointIndex;
		SequenceEditorTypes::TanPointTypes	type;
		int				curveIndex;
	};

	class SequenceGUILoadShowData : public SequenceGUIActionData
	{
	public:
		SequenceGUILoadShowData() {}

		int selectedShow = 0;
		std::string errorString;
	};

	class SequenceGUIHoveringCurveData : public SequenceGUIActionData
	{
	public:
		SequenceGUIHoveringCurveData(int index) : selectedIndex(index) {}

		int selectedIndex;
	};

	class SequenceGUIInsertCurvePointData : public SequenceGUIActionData
	{
	public:
		SequenceGUIInsertCurvePointData(
			std::string trackID, 
			std::string segmentID,
			int index,
			float pos,
			SequenceTrackTypes::Types type) :
			mTrackID(trackID), mSegmentID(segmentID), 
			mSelectedIndex(index), mPos(pos), mSegmentType(type) {}

		std::string mTrackID;
		std::string mSegmentID;
		int mSelectedIndex;
		float mPos;
		SequenceTrackTypes::Types mSegmentType;
	};

	class SequenceGUIChangeCurveData : public SequenceGUIActionData
	{
	public:
		SequenceGUIChangeCurveData(
			std::string trackID,
			std::string segmentID,
			int index,
			SequenceTrackTypes::Types type,
			ImVec2 windowPos) :
			mTrackID(trackID), mSegmentID(segmentID),
			mSelectedIndex(index), mSegmentType(type), mWindowPos(windowPos) {}

		std::string mTrackID;
		std::string mSegmentID;
		int mSelectedIndex;
		SequenceTrackTypes::Types mSegmentType;
		ImVec2 mWindowPos;
	};

	class SequenceGUIControlPointData : public SequenceGUIActionData
	{
	public:
		SequenceGUIControlPointData(
			std::string trackId_, 
			std::string segmentID_,
			int controlPointIndex_,
			int curveIndex_)
			: trackID(trackId_), 
			segmentID(segmentID_),
			controlPointIndex(controlPointIndex_),
			curveIndex(curveIndex_){}

		std::string trackID;
		std::string segmentID;
		int			controlPointIndex;
		int			curveIndex;
	};

	class SequenceGUIControlPointActionData : public SequenceGUIActionData
	{
	public:
		SequenceGUIControlPointActionData(
			std::string trackId,
			std::string segmentID,
			int controlPointIndex,
			int curveIndex,
			SequenceTrackTypes::Types trackType)
			: mTrackId(trackId), 
			mSegmentID(segmentID), 
			mControlPointIndex(controlPointIndex),
			mCurveIndex(curveIndex),
			mTrackType(trackType)
			{}

		std::string mTrackId;
		std::string mSegmentID;
		int			mControlPointIndex;
		int			mCurveIndex;
		SequenceTrackTypes::Types mTrackType;
	};

	class SequenceGUISaveShowData : public SequenceGUIActionData
	{
	public:
		SequenceGUISaveShowData() {}

		int selectedShow = 0;
		std::string errorString;
	};

	class SequenceGUIInsertEventSegment : public SequenceGUIActionData
	{
	public:
		SequenceGUIInsertEventSegment(std::string id, double aTime)
			: trackID(id), time(aTime){}

		std::string trackID;
		double time;
		std::string eventMessage = "Hello world";
	};

	class SequenceGUIEditEventSegment : public SequenceGUIActionData
	{
	public:
		SequenceGUIEditEventSegment(
			std::string trackId,
			std::string segmentID,
			std::string message,
			ImVec2 windowPos)
			: 
			mTrackID(trackId),
			mSegmentID(segmentID),
			mMessage(message),
			mWindowPos(windowPos){}

		std::string mTrackID;
		std::string mSegmentID;
		std::string mMessage;
		ImVec2 mWindowPos;
	};
}

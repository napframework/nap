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
	 */
	class NAPAPI SequenceEditorGUI : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		virtual bool init(utility::ErrorState& errorState);

		virtual void onDestroy();

		void draw();
	public:
		ResourcePtr<SequenceEditor> mSequenceEditor = nullptr;
	protected:
		std::unique_ptr<SequenceEditorGUIView> mView = nullptr;
	};


	enum SequenceGUIMouseActions
	{
		// ACTIONS
		DRAGGING_SEGMENT,
		INSERTING_SEGMENT,
		OPEN_INSERT_SEGMENT_POPUP,
		DELETING_SEGMENT,
		OPEN_DELETE_SEGMENT_POPUP,
		HOVERING_SEGMENT,
		HOVERING_SEGMENT_VALUE,
		DRAGGING_SEGMENT_VALUE,
		HOVERING_CONTROL_POINT,
		DRAGGING_CONTROL_POINT,
		DELETE_CONTROL_POINT,
		HOVERING_TAN_POINT,
		DRAGGING_TAN_POINT,
		HOVERING_CURVE,
		HOVERING_PLAYER_TIME,
		DRAGGING_PLAYER_TIME,
		OPEN_INSERT_TRACK_POPUP,
		INSERTING_TRACK,
		LOAD,
		SAVE_AS,
		NONE
	};


	/**
	 * 
	 */
	class SequenceGUIState
	{
	public:
		SequenceGUIMouseActions currentAction = SequenceGUIMouseActions::NONE;
		std::string currentObjectID = "";
		std::unique_ptr<SequenceGUIActionData> currentActionData;
		bool isWindowFocused = false;
		ImVec2 mouseDelta;
		ImVec2 mousePos;
		ImVec2 windowPos;
		ImVec2 timelineControllerPos;
		float timelineWidth;
		float stepSize;
		float trackHeight;
		float inspectorWidth;
	};


	/**
	 */
	class SequenceEditorView
	{
	public:
		// constructor
		SequenceEditorView(SequenceEditorController& controller);
	protected:
		SequenceEditorController& mController;
	};


	/**
	 */
	class SequenceEditorGUIView : public SequenceEditorView
	{
	public:
		SequenceEditorGUIView(
			SequenceEditorController& controller,
			std::string id);

		void draw();
	private:
		void drawTracks(
			const SequencePlayer& sequencePlayer,
			const Sequence &sequence);

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
			const SegmentValueTypes segmentType,
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
			const TanPointTypes type,
			ImDrawList* drawList);

		void drawPlayerController(SequencePlayer& playerm);

		void handleInsertSegmentPopup();

		void handleDeleteSegmentPopup();

		void handleInsertTrackPopup();

		void handleLoadPopup();

		void handleSaveAsPopup();

		void drawTimelinePlayerPosition(const Sequence& sequence, SequencePlayer& player);

		std::string mID;
		SequenceGUIState mState;
		ImVec2 mPreviousMousePos = { 0,0 };
	private:
		// ImGUI tools
		bool Combo(const char* label, int* currIndex, std::vector<std::string>& values);
		bool ListBox(const char* label, int* currIndex, std::vector<std::string>& values);
		std::string formatTimeString(double time);

		float mVerticalResolution = 100.0f;
		float mHorizontalResolution = 100.0f;
		std::unordered_map<std::string, std::vector<ImVec2>> mCurveCache;

		// used to determine if we need to cache the curves again
		ImVec2 mPrevWindowPos;
		ImVec2 mPrevScroll;
	private:
	};

	class SequenceGUIActionData
	{
	public:
		SequenceGUIActionData() {}
		virtual ~SequenceGUIActionData() {}
	};

	class SequenceGUIDeleteSegmentData : public SequenceGUIActionData
	{
	public:
		SequenceGUIDeleteSegmentData(std::string trackId_, std::string segmentID_) : trackID(trackId_), segmentID(segmentID_) {}

		std::string trackID;
		std::string segmentID;
	};

	class SequenceGUIInsertSegmentData : public SequenceGUIActionData
	{
	public:
		SequenceGUIInsertSegmentData(std::string id, double t, SequenceTrackTypes type) : trackID(id), time(t), trackType(type) {}

		double time = 0.0;
		std::string trackID;
		SequenceTrackTypes trackType;
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
			SegmentValueTypes type_,
			int curveIndex_)
			: trackID(trackId_),
			segmentID(segmentID_),
			type(type_),
			curveIndex(curveIndex_){}

		std::string			trackID;
		std::string			segmentID;
		SegmentValueTypes	type;
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
			TanPointTypes type_)
			: trackID(trackId_),
			segmentID(segmentID_),
			controlPointIndex(controlPointIndex_),
			curveIndex(curveIndex_),
			type(type_) {}

		std::string		trackID;
		std::string		segmentID;
		int				controlPointIndex;
		TanPointTypes	type;
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

	class SequenceGUIDeleteControlPointData : public SequenceGUIActionData
	{
	public:
		SequenceGUIDeleteControlPointData(
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

	class SequenceGUISaveShowData : public SequenceGUIActionData
	{
	public:
		SequenceGUISaveShowData() {}

		int selectedShow = 0;
		std::string errorString;
	};
}

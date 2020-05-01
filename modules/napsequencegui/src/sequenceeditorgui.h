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
		void show();
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
	class NAPAPI SequenceEditorGUIView
	{
	public:
		// shortcuts to member function pointers, used in static maps
		using DrawTrackMemFunPtr = void(SequenceEditorGUIView::*)(const SequenceTrack&, ImVec2&, const float, const SequencePlayer&, bool&, std::string&);
		using DrawCurveSegmentMemFunPtr = void(SequenceEditorGUIView::*)(const SequenceTrack&, const SequenceTrackSegment&, const ImVec2& , float, float, float, ImDrawList*, bool);

		/**
		 * Constructor
		 * @param controller reference to editor controller
		 * @param id id of the GUI resource, used to push ID by IMGUI
		 */
		SequenceEditorGUIView(SequenceEditorController& controller, std::string id);

		/**
		 * shows the editor interface
		 */
		virtual void show();
	private:
		/**
		 * Draws the tracks of the sequence
		 * @param sequencePlayer reference to sequenceplayer
		 * @param sequence reference to sequence
		 */
		void drawTracks(const SequencePlayer& sequencePlayer, const Sequence &sequence);

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
		void drawCurveTrack(const SequenceTrack &track,ImVec2 &cursorPos, const float marginBetweenTracks, const SequencePlayer &sequencePlayer, bool &deleteTrack, std::string &deleteTrackID);

		/**
		 * drawEventTrack
		 * draws event track
		 * @param track reference to track
		 * @param cursorPos imgui cursorposition
		 * @param marginBetweenTracks y margin between tracks
		 * @param sequencePlayer reference to sequence player
		 * @param deleteTrack set to true when delete track button is pressed
		 * @param deleteTrackID the id of track that needs to be deleted
		 */
		void drawEventTrack(const SequenceTrack &track,ImVec2 &cursorPos, const float marginBetweenTracks, const SequencePlayer &sequencePlayer, bool &deleteTrack, std::string &deleteTrackID);

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
		void drawSegmentValue(const SequenceTrack& track, const SequenceTrackSegment& segment, const ImVec2 &trackTopLeft,const float segmentX, const float segmentWidth, const SequenceEditorTypes::SegmentValueTypes segmentType, ImDrawList* drawList);

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
		void drawTanHandler(const SequenceTrack &track, const SequenceTrackSegment &segment, std::ostringstream &stringStream, const float segmentWidth,const math::FCurvePoint<float, float> &curvePoint, const ImVec2 &circlePoint, const int controlPointIndex, const int curveIndex, const SequenceEditorTypes::TanPointTypes type, ImDrawList* drawList);

		/**
		 * drawPlayerController
		 * draws player controller bar
		 * @param player reference to player
		 */
		void drawPlayerController(SequencePlayer& player);

		/**
		 * drawTimelinePlayerPosition
		 * draws line of player position
		 * @param sequence reference to sequence
		 * @param player reference to player
		 */
		void drawTimelinePlayerPosition(const Sequence& sequence, SequencePlayer& player);

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
		 * handleInsertTrackPopup
		 * handles insert track popup
		 */
		void handleInsertTrackPopup();

		/**
		 * handlerInsertEventSegmentPopup
		 * handles insert event segment popup
		 */
		void handleInsertEventSegmentPopup();

		/**
		 * handleInsertCurvePointPopup
		 * handles insert curve point popup
		 */
		void handleInsertCurvePointPopup();

		/**
		 * handleLoadPopup
		 * handles load popup
		 */
		void handleLoadPopup();

		/**
		 * handleSaveAsPopup
		 * handles save as popup
		 */
		void handleSaveAsPopup();

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
		 * handleEditEventSegmentPopup
		 * handles event segment popup
		 */
		void handleEditEventSegmentPopup();
	protected:
		/**
		 * Combo
		 * Combobox that takes std::vector as input
		 * @param label label of box
		 * @param currIndex current index of combo box
		 * @param values vector of string values
		 * @return true if someting is selected
		 */
		bool Combo(const char* label, int* currIndex, std::vector<std::string>& values);

		/**
		 * ListBox
		 * ListBox that takes std::vector as input
		 * @param label label of box
		 * @param currIndex current index of combo box
		 * @param values vector of string values
		 * @return true if someting is selected
		 */
		bool ListBox(const char* label, int* currIndex, std::vector<std::string>& values);

		/**
		 * inputFloat
		 * input float that takes type T as input
		 * @tparam T type of inputFloat
		 * @param precision decimal precision
		 * @return true if dragged
		 */
		template<typename T>
		bool inputFloat(T &, int precision);

		/**
		 * formatTimeString
		 * formats time ( seconds ) to human readable time
		 * @param time time
		 * @return string with readable time
		 */
		std::string formatTimeString(double time);

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
	protected:
		// cache of curves
		std::unordered_map<std::string, std::vector<ImVec2>> mCurveCache;

		// reference to controller
		SequenceEditorController& mController;
	protected:
		// action information
		SequenceEditorGUIState mEditorAction;

		// id
		std::string mID;

		// previous mouse pos
		ImVec2 mPreviousMousePos;

		// window focused
		bool mIsWindowFocused = false;

		// mouse delte
		ImVec2 mMouseDelta;

		// current mouse position
		ImVec2 mMousePos;

		// current window position
		ImVec2 mWindowPos;

		// current window size
		ImVec2 mWindowSize;

		// current timeline controller position
		ImVec2 mTimelineControllerPos;

		// current timelinewidth
		float mTimelineWidth;

		// stepsize ( how many pixels per second ? )
		float mStepSize;

		// current trackheight / vertical resolution
		float mTrackHeight;

		// width of inspector box
		float mInspectorWidth;

		// previous window position
		ImVec2 mPrevWindowPos;

		// previous window scroll
		ImVec2 mPrevScroll;

		// vertical resolution
		float mVerticalResolution = 150.0f;

		// horizontal resolution
		float mHorizontalResolution = 100.0f;

		// current time in sequence of mouse cursor
		double mMouseCursorTime;

		//
		static std::unordered_map<rttr::type, DrawTrackMemFunPtr> sDrawTracksMap;

		static std::unordered_map<rttr::type, DrawCurveSegmentMemFunPtr> sDrawCurveSegmentsMap;
	};

	/**
	 * SequenceGUIActionData
	 * Base class for data needed for handling GUI actions
	 */
	class SequenceGUIActionData
	{
	public:
		/**
		 * Constructor
		 */
		SequenceGUIActionData() {}

		/**
		 * Deconstructor
		 */
		virtual ~SequenceGUIActionData() {}
	};

	/**
	 * Data needed for editing segment
	 */
	class SequenceGUIEditSegmentData : public SequenceGUIActionData
	{
	public:
		SequenceGUIEditSegmentData(std::string trackID, std::string segmentID,  rttr::type typeInfo) :
				mTrackID(trackID), mSegmentID(segmentID), mSegmentTypeInfo(typeInfo){}

		std::string mTrackID;
		std::string mSegmentID;
		rttr::type mSegmentTypeInfo;
	};

	/**
	 * Data needed for inserting segments
	 */
	class SequenceGUIInsertSegmentData : public SequenceGUIActionData
	{
	public:
		SequenceGUIInsertSegmentData(std::string id, double t, rttr::type typeInfo) : mID(id), mTime(t), mTrackTypeInfo(typeInfo){}

		double mTime = 0.0;
		std::string mID;
		rttr::type mTrackTypeInfo;
	};

	/**
	 * Data needed for dragging player position
	 */
	class SequenceGUIDragPlayerData : public SequenceGUIActionData
	{
	public:
		SequenceGUIDragPlayerData(bool playerWasPlaying,bool playerWasPaused)
			:	mPlayerWasPlaying(playerWasPlaying), mPlayerWasPaused(playerWasPaused){}

		bool mPlayerWasPlaying;
		bool mPlayerWasPaused;
	};

	/**
	 * Data needed for dragging segments
	 */
	class SequenceGUIDragSegmentData : public SequenceGUIActionData
	{
	public:
		SequenceGUIDragSegmentData(std::string trackId, std::string segmentID, SequenceEditorTypes::SegmentValueTypes type, int curveIndex)
			: mTrackID(trackId), mSegmentID(segmentID), mType(type), mCurveIndex(curveIndex){}

		std::string mTrackID;
		std::string mSegmentID;
		SequenceEditorTypes::SegmentValueTypes mType;
		int mCurveIndex;
	};

	/**
	 * Data needed for handling dragging of tangents
	 */
	class SequenceGUIDragTanPointData : public SequenceGUIActionData
	{
	public:
		SequenceGUIDragTanPointData(std::string trackId, std::string segmentID, int controlPointIndex, int curveIndex, SequenceEditorTypes::TanPointTypes type)
			: mTrackID(trackId), mSegmentID(segmentID), mControlPointIndex(controlPointIndex), mCurveIndex(curveIndex), mType(type) {}

		std::string mTrackID;
		std::string mSegmentID;
		int mControlPointIndex;
		SequenceEditorTypes::TanPointTypes mType;
		int mCurveIndex;
	};

	/**
	 * Data needed for loading shows
	 */
	class SequenceGUILoadShowData : public SequenceGUIActionData
	{
	public:
		SequenceGUILoadShowData() {}

		int mSelectedShow = 0;
		std::string mErrorString;
	};

	/**
	 * Data needed for hovering curves
	 */
	class SequenceGUIHoveringCurveData : public SequenceGUIActionData
	{
	public:
		SequenceGUIHoveringCurveData(int index) : mSelectedIndex(index) {}

		int mSelectedIndex;
	};

	/**
	 * Data needed for inserting points
	 */
	class SequenceGUIInsertCurvePointData : public SequenceGUIActionData
	{
	public:
		SequenceGUIInsertCurvePointData(std::string trackID, std::string segmentID, int index, float pos) :
			mTrackID(trackID), mSegmentID(segmentID), mSelectedIndex(index), mPos(pos) {}

		std::string mTrackID;
		std::string mSegmentID;
		int mSelectedIndex;
		float mPos;
	};

	/**
	 * Data needed for changing curves
	 */
	class SequenceGUIChangeCurveData : public SequenceGUIActionData
	{
	public:
		SequenceGUIChangeCurveData(std::string trackID, std::string segmentID, int index, ImVec2 windowPos) :
			mTrackID(trackID), mSegmentID(segmentID), mSelectedIndex(index), mWindowPos(windowPos) {}

		std::string mTrackID;
		std::string mSegmentID;
		int mSelectedIndex;
		ImVec2 mWindowPos;
	};

	/**
	 * Data needed for handling control point
	 */
	class SequenceGUIControlPointData : public SequenceGUIActionData
	{
	public:
		SequenceGUIControlPointData(std::string trackID, std::string segmentID, int controlPointIndex, int curveIndex)
			: mTrackID(trackID), mSegmentID(segmentID), mControlIndex(controlPointIndex), mCurveIndex(curveIndex){}

		std::string mTrackID;
		std::string mSegmentID;
		int mControlIndex;
		int mCurveIndex;
	};

	/**
	 * Data needed for handling control points
	 */
	class SequenceGUIControlPointActionData : public SequenceGUIActionData
	{
	public:
		SequenceGUIControlPointActionData(std::string trackId, std::string segmentID, int controlPointIndex, int curveIndex)
			: mTrackId(trackId), mSegmentID(segmentID), mControlPointIndex(controlPointIndex), mCurveIndex(curveIndex)
			{}

		std::string mTrackId;
		std::string mSegmentID;
		int			mControlPointIndex;
		int			mCurveIndex;
	};

	/**
	 * Data needed for handling saving of sequences
	 */
	class SequenceGUISaveShowData : public SequenceGUIActionData
	{
	public:
		SequenceGUISaveShowData() {}

		int mSelectedShow = 0;
		std::string mErrorString;
	};

	/**
	 * Data needed for handling insertion of event segments
	 */
	class SequenceGUIInsertEventSegment : public SequenceGUIActionData
	{
	public:
		SequenceGUIInsertEventSegment(std::string id, double aTime)
			: mTrackID(id), mTime(aTime){}

		std::string mTrackID;
		double mTime;
		std::string mEventMessage = "Hello world";
	};

	/**
	 * Data needed for editing event segments
	 */
	class SequenceGUIEditEventSegment : public SequenceGUIActionData
	{
	public:
		SequenceGUIEditEventSegment( std::string trackId,std::string segmentID,std::string message,ImVec2 windowPos)
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

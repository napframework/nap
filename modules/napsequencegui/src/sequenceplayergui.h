#pragma once

// internal includes
#include "sequenceplayer.h"

// external includes
#include <nap/resource.h>
#include <nap/resourceptr.h>
#include <rtti/objectptr.h>
#include <imgui/imgui.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	// forward declares
	class SequencePlayerGUIView;
	class SequencePlayerView;
	class SequencePlayerGUIActionData;

	/**
	 * SequenceEditorGUI
	 * A GUI resource that can be instantiated to draw a GUI (view) for the sequence player
	 */
	class NAPAPI SequencePlayerGUI : public Resource
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
		ResourcePtr<SequencePlayer> mSequencePlayer = nullptr; ///< Property: 'Sequence Player' link to player resource
	protected:
		// instatiated view
		std::unique_ptr<SequencePlayerGUIView> mView = nullptr;
	public:
	};

	/**
	 * Types of possible interactions with GUI
	 * Used by the gui state to handle mouse input / popups / actions
	 */
	namespace SequencePlayerActions
	{
		enum SequencePlayerActions
		{
			// ACTIONS
			HOVERING_PLAYER_TIME,
			DRAGGING_PLAYER_TIME,
			LOAD,
			NONE
		};
	}

	/**
	 * SequenceEditorGUIState
	 * Class holding state information for current Player GUI
	 */
	class SequencePlayerGUIState
	{
	public:
		/**
		 * default constructor
		 */
		SequencePlayerGUIState() = default;

		/**
		 * deconstructor
		 */
		~SequencePlayerGUIState() {}

		// current action
		SequencePlayerActions::SequencePlayerActions currentAction = SequencePlayerActions::NONE;

		// current action data
		std::unique_ptr<SequencePlayerGUIActionData> currentActionData = nullptr;
	};

	using drawSegmentContentFunction = std::function<void(const SequenceTrack&, const SequenceTrackSegment &, const ImVec2& , float , float , float , ImDrawList* , bool)>;
	using drawTrackFunction = std::function<void(const SequenceTrack&, ImVec2&, const float, const SequencePlayer&)>;
	
	/**
	 * SequencePlayerGUIView
	 * Responsible for draw the GUI for the sequence player
	 * Needs reference to player
	 */
	class NAPAPI SequencePlayerGUIView
	{
	public:
		/**
		 * Constructor
		 * @param controller reference to player
		 * @param id id of the GUI resource, used to push ID by IMGUI
		 */
		SequencePlayerGUIView(SequencePlayer& player, std::string id);

		/**
		 * shows the interface
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
		void drawCurveTrack(const SequenceTrack &track,ImVec2 &cursorPos, const float marginBetweenTracks, const SequencePlayer &sequencePlayer);


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
		 * drawEventTrack
		 * draws event track
		 * @param track reference to track
		 * @param cursorPos imgui cursorposition
		 * @param marginBetweenTracks y margin between tracks
		 * @param sequencePlayer reference to sequence player
		 */
		void drawEventTrack(const SequenceTrack& track, ImVec2& cursorPos, const float marginBetweenTracks, const SequencePlayer& sequencePlayer);

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
		 * handleLoadPopup
		 * handles load popup
		 */
		void handleLoadPopup();
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
		 * formatTimeString
		 * formats time ( seconds ) to human readable time
		 * @param time time
		 * @return string with readable time
		 */
		std::string formatTimeString(double time);
	protected:
		// cache of curves
		std::unordered_map<std::string, std::vector<ImVec2>> mCurveCache;

		// reference to player
		SequencePlayer& mPlayer;
	protected:
		// action information
		SequencePlayerGUIState mPlayerAction;

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
		std::unordered_map<rttr::type, drawSegmentContentFunction> mDrawSegmentsMap;

		//
		std::unordered_map<rttr::type, drawTrackFunction> mDrawTracksMap;
	};

	/**
	 * SequencePlayerGUIActionData
	 * Base class for data needed for handling GUI actions
	 */
	class SequencePlayerGUIActionData
	{
	public:
		/**
	 	 * Constructor
 		 */
		SequencePlayerGUIActionData() {}

		/**
		 * Deconstructor
		 */
		virtual ~SequencePlayerGUIActionData() {}
	};

	/**
	 * Data needed for loading show
	 */
	class SequencePlayerGUILoadShowData : public SequencePlayerGUIActionData
	{
	public:
		SequencePlayerGUILoadShowData() {}

		int selectedShow = 0;
		std::string errorString;
	};

	/**
	 * Data needed for dragging player
	 */
	class SequenceGUIPlayerDragPlayerData : public SequencePlayerGUIActionData
	{
	public:
		SequenceGUIPlayerDragPlayerData(
			bool playerWasPlaying_,
			bool playerWasPaused_)
			: playerWasPlaying(playerWasPlaying_),
			playerWasPaused(playerWasPaused_) {}

		bool playerWasPlaying;
		bool playerWasPaused;
	};
}

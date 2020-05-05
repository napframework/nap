#pragma once

// internal includes
#include "sequenceeditor.h"
#include "sequencetrackview.h"

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
	class SequenceTrackView;

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
	namespace SequenceGUIActions
	{
		struct SequenceGUIAction { RTTI_ENABLE() };

		struct None : SequenceGUIAction { RTTI_ENABLE(SequenceGUIAction) };
		struct DraggingSegment : SequenceGUIAction { RTTI_ENABLE(SequenceGUIAction) };
		struct InsertingSegment : SequenceGUIAction { RTTI_ENABLE(SequenceGUIAction) };
		struct OpenInsertSegmentPopup : SequenceGUIAction { RTTI_ENABLE(SequenceGUIAction) };
		struct EditingSegment : SequenceGUIAction { RTTI_ENABLE(SequenceGUIAction) };
		struct OpenEditSegmentPopup : SequenceGUIAction { RTTI_ENABLE(SequenceGUIAction) };
		struct HoveringSegment : SequenceGUIAction { RTTI_ENABLE(SequenceGUIAction) };
		struct HoveringSegmentValue : SequenceGUIAction { RTTI_ENABLE(SequenceGUIAction) };
		struct DraggingSegmentValue : SequenceGUIAction { RTTI_ENABLE(SequenceGUIAction) };
		struct HoveringPlayerTime : SequenceGUIAction { RTTI_ENABLE(SequenceGUIAction) };
		struct DraggingPlayerTime : SequenceGUIAction { RTTI_ENABLE(SequenceGUIAction) };
		struct OpenInsertTrackPopup : SequenceGUIAction { RTTI_ENABLE(SequenceGUIAction) };
		struct InsertingTrack : SequenceGUIAction { RTTI_ENABLE(SequenceGUIAction) };
		struct LoadPopup : SequenceGUIAction { RTTI_ENABLE(SequenceGUIAction) };
		struct SaveAsPopup : SequenceGUIAction { RTTI_ENABLE(SequenceGUIAction) };
	}

	/**
	 */
	class SequenceEditorGUIAction
	{
	public:
		/**
		 * default constructor
		 */
		SequenceEditorGUIAction() = default;

		/**
		 * deconstructor
		 */
		~SequenceEditorGUIAction() {}

		// current action
		SequenceGUIActions::SequenceGUIAction currentAction = SequenceGUIActions::None();

		// current object id, used to identify object with actions
		std::string currentObjectID = "";

		// current action data
		std::unique_ptr<SequenceGUIActionData> currentActionData = nullptr;
	};

	struct SequenceEditorGUIState
	{
	public:
		// action information
		SequenceEditorGUIAction mAction;

		//
		bool mDirty = false;

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

		ImVec2 mCursorPos;
	};

	/**
	 * SequenceEditorGUIView
	 * Responsible for draw the GUI for the sequence editor
	 * Needs reference to controller
	 */
	class NAPAPI SequenceEditorGUIView
	{
		friend class SequenceTrackView;
	public:
		/**
		 * Constructor
		 * @param controller reference to editor controller
		 * @param id id of the GUI resource, used to push ID by IMGUI
		 */
		SequenceEditorGUIView(SequenceEditor& editor, std::string id);

		/**
		 * shows the editor interface
		 */
		virtual void show();

		static bool registerTrackViewType(rttr::type trackType, rttr::type viewType);
	private:
		/**
		 * Draws the tracks of the sequence
		 * @param sequencePlayer reference to sequenceplayer
		 * @param sequence reference to sequence
		 */
		void drawTracks(const SequencePlayer& sequencePlayer, const Sequence &sequence);

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

		/**
		 * handleSaveAsPopup
		 * handles save as popup
		 */
		void handleSaveAsPopup();
	protected:
		// reference to controller
		SequenceEditor& mEditor;
	public:
		
	protected:
		SequenceEditorGUIState mState;

		// id
		std::string mID;

		//
		std::unordered_map<rttr::type, std::unique_ptr<SequenceTrackView>> mViews;
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
	 * Data needed for handling saving of sequences
	 */
	class SequenceGUISaveShowData : public SequenceGUIActionData
	{
	public:
		SequenceGUISaveShowData() {}

		int mSelectedShow = 0;
		std::string mErrorString;
	};
}

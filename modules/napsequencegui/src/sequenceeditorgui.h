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
		class Action
		{
			RTTI_ENABLE()
		public:
			template<typename T>
			bool isAction()
			{
				return this->get_type() == RTTI_OF(T);
			}

			template<typename T>
			T* getDerived()
			{
				assert(this->get_type() == RTTI_OF(T));
				return dynamic_cast<T*>(this);
			}
		};

		using SequenceActionPtr = std::unique_ptr<Action>;

		template<typename T, typename... Args>
		static SequenceActionPtr createAction(Args... args)
		{
			return std::make_unique<T>(args...);
		}

		class None : public Action { RTTI_ENABLE() };

		class DraggingSegment : public Action
		{ 
			RTTI_ENABLE(Action)
		public:
			DraggingSegment(std::string trackId, std::string segmentID)
				: mTrackID(trackId), mSegmentID(segmentID) {}

			std::string mTrackID;
			std::string mSegmentID;

		};

		class InsertingSegment :
			public Action
		{
			RTTI_ENABLE(Action)
		public:
			InsertingSegment(std::string id, double time, rttr::type type)
				: mTrackID(id), mTime(time), mTrackType(type) {}

			std::string mTrackID;
			double mTime;
			rttr::type mTrackType;
		};


		class OpenInsertSegmentPopup :
			public Action
		{
			RTTI_ENABLE(Action)
		public:
			OpenInsertSegmentPopup(std::string id, double time, rttr::type type)
				: mTrackID(id), mTime(time), mTrackType(type) {}

			std::string mTrackID;
			double mTime;
			rttr::type mTrackType;
		};

		class EditingSegment : public Action { 
			RTTI_ENABLE(Action)
		public:
			EditingSegment(std::string trackID, std::string segmentID, rttr::type segmentType)
				: mTrackID(trackID), mSegmentID(segmentID), mSegmentType(segmentType)
			{

			}

			std::string mTrackID;
			std::string mSegmentID;
			rttr::type mSegmentType;
		};

		class OpenEditSegmentPopup : public Action
		{
			RTTI_ENABLE(Action)
		public:
			OpenEditSegmentPopup(std::string trackID, std::string segmentID, rttr::type segmentType)
				: mTrackID(trackID), mSegmentID(segmentID), mSegmentType(segmentType)
			{

			}

			std::string mTrackID;
			std::string mSegmentID;
			rttr::type mSegmentType;
		};
		
		class HoveringSegment : public Action 
		{
			RTTI_ENABLE(Action) 
		public:
			HoveringSegment(std::string trackId, std::string segmentID)
				: mTrackID(trackId), mSegmentID(segmentID) {}

			std::string mTrackID;
			std::string mSegmentID;
		};

		class HoveringSegmentValue : public Action 
		{ 
			RTTI_ENABLE(Action) 
		public:
			HoveringSegmentValue(std::string trackId, std::string segmentID, SequenceEditorTypes::SegmentValueTypes type, int curveIndex)
				: mTrackID(trackId), mSegmentID(segmentID), mType(type), mCurveIndex(curveIndex) {}

			std::string mTrackID;
			std::string mSegmentID;
			SequenceEditorTypes::SegmentValueTypes mType;
			int mCurveIndex;
		};

		class DraggingSegmentValue : 
			public Action 
		{ 
			RTTI_ENABLE(Action) 
		public:
			DraggingSegmentValue(std::string trackId, std::string segmentID, SequenceEditorTypes::SegmentValueTypes type, int curveIndex)
				: mTrackID(trackId), mSegmentID(segmentID), mType(type), mCurveIndex(curveIndex) {}

			std::string mTrackID;
			std::string mSegmentID;
			SequenceEditorTypes::SegmentValueTypes mType;
			int mCurveIndex;
		};

		class HoveringPlayerTime : public Action { RTTI_ENABLE(Action) };
		class DraggingPlayerTime : public Action 
		{ 
			RTTI_ENABLE(Action)
		public:
			DraggingPlayerTime(bool wasPlaying, bool wasPaused)
				: mWasPlaying(wasPlaying), mWasPaused(wasPaused) {}
			
			bool mWasPlaying; bool mWasPaused;
		};

		class OpenInsertTrackPopup : public Action { RTTI_ENABLE(Action) };
		class InsertingTrack : public Action { RTTI_ENABLE(Action) };

		class LoadPopup : public Action 
		{ 
			RTTI_ENABLE(Action) 
		public:
			int mSelectedShowIndex = 0;
			std::string mErrorString;
		};

		class SaveAsPopup : public Action 
		{
			RTTI_ENABLE(Action)
		public:
			int mSelectedShowIndex = 0;
			std::string mErrorString;
		};
	}

	struct SequenceEditorGUIState
	{
	public:
		// action information
		SequenceGUIActions::SequenceActionPtr mAction = nullptr;

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
}

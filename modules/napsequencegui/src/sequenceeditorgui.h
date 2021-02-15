/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// internal includes
#include "sequenceeditor.h"
#include "sequencetrackview.h"
#include "sequenceeditorguistate.h"
#include "sequenceeditorguiactions.h"
#include "sequenceeditorguiclipboard.h"

// external includes
#include <imgui/imgui.h>
#include <nap/resource.h>
#include <nap/resourceptr.h>
#include <renderwindow.h>
#include <rtti/objectptr.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	// forward declares
	class SequenceEditorGUIView;
	class SequenceEditorView;
	class SequenceTrackView;

	/**
	 * A GUI resource that can be instantiated to draw a GUI (view) for the sequence editor
	 */
	class NAPAPI SequenceEditorGUI : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		/**
		 * @param errorState contains any errors
		 * @return true on success
		 */
		virtual bool init(utility::ErrorState& errorState);

		/**
		 * called before deconstruction
		 */
		virtual void onDestroy();

		/**
		 * Call this method to draw the GUI
		 */
		virtual void show();
	public:
		// properties
		ResourcePtr<RenderWindow> mRenderWindow = nullptr;
		ResourcePtr<SequenceEditor> mSequenceEditor = nullptr; ///< Property: 'Sequence Editor' link to editor resource
		bool mDrawFullWindow = false; ///< Property: 'Draw Full Window' if true, gui will span entire window size
	protected:
		// instantiated view
		std::unique_ptr<SequenceEditorGUIView> mView = nullptr;
	};

	/**
	 * Responsible for drawing the GUI for the sequence editor
	 * Needs reference to editor
	 */
	class NAPAPI SequenceEditorGUIView
	{
		friend class SequenceTrackView;
	public:
		/**
		 * Constructor
		 * @param editor reference to editor
		 * @param id id of the GUI resource, used to push ID by IMGUI
		 * @param renderWindow the render window
		 * @param drawFullWindow if the editor occupies the entire window space
		 */
		SequenceEditorGUIView(SequenceEditor& editor, std::string id, RenderWindow* renderWindow, bool drawFullWindow);

		/**
		 * shows the editor interface
		 */
		virtual void show();

		/**
		 * static method for registering a view type that draws the appropriate track type
		 */
		static bool registerTrackViewType(rttr::type trackType, rttr::type viewType);

		/**
		 * returns view that corresponds to a certain track type, asserts when not found
		 * @param type the track type
		 * @return the view type
		 */
		static rttr::type getViewForTrackType(rttr::type type);
	protected:
		/**
		 * Draws the tracks of the sequence
		 * @param sequencePlayer reference to sequenceplayer
		 * @param sequence reference to sequence
		 */
		void drawTracks(const SequencePlayer& sequencePlayer, const Sequence &sequence);

		/**
		 * Draws inspectors of the sequence tracks
		 * @param sequencePlayer reference to sequenceplayer
		 * @param sequence reference to sequence
		 */
		void drawInspectors(const SequencePlayer& sequencePlayer, const Sequence &sequence);

		/**
		 * Draws markers
		 * @param sequencePlayer reference to sequenceplayer
		 * @param sequence reference to sequence
		 */
		void drawMarkers(const SequencePlayer& sequencePlayer, const Sequence &sequence );

		/**
		 * Draw lines of markers
		 * @param sequencePlayer reference to sequenceplayer
		 * @param sequence reference to sequence
		 */
		void drawMarkerLines(const Sequence& sequence, SequencePlayer& player);

		/**
		 * draws player controller bar
		 * @param player reference to player
		 */
		void drawPlayerController(SequencePlayer& player);

		/**
		 * draws line of player position
		 * @param sequence reference to sequence
		 * @param player reference to player
		 */
		void drawTimelinePlayerPosition(const Sequence& sequence, SequencePlayer& player);

		/**
		 * draws end of sequence
		 * @param sequence reference to sequence
		 * @param player reference to player
		 */
		void drawEndOfSequence(const Sequence& sequence, SequencePlayer& player);

		/**
		 * Handles insertion of track popup
		 */
		void handleInsertTrackPopup();

		/**
		 * handles load popup
		 */
		void handleLoadPopup();

		/**
		 * handles save as popup
		 */
		void handleSaveAsPopup();

		/**
		 * handle editing of sequence duration
		 */
		void handleSequenceDurationPopup();

		/*
		 * handle editing of markers
		 */
		void handleEditMarkerPopup();

		/*
		 * handle insertion of new markers
		 */
		void handleInsertMarkerPopup();
	protected:
		// reference to editor
		SequenceEditor& mEditor;

		// holds current state information
		SequenceEditorGUIState mState;

		// id
		std::string mID;

		// map of all track views
		std::unordered_map<rttr::type, std::unique_ptr<SequenceTrackView>> mViews;

		//
		bool mDrawFullWindow = false;

		RenderWindow* mRenderWindow = nullptr;

		std::unordered_map<rttr::type, std::function<void()>> mPopups;
	};
}

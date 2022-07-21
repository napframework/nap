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
#include "sequenceguiservice.h"

// external includes
#include <imgui/imgui.h>
#include <nap/resource.h>
#include <nap/resourceptr.h>
#include <renderwindow.h>
#include <rtti/objectptr.h>
#include <imagefromfile.h>

namespace nap
{
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
        SequenceEditorGUI(SequenceGUIService& service);

        /**
         * @param errorState contains any errors
         * @return true on success
         */
        bool init(utility::ErrorState& errorState) override;

        /**
         * called before deconstruction
         */
        void onDestroy() override;

        /**
         * Shows the editor interface
         * @param newWindow when true interface will be drawn in a new ImGUI window
         */
        virtual void show(bool newWindow = true);


        /**
         * @return sequence editor gui service
         */
        SequenceGUIService& getService() { return mService; }


        // properties
        ResourcePtr<RenderWindow> mRenderWindow = nullptr;
        ResourcePtr<SequenceEditor> mSequenceEditor = nullptr; ///< Property: 'Sequence Editor' link to editor resource
        bool mDrawFullWindow = false; ///< Property: 'Draw Full Window' if true, gui will span entire window size
        bool mHideMarkerLabels = false; ///< Property: 'Hide Marker Labels' if true, hides marker labels when not hovered
    protected:
        // instantiated view
        std::unique_ptr<SequenceEditorGUIView> mView = nullptr;

        // reference to service
        SequenceGUIService &mService;
    };

    using SequenceEditorGUIObjectCreator = rtti::ObjectCreator<SequenceEditorGUI, SequenceGUIService>;

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
        SequenceEditorGUIView(SequenceGUIService& service, SequenceEditor& editor, std::string id, RenderWindow* renderWindow, bool drawFullWindow);

        /**
         * Shows the editor interface
         * @param newWindow when true interface will be drawn in a new ImGUI window
         */
        virtual void show(bool newWindow = true);


        /**
         * Hides marker labels when not hovered
         * @param hide when true, hides marker labels when not hovered
         */
        void hideMarkerLabels(bool hide){ mHideMarkerLabels = hide; }


        SequenceGUIService& getService() { return mService; }


    protected:
        /**
         * Draws the tracks of the sequence
         * @param sequencePlayer reference to sequenceplayer
         * @param sequence reference to sequence
         */
        void drawTracks(const SequencePlayer& sequencePlayer, const Sequence& sequence);

        /**
         * Draws inspectors of the sequence tracks
         * @param sequencePlayer reference to sequenceplayer
         * @param sequence reference to sequence
         */
        void drawInspectors(const SequencePlayer& sequencePlayer, const Sequence& sequence);

        /**
         * Draws markers
         * @param sequencePlayer reference to sequenceplayer
         * @param sequence reference to sequence
         */
        void drawMarkers(const SequencePlayer& sequencePlayer, const Sequence& sequence);

        /**
         * Draw lines of markers
         * @param sequencePlayer reference to sequenceplayer
         * @param sequence reference to sequence
         */
        void drawMarkerLines(const Sequence& sequence, SequencePlayer& player) const;

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
        void drawTimelinePlayerPosition(const Sequence& sequence, SequencePlayer& player) const;

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

        /**
         * handle editing of markers
         */
        void handleEditMarkerPopup();

        /**
         * handle insertion of new markers
         */
        void handleInsertMarkerPopup();

        /**
         * when zooming, zoom around the center of the timeline, keeping the focus in the middle
         */
        void handleHorizontalZoom();

        /**
         * when show help popup is pressed, show modal help popup
         */
        void handleHelpPopup();

        /**
         * when save clipboard is pressed, show save clipboard popup
         */
        void handleSaveClipboardPopup();

        /**
         * registers handlers for actions
         * @param actionType the action type to register a handler function for
         * @param action the handler function
         */
        void registerActionHandler(const rttr::type& actionType, const std::function<void()>& action);

    protected:
        void registerActionHandlers();

        // reference to editor
        SequenceEditor &mEditor;

        // holds current gui state information
        SequenceEditorGUIState mState;

        // id
        std::string mID;

        // set to true if we draw full window
        bool mDrawFullWindow = false;

        // set to true when marker labels should be hidden when not hovered
        bool mHideMarkerLabels = false;

        // pointer to render window
        RenderWindow *mRenderWindow = nullptr;

        // map of action handlers
        std::unordered_map<rttr::type, std::function<void()>> mActionHandlers;

        std::unordered_map<rttr::type, std::unique_ptr<SequenceTrackView>> mViews;

        // reference to service
        SequenceGUIService &mService;
    };
}

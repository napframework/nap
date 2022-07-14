/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "sequenceeditorgui.h"
#include "sequenceeditorguiactions.h"
#include "sequenceeditorguistate.h"
#include "sequenceguiservice.h"

// nap includes
#include <nap/core.h>

namespace nap
{
    //////////////////////////////////////////////////////////////////////////

    // forward declares
    class SequenceTrack;
    class SequenceTrackView;
    class SequenceEditorGUIView;

    /**
     * Base class of track views
     * Responsible for drawing a track of a specific type
     * Needs to be extended for each track type. F.E. SequenceCurveTrackView is responsible for drawing curve tracks
     */
    class NAPAPI SequenceTrackView
    {
    RTTI_ENABLE()
    public:
        /**
         * Constructor
         * @param view reference to view
         * @param state object that contains the active gui state
         */
        SequenceTrackView(SequenceEditorGUIView &view, SequenceEditorGUIState &state);

        /**
         * Destructor
         */
        virtual ~SequenceTrackView() = default;

        /**
         * handles any actions that are created upon show
         * this might be useful when certain actions in tracks are overlapping and/ or when we create an action that
         * needs to do something in the next frame update
         */
        virtual void handleActions();

        /**
         * shows inspector block
         * @param track reference to track
         * @param deleteTrack reference to bool to indicate whether delete is pressed
         * @param moveUp reference to bool indicating to move the track upwards
         * @param moveDown reference to bool indicating to move the track downwards
         */
        virtual void showInspector(const SequenceTrack &track);

        /**
         * shows track contents
         * @param track reference to track
         */
        virtual void showTrack(const SequenceTrack &track);

        /////////////////////////////////////////////////////////////////////////////
        // static utility methods
        ////////////////////////////////////////////////////////////////////////////

        /**
         * Combobox that takes std::vector as input
         * @param label label of box
         * @param currIndex current index of combo box
         * @param values vector of string values
         * @return true if something is selected
         */
        static bool Combo(const char *label, int *currIndex, std::vector<std::string> &values);

        /**
         * ListBox that takes std::vector as input
         * @param label label of box
         * @param currIndex current index of combo box
         * @param values vector of string values
         * @return true if something is selected
         */
        static bool ListBox(const char *label, int *currIndex, std::vector<std::string> &values);

        /**
         * formats time ( seconds ) to human readable time
         * @param time time
         * @return string with readable time
         */
        static std::string formatTimeString(double time);

    protected:
        /**
         * Handles resizing of track height
         */
        void handleResizeTrackHeight();

        /**
         * this methods needs to be overloaded, contents for the inspector of a specific track type can be drawn in this function
         * @param track reference to track
         */
        virtual void showInspectorContent(const SequenceTrack &track) = 0;

        /**
         * this method needs to be overloaded, contents for the track can be drawn in this function
         * @param track reference to track
         * @param trackTopLeft orientation for drawing stuff in track window
         */
        virtual void showTrackContent(const SequenceTrack &track, const ImVec2 &trackTopLeft) = 0;

        /**
         * registers an action handler for a certain action
         * @param type action type info
         * @param handler handler function
         */
        void registerActionHandler(const rttr::type &type, const std::function<void()> &handler);

        // reference to gui view
        SequenceEditorGUIView &mView;

        // reference to sequence player
        const SequencePlayer &getPlayer();

        // reference to editor
        SequenceEditor &getEditor();

        // reference to gui state
        SequenceEditorGUIState &mState;
        // map of action handlers
        std::unordered_map<rttr::type, std::function<void()>> mActionHandlers;
        // reference to service
        SequenceGUIService &mService;
    };
}

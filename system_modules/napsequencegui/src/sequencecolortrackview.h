/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "sequencetrackview.h"
#include "sequencecontrollercolor.h"
#include "sequencetracksegment.h"
#include "sequenceeditorguiclipboard.h"
#include "sequencecolortrackview_guiactions.h"
#include "sequenceguiutils.h"

#include <utility>
#include <imguiutils.h>

namespace nap
{
    //////////////////////////////////////////////////////////////////////////

    // forward declares
    class SequenceColorTrackView;
    class SequenceGUIService;

    class NAPAPI SequenceColorTrackSegmentView final
    {
    RTTI_ENABLE()
    public:
        /**
         * Constructor
         */
        SequenceColorTrackSegmentView() = default;

        virtual ~SequenceColorTrackSegmentView() = default;

        bool handleEditPopupContent(sequenceguiactions::Action& action);

        void drawEvent(const SequenceTrackSegment& segment, ImDrawList* drawList, const ImVec2& topLeft, float x, ImU32 color);

        void insertSegment(SequenceControllerColor& controller, const std::string& trackID, double time);
    };


    class NAPAPI SequenceColorTrackView final : public SequenceTrackView
    {
        friend class SequenceGUIService;

    RTTI_ENABLE(SequenceTrackView)
    public:
        /**
         * Constructor
         * @param service reference to gui service
         * @param view reference to editor view
         * @param state reference to editor state
         */
        SequenceColorTrackView(SequenceGUIService& service, SequenceEditorGUIView& view, SequenceEditorGUIState& state);

        // make this class explicitly non-copyable
        SequenceColorTrackView(const SequenceColorTrackView&) = delete;
        SequenceColorTrackView& operator=(const SequenceColorTrackView&) = delete;

    protected:
        /**
         * Opens edit color segment popup
         * @param track reference to track
         * @param segment reference to segment
         */
        void onHandleEditSegment(const nap::SequenceTrack &track, const nap::SequenceTrackSegment &segment) override;

        /**
         * Overridden function to draw segment handler,
         * @param top top position of handler
         * @param bottom bottom position of handler
         * @param drawList drawlist to draw on
         * @param inClipboard if true, segment is in clipboard
         * @param bold if true, draw handler bold
         */
        void onDrawSegmentHandler(ImVec2 top, ImVec2 bottom, ImDrawList *drawList, bool inClipboard, bool bold) override;

        /**
         * Adds segment to clipboard
         * @param track reference to track
         * @param segment reference to segment
         */
        void onAddSegmentToClipboard(const nap::SequenceTrack &track, const nap::SequenceTrackSegment &segment) override;

        /**
         * shows inspector content
         * @param track reference to track
         */
        void showInspectorContent(const SequenceTrack& track) override;

        /**
         * shows track contents
         * @param track reference to track
         * @param trackTopLeft orientation
         */
        void showTrackContent(const SequenceTrack& track, const ImVec2& trackTopLeft) override;

        /**
         * handles insert event segment popup
         */
        void handleInsertColorSegmentPopup();

        /**
         * handles pasting of clipboard content to event segment
         * @tparam T the event segment type
         * @param trackID the track id of where to paste the new event
         * @param time the time where to insert copied event segment
         * @param errorState contains any errors
         * @return true on succes
         */
        bool pasteFromClipboard(const std::string& trackID, double time, utility::ErrorState& errorState);

        /**
         * Called before call to showTrack during the same frame
         * Clears the cache of points and polylines
         */
        void onPreShowTrack() override;

        /**
         * handles delete segment popup
         */
        void handleEditSegmentValuePopup();

        /**
         * update the segment in the clipboard
         * @param trackID the track id of the track containing the segment
         * @param segmentID the segment id
         */
        void updateSegmentInClipboard(const std::string& trackID, const std::string& segmentID);


        /**
         * handles assigning of new output id to track
         */
        void handleAssignOutputIDToTrack();

        /**
         * handles dragging of event segment
         */
        void handleSegmentDrag();

        /**
         * handles the load preset popup
         */
        void handleLoadPresetPopup();

        /**
         * handles the edit curve popup
         */
        void handleEditCurvePopup();

        /**
         * handles the drag curve point popup
         */
        void handleDragCurvePoint();

        /**
         * handles the edit curve point popup
         */
        void handleEditCurvePointPopup();

        /**
         * handles the drag curve tan point popup
         */
        void handleDragCurveTanPoint();
    private:
        // map of segment views for different event views
        std::unordered_map<rtti::TypeInfo, std::unique_ptr<SequenceColorTrackSegmentView>> mSegmentViews;

        /**
         * Internal struct that caches points for drawing
         */
        struct CachePoint
        {
            ImU32 mColorStart;
            ImU32 mColorEnd;
            ImVec2 mRectStart;
            ImVec2 mRectEnd;
        };

        // caches
        std::unordered_map<std::string, std::vector<CachePoint>> mCachePoints;
        std::unordered_map<std::string, std::vector<ImVec2>> mCachedPolylines;
    };


    //////////////////////////////////////////////////////////////////////////
    // Event Clipboards
    //////////////////////////////////////////////////////////////////////////

    namespace sequenceguiclipboard
    {
        class ColorSegmentClipboard :
                public Clipboard
        {
        RTTI_ENABLE(Clipboard)
        public:
            ColorSegmentClipboard(const rttr::type& type, std::string sequenceName)
                    : Clipboard(type), mSequenceName(std::move(sequenceName))
            {};


            const std::string &getSequenceName() const{ return mSequenceName; }

        private:
            std::string mSequenceName;
        };
    }
}

/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <utility>

#include "sequencetrackview.h"
#include "sequencetracksegmentcurve.h"
#include "sequencecontrollercurve.h"
#include "sequenceguiservice.h"

namespace nap
{
    //////////////////////////////////////////////////////////////////////////

    // forward declares
    class SequencePlayerAudioOutput;
    class SequenceAudioGUIService;

    /**
     * The SequenceAudioTrackView is created by the SequenceEditorGUI for SequenceAudioTrack types
     * The SequenceAudioTrackView is responsible for displaying SequenceTrackAudio data
     */
    class NAPAPI SequenceAudioTrackView final : public SequenceTrackView
    {
        RTTI_ENABLE(SequenceTrackView)
    public:
        /**
         * Constructor
         * @param service reference to SequenceGUIService
         * @param view reference to SequenceEditorGUIView
         * @param state reference to SequenceEditorGUIState
         */
        SequenceAudioTrackView(SequenceGUIService& service, SequenceEditorGUIView& view, SequenceEditorGUIState& state);

        // make this class explicitly non-copyable
        SequenceAudioTrackView(const SequenceAudioTrackView&) = delete;

        SequenceAudioTrackView& operator=(const SequenceAudioTrackView&) = delete;

    protected:
        /**
         * showInspectorContent is called when inspector for AudioTrack needs to be drawn
         * @param track reference to SequenceTrack, must be of SequenceAudioTrack type
         */
        virtual void showInspectorContent(const SequenceTrack& track) override;

        /**
         * showTrackContent is called when contents of AudioTrack needs to be drawn
         * @param track reference to SequenceTrack, must be of SequenceAudioTrack type
         * @param trackTopLeft top left position of window containing the view
         */
        virtual void showTrackContent(const SequenceTrack& track, const ImVec2& trackTopLeft) override;

    private:
        /**
         * handles action when output id of track needs to be changed
         */
        void handleAssignOutputIDToTrack();

        /**
         * handles dragging of segment in view
         */
        void handleSegmentDrag();

        /**
         * handles insertion of segments
         */
        void handleSegmentInsert();

        /**
         * handles edit segment popup
         */
        void handleEditSegment();

        /**
         * handles dragging of left handler of audio segment
         */
        void handleLeftHandlerDrag();

        /**
         * handles dragging of right handler of audio segment
         */
        void handleRightHandlerDrag();

    private:
        /**
         * cache of drawn waveforms
         */
        std::unordered_map<std::string, std::vector<std::vector<ImVec2>>> mWaveformCache;

        /**
         * returns audio output assigned to certain track
         * @param trackID the track id
         * @return pointer to audio output, can be nullptr when not found
         */
        SequencePlayerAudioOutput* getAudioOutputForTrack(const std::string& trackID);

        /**
         * returns vector of audio buffers ids that can be assigned by an AudioTrack
         * @param trackID the track id
         * @return vector of audio buffers ids that can be assigned by an AudioTrack
         */
        std::vector<std::string> getAudioBuffersForTrack(const std::string& trackID);

        // pointer to sequence audio gui service
        SequenceAudioGUIService* mAudioGUIService = nullptr;
    };

    //////////////////////////////////////////////////////////////////////////
    // Audio Clipboards
    //////////////////////////////////////////////////////////////////////////

    namespace sequenceguiclipboard
    {
        /**
         * An AudioSegmentClipboard can contain multiple serialized audio track segments
         */
        class AudioSegmentClipboard : public Clipboard
        {
            RTTI_ENABLE(Clipboard)
        public:
            /**
             * Constructor
             * @param type the segment type, must be of SequenceTrackSegmentAudio
             * @param sequenceName the sequence name
             */
            AudioSegmentClipboard(const rttr::type& type, std::string sequenceName)
                    :Clipboard(type), mSequenceName(std::move(sequenceName))
            {
            };


            /**
             * returns sequence name that contains serialized segments
             * @return sequence name
             */
            const std::string& getSequenceName() const
            {
                return mSequenceName;
            }


        private:
            std::string mSequenceName;
        };
    }
}

#include "sequenceaudiotrackview_guiactions.h"


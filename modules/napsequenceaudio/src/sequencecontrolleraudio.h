/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// nap includes
#include <sequencecontroller.h>
#include <nap/logger.h>
#include <audio/resource/audiobufferresource.h>

// local includes
#include "sequencetracksegmentaudio.h"

namespace nap
{
    //////////////////////////////////////////////////////////////////////////

    // forward declares
    class SequenceService;

    /**
     * The SequenceControllerAudio is responsible for manipulating, editing and inserting audio segments on an audio track
     */
    class NAPAPI SequenceControllerAudio final : public SequenceController
    {
        RTTI_ENABLE(SequenceController)
    public:
        /**
         * Constructor
         * @param service reference to sequence service
         * @param player reference to sequence player
         * @param editor reference to sequence editor
         */
        SequenceControllerAudio(SequenceService& service, SequencePlayer& player, SequenceEditor& editor)
                :SequenceController(service, player, editor)
        {
        }

        /**
         * Changes the start time of an audio segment on the track
         * @param trackID the track id of the track containing the segment
         * @param segmentID the segment id
         * @param time the new start time of the segment
         * @return the new start time of the segment
         */
        double segmentAudioStartTimeChange(const std::string& trackID, const std::string& segmentID, double time);

        /**
         * Inserts a segment, don;t use this function. Instead use insertAudioSegment
         * @param trackID track id of the track
         * @param time the time at which to insert the segment
         * @return raw pointer to newly inserted segment
         */
        const SequenceTrackSegment* insertSegment(const std::string& trackID, double time) override;

        /**
         * Deletes a segment from the track
         * @param trackID the id of the track containing the segment
         * @param segmentID the id of the segment to delete
         */
        void deleteSegment(const std::string& trackID, const std::string& segmentID) override;

        /**
         * Inserts an audio segment at given time with a certain buffer id. Returns id of newly inserted segment
         * @param trackID the id of the track where to insert the segment
         * @param time the time at when to insert the segment
         * @param audioBufferID the buffer id of the audio segment
         * @return id of the newly inserted segment
         */
        std::string insertAudioSegment(const std::string& trackID, double time, const std::string& audioBufferID);

        /**
         * Inserts an audio segment at given time with a certain buffer id, certain duration and a certain start time in
         * segment
         * @param trackID trackID the id of the track where to insert the segment
         * @param time the time at when to insert the segment
         * @param audioBufferID the buffer id of the audio segment
         * @param duration the duration of the segment
         * @param startTimeInSegment the start time within the segment
         * @return id of the newly inserted segment
         */
        std::string insertAudioSegment(const std::string& trackID, double time, const std::string& audioBufferID,
                                       double duration, double startTimeInSegment);

        /**
         * Adds a new audio track to the sequence
         */
        void addNewAudioTrack();

        /**
         * Inserts new track, type must be of SequenceTrackAudio
         * @param type the type of track to insert, type must be of SequenceTrackAudio
         */
        void insertTrack(rttr::type type) override;

        /**
         * Changes to buffer the audio segment points to
         * @param trackID track id of the track containing the segment
         * @param segmentID the segment id
         * @param audioBufferID the new audio segment buffer id
         */
        void changeAudioSegmentAudioBuffer(const std::string& trackID, const std::string& segmentID,
                                           const std::string& audioBufferID);

        /**
         * Changes the start time within the audio buffer of the audio segment
         * @param trackID trackID track id of the track containing the segment
         * @param segmentID segmentID the segment id
         * @param time the new start time within the audio buffer
         * @return the adjusted start time within the audio buffer
         */
        double segmentAudioStartTimeInSegmentChange(const std::string& trackID, const std::string& segmentID,
                                                    double time);

        /**
         * Changes the duration of the audio segment
         * @param trackID trackID track id of the track containing the segment
         * @param segmentID segmentID the segment id
         * @param newDuration the new duration of the audio buffer
         * @return the adjusted duration time
         */
        double segmentAudioDurationChange(const std::string& trackID, const std::string& segmentID, double newDuration);
    private:
        /**
         * Finds a certain audio buffer for the assigned output of an audio track
         * @param trackID the track id of the audio track
         * @param audioBufferID the buffer id
         * @return raw pointer to buffer resource
         */
        audio::AudioBufferResource* findAudioBufferForTrack(const std::string& trackID,
                                                            const std::string& audioBufferID);

        /**
         * Aligns audio segments of an audio track so they don't overlap
         * @param trackID the track id
         */
        void alignAudioSegments(const std::string& trackID);
    };
}
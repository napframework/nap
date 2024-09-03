/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "sequenceplayeraudioadapter.h"
#include "sequencetrackaudio.h"
#include "sequenceplayeraudiooutput.h"
#include "sequenceplayeraudioclock.h"

#include <sequenceplayer.h>
#include <nap/logger.h>

namespace nap
{
    SequencePlayerAudioAdapter::SequencePlayerAudioAdapter(const SequenceTrack& track,
                                                           SequencePlayerAudioOutput& output,
                                                           const SequencePlayer& player)
            :mTrack(track), mOutput(output), mPlayer(player)
    {
        mOutput.registerAdapter(this);

        /**
         * if clock is different then SequencePlayerAudioClock, updates don't occur on AudioThread and ignore any ticks
         */
        mDisabled = mPlayer.mClock.get()->get_type()!=RTTI_OF(SequencePlayerAudioClock);
    }


    void SequencePlayerAudioAdapter::tick(double time)
    {
        if (mDisabled)
            return;

        std::string started_segment_id = "";
        const auto& audio_track = static_cast<const SequenceTrackAudio&>(mTrack);

        // iterate trough audio segments
        for (const auto& segment : audio_track.mSegments)
        {
            // upcast to audio segment
            assert(segment.get()->get_type().is_derived_from(RTTI_OF(SequenceTrackSegmentAudio)));
            auto* segment_audio = static_cast<SequenceTrackSegmentAudio*>(segment.get());

            // check if time is inside audio segment
            if (time>=segment_audio->mStartTime && time<segment_audio->mStartTime+segment_audio->mDuration)
            {
                // get time in segment
                double time_in_segment = time-segment_audio->mStartTime+segment_audio->mStartTimeInAudioSegment;
                started_segment_id = segment_audio->mAudioBufferID;

                // if time is different, continue to play from the right position
                // else stop the player
                if (time!=mPrevTime)
                {
                    if (mCurrentStartedSegmentID!=started_segment_id)
                    {
                        mPrevTimeInSegment = time_in_segment;
                    }
                    else
                    {
                        mOutput.handleAudioSegmentPlay(this, segment_audio->mAudioBufferID, mPrevTimeInSegment, mPlayer.getPlaybackSpeed());

                        mPrevTimeInSegment = time_in_segment;
                    }
                }
                else
                {
                    mOutput.handleAudioSegmentStop(this, segment_audio->mAudioBufferID);
                }

                mPrevTime = time;
                break;
            }
        }

        // if we changed audio segment id, stop the previous segment
        if (started_segment_id!=mCurrentStartedSegmentID)
        {
            if (!mCurrentStartedSegmentID.empty())
            {
                mOutput.handleAudioSegmentStop(this, mCurrentStartedSegmentID);
            }
            mCurrentStartedSegmentID = started_segment_id;
        }
    }


    void SequencePlayerAudioAdapter::destroy()
    {
        if (!mCurrentStartedSegmentID.empty())
        {
            mOutput.handleAudioSegmentStop(this, mCurrentStartedSegmentID);
        }

        mOutput.unregisterAdapter(this);
    }
}

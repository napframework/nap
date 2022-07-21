/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// local includes
#include "sequenceplayer.h"
#include "sequenceplayeradapter.h"
#include "sequenceplayereventoutput.h"
#include "sequencetracksegmentevent.h"

namespace nap
{
    //////////////////////////////////////////////////////////////////////////

    // forward declares
    class SequencePlayerEventOutput;

    /**
     * Adapter responsible for handling events from an event track and sync them with the main thread using a
     * SequencePlayerEventOutput intermediate class.
     */
    class NAPAPI SequencePlayerEventAdapter final : public SequencePlayerAdapter
    {
    public:
        /**
         * Constructor
         * @param track reference to sequence event track
         * @param output reference to event receiver
         * @param player reference to the sequence player
         */
        SequencePlayerEventAdapter(const SequenceTrack& track, SequencePlayerEventOutput& output,
                                   const SequencePlayer& player);

        /**
         * called from sequence player thread
         * @param time time in sequence player
         */
        void tick(double time) override;


        /**
         * called upon destruction of the adapter
         */
        void destroy() override{};
    private:
        // reference to track linked to adapter
        const SequenceTrack& mTrack;

        // reference to output linked to adapter
        SequencePlayerEventOutput& mOutput;

        // list of dispatched events
        std::unordered_set<SequenceTrackSegmentEventBase*> mDispatchedEvents;

        //
        bool mPlayingBackwards = false;

        //
        double mPrevTime = 0.0;
    };
}

#pragma once
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// local includes
#include "sequenceplayer.h"
#include "sequenceplayeradapter.h"
#include "sequenceplayercoloroutput.h"

namespace nap
{
    //////////////////////////////////////////////////////////////////////////

    // forward declares
    class SequencePlayerColorOutput;

    /**
     */
    class NAPAPI SequencePlayerColorAdapter final : public SequencePlayerAdapter
    {
    public:
        /**
         * Constructor
         * @param track reference to sequence color track
         * @param output reference to event receiver
         * @param player reference to the sequence player
         */
        SequencePlayerColorAdapter(const SequenceTrack& track,
                                   SequencePlayerColorOutput& output,
                                   const SequencePlayer& player);

        /**
         * called from sequence player thread
         * @param time time in sequence player
         */
        void tick(double time) override;

        /**
         * called when adapter is destroyed
         */
        void destroy() override{}
    private:
        const SequencePlayer& mPlayer;

        // reference to track linked to adapter
        const SequenceTrack& mTrack;

        // reference to output linked to adapter
        SequencePlayerColorOutput& mOutput;

        // set or store, depending on main thread or player thread
        std::function<void(const RGBAColorFloat&)> mSetFunction;
    };
}

/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// local includes
#include "sequencetrack.h"

// external includes
#include <nap/resource.h>

namespace nap
{
    //////////////////////////////////////////////////////////////////////////

    // forward declares
    class SequencePlayerOutput;
    class SequencePlayerAdapter;
    class SequencePlayer;

    /**
     * A SequencePlayerAdapter can be created by the SequencePlayer and syncs with the player thread
     * Typically, a SequencePlayerAdapter is responsible for doing something with a track while the player is playing
     */
    class NAPAPI SequencePlayerAdapter
    {
    public:
        /**
         * Constructor
         */
        SequencePlayerAdapter() = default;

        /**
         * called from sequence player thread
         * @param time time in sequence player
         */
        virtual void tick(double time) = 0;

        /**
         * called when sequence player is stopped and adapter needs to be destroyed
         */
        virtual void destroy() = 0;
    };
}
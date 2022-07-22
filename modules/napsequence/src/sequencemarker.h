/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// external includes
#include <nap/resource.h>

namespace nap
{
    //////////////////////////////////////////////////////////////////////////

    /**
     * SequenceMarker can be used to mark certain positions in the sequencer
     * This can be a useful tool for the user to maintain an overview within created sequences
     */
    class NAPAPI SequenceMarker : public Resource
    {
    RTTI_ENABLE(Resource)
    public:
        ~SequenceMarker() override = default;

        std::string mMessage; ///< Property: 'Message' message in marker
        double mTime;  ///< Property: 'Time' time in seconds of marker in sequence
    };
}

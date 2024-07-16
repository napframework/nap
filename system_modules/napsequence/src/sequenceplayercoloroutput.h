/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// local includes
#include "sequenceplayeroutput.h"
#include "sequenceevent.h"

// nap includes
#include <nap/resourceptr.h>
#include <nap/signalslot.h>
#include <parametercolor.h>

// external includes
#include <queue>

namespace nap
{
    //////////////////////////////////////////////////////////////////////////

    // forward declares
    class SequenceService;

    class NAPAPI SequencePlayerColorOutput final : public SequencePlayerOutput
    {
        friend class SequencePlayerColorAdapter;

    RTTI_ENABLE(SequencePlayerOutput);
    public:
        /**
         * Constructor
         * @param service reference to SequenceService
         */
        SequencePlayerColorOutput(SequenceService& service);

        ResourcePtr<ParameterRGBColorFloat> mParameter;
        bool mUseMainThread = true;
    protected:
        /**
         * called from sequence service main thread
         * @param deltaTime time since last update
         */
        void update(double deltaTime) override;

    private:
    };

    using SequencePlayerColorOutputObjectCreator = rtti::ObjectCreator<SequencePlayerColorOutput, SequenceService>;
}
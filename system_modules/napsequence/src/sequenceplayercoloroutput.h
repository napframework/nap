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
#include <concurrentqueue.h>

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

        ResourcePtr<ParameterRGBAColorFloat> mParameter; ///< Property: 'Parameter' parameter resource
        bool mUseMainThread = true; ///< Property: 'Use Main Thread' update in main thread or player thread
    protected:
        /**
         * called from sequence service main thread
         * @param deltaTime time since last update
         */
        void update(double deltaTime) override;

    private:
        /**
         * stores color in queue
         * @param color color to store
         */
        void storeColor(const RGBAColorFloat& color);

        /**
         * sets color
         * @param color color to set
         */
        void setColor(const RGBAColorFloat& color);

        moodycamel::ConcurrentQueue<RGBAColorFloat> mColorQueue;
    };

    using SequencePlayerColorOutputObjectCreator = rtti::ObjectCreator<SequencePlayerColorOutput, SequenceService>;
}
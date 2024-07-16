/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "sequenceplayercoloroutput.h"
#include "sequenceservice.h"
#include "sequencetrackevent.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SequencePlayerColorOutput, "Links a parameter to a curve track")
        RTTI_PROPERTY("Parameter", &nap::SequencePlayerColorOutput::mParameter, nap::rtti::EPropertyMetaData::Required, "Parameter to update")
        RTTI_PROPERTY("Use Main Thread", &nap::SequencePlayerColorOutput::mUseMainThread, nap::rtti::EPropertyMetaData::Default, "Update parameter from the main thread, instead of the player thread")
RTTI_END_CLASS


namespace nap
{
    SequencePlayerColorOutput::SequencePlayerColorOutput(SequenceService& service)
            : SequencePlayerOutput(service)
    {
    }


    void SequencePlayerColorOutput::update(double deltaTime)
    {
    }
}

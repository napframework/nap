/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// local includes
#include "sequencemarker.h"

// Define base class
RTTI_DEFINE_BASE(nap::SequenceMarker)

// Define sequence events
RTTI_BEGIN_CLASS(nap::SequenceMarker, "Adds information to a sequence at a certain position")
        RTTI_PROPERTY("Message", &nap::SequenceMarker::mMessage, nap::rtti::EPropertyMetaData::Default, "The message");
        RTTI_PROPERTY("Time", &nap::SequenceMarker::mTime, nap::rtti::EPropertyMetaData::Default, "Time in seconds in the sequence");
RTTI_END_CLASS


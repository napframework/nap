/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// local includes
#include "sequencemarker.h"

// Define base class
RTTI_DEFINE_BASE(nap::SequenceMarker)

// Define sequence events
RTTI_BEGIN_CLASS(nap::SequenceMarker)
    RTTI_PROPERTY("Message", &nap::SequenceMarker::mMessage, nap::rtti::EPropertyMetaData::Default);
    RTTI_PROPERTY("Time", &nap::SequenceMarker::mTime, nap::rtti::EPropertyMetaData::Default);
RTTI_END_CLASS


/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// local includes
#include "sequenceevent.h"

// Define base class
RTTI_DEFINE_BASE(nap::SequenceEventBase)

// Define sequence events
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SequenceEventString)
        RTTI_CONSTRUCTOR(const std::string&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SequenceEventFloat)
        RTTI_CONSTRUCTOR(const float&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SequenceEventInt)
        RTTI_CONSTRUCTOR(const int&)
RTTI_END_CLASS


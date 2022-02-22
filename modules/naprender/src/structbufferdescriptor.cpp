/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local includes
#include "structbufferdescriptor.h"
#include "uniform.h"

RTTI_BEGIN_STRUCT(nap::StructBufferDescriptor)
	RTTI_PROPERTY("Element", &nap::StructBufferDescriptor::mElement, nap::rtti::EPropertyMetaData::Required | nap::rtti::EPropertyMetaData::Embedded)
	RTTI_PROPERTY("Count", &nap::StructBufferDescriptor::mCount, nap::rtti::EPropertyMetaData::Required)
RTTI_END_STRUCT

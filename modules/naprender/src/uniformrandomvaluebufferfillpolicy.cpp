/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "uniformrandomvaluebufferfillpolicy.h"
#include "uniform.h"

// External Includes
#include <nap/core.h>

RTTI_BEGIN_CLASS(nap::UniformRandomIntBufferFillPolicy)
	RTTI_PROPERTY("LowerBound", &nap::UniformRandomIntBufferFillPolicy::mLowerBound, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("UpperBound", &nap::UniformRandomIntBufferFillPolicy::mUpperBound, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniformRandomFloatBufferFillPolicy)
	RTTI_PROPERTY("LowerBound", &nap::UniformRandomFloatBufferFillPolicy::mLowerBound, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("UpperBound", &nap::UniformRandomFloatBufferFillPolicy::mUpperBound, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniformRandomVec2BufferFillPolicy)
	RTTI_PROPERTY("LowerBound", &nap::UniformRandomVec2BufferFillPolicy::mLowerBound, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("UpperBound", &nap::UniformRandomVec2BufferFillPolicy::mUpperBound, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniformRandomVec3BufferFillPolicy)
	RTTI_PROPERTY("LowerBound", &nap::UniformRandomVec3BufferFillPolicy::mLowerBound, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("UpperBound", &nap::UniformRandomVec3BufferFillPolicy::mUpperBound, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniformRandomVec4BufferFillPolicy)
	RTTI_PROPERTY("LowerBound", &nap::UniformRandomVec4BufferFillPolicy::mLowerBound, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("UpperBound", &nap::UniformRandomVec4BufferFillPolicy::mUpperBound, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniformRandomMat4BufferFillPolicy)
	RTTI_PROPERTY("LowerBound", &nap::UniformRandomMat4BufferFillPolicy::mLowerBound, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("UpperBound", &nap::UniformRandomMat4BufferFillPolicy::mUpperBound, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

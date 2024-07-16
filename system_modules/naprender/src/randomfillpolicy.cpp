/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "randomfillpolicy.h"
#include "uniform.h"

// External Includes
#include <nap/core.h>

RTTI_BEGIN_CLASS(nap::RandomFillPolicyUInt)
	RTTI_PROPERTY("LowerBound", &nap::RandomFillPolicyUInt::mLowerBound, nap::rtti::EPropertyMetaData::Default, "Lower unsigned interger value")
	RTTI_PROPERTY("UpperBound", &nap::RandomFillPolicyUInt::mUpperBound, nap::rtti::EPropertyMetaData::Default, "Upper unsigned interger value")
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::RandomFillPolicyInt)
	RTTI_PROPERTY("LowerBound", &nap::RandomFillPolicyInt::mLowerBound, nap::rtti::EPropertyMetaData::Default, "Lower integer value")
	RTTI_PROPERTY("UpperBound", &nap::RandomFillPolicyInt::mUpperBound, nap::rtti::EPropertyMetaData::Default, "Upper integer value")
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::RandomFillPolicyFloat)
	RTTI_PROPERTY("LowerBound", &nap::RandomFillPolicyFloat::mLowerBound, nap::rtti::EPropertyMetaData::Default, "Lower float value")
	RTTI_PROPERTY("UpperBound", &nap::RandomFillPolicyFloat::mUpperBound, nap::rtti::EPropertyMetaData::Default, "Upper float value")
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::RandomFillPolicyVec2)
	RTTI_PROPERTY("LowerBound", &nap::RandomFillPolicyVec2::mLowerBound, nap::rtti::EPropertyMetaData::Default, "Lower vector (vec2) value")
	RTTI_PROPERTY("UpperBound", &nap::RandomFillPolicyVec2::mUpperBound, nap::rtti::EPropertyMetaData::Default, "Upper vector (vec2) value")
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::RandomFillPolicyVec3)
	RTTI_PROPERTY("LowerBound", &nap::RandomFillPolicyVec3::mLowerBound, nap::rtti::EPropertyMetaData::Default, "Lower vector (vec3) value")
	RTTI_PROPERTY("UpperBound", &nap::RandomFillPolicyVec3::mUpperBound, nap::rtti::EPropertyMetaData::Default, "Upper vector (vec3) value")
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::RandomFillPolicyVec4)
	RTTI_PROPERTY("LowerBound", &nap::RandomFillPolicyVec4::mLowerBound, nap::rtti::EPropertyMetaData::Default,	"Lower vector (vec4) value")
	RTTI_PROPERTY("UpperBound", &nap::RandomFillPolicyVec4::mUpperBound, nap::rtti::EPropertyMetaData::Default, "Upper vector (vec4) value")
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::RandomFillPolicyMat4)
	RTTI_PROPERTY("LowerBound", &nap::RandomFillPolicyMat4::mLowerBound, nap::rtti::EPropertyMetaData::Default, "Lower matrix (4x4) value")
	RTTI_PROPERTY("UpperBound", &nap::RandomFillPolicyMat4::mUpperBound, nap::rtti::EPropertyMetaData::Default, "Upper matrix (4x4) value")
RTTI_END_CLASS

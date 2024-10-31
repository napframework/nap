/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "fillpolicy.h"
#include "uniform.h"

// External Includes
#include <nap/core.h>

RTTI_DEFINE_BASE(nap::BaseFillPolicy, "Rule that defines how to fill a large block of memory")

RTTI_DEFINE_BASE(nap::FillPolicyUInt)
RTTI_DEFINE_BASE(nap::FillPolicyInt)
RTTI_DEFINE_BASE(nap::FillPolicyFloat)
RTTI_DEFINE_BASE(nap::FillPolicyVec2)
RTTI_DEFINE_BASE(nap::FillPolicyVec3)
RTTI_DEFINE_BASE(nap::FillPolicyVec4)
RTTI_DEFINE_BASE(nap::FillPolicyMat4)

RTTI_BEGIN_CLASS(nap::ConstantFillPolicyUInt)
	RTTI_PROPERTY("Constant", &nap::ConstantFillPolicyUInt::mConstant, nap::rtti::EPropertyMetaData::Default, "Unsigned integer fill value")
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::ConstantFillPolicyInt)
	RTTI_PROPERTY("Constant", &nap::ConstantFillPolicyInt::mConstant, nap::rtti::EPropertyMetaData::Default, "Integer fill value")
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::ConstantFillPolicyFloat)
	RTTI_PROPERTY("Constant", &nap::ConstantFillPolicyFloat::mConstant, nap::rtti::EPropertyMetaData::Default, "Float fill value")
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::ConstantFillPolicyVec2)
	RTTI_PROPERTY("Constant", &nap::ConstantFillPolicyVec2::mConstant, nap::rtti::EPropertyMetaData::Default, "Vec2 fill value")
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::ConstantFillPolicyVec3)
	RTTI_PROPERTY("Constant", &nap::ConstantFillPolicyVec3::mConstant, nap::rtti::EPropertyMetaData::Default, "Vec3 fill value")
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::ConstantFillPolicyVec4)
	RTTI_PROPERTY("Constant", &nap::ConstantFillPolicyVec4::mConstant, nap::rtti::EPropertyMetaData::Default, "Vec4 fill value")
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::ConstantFillPolicyMat4)
	RTTI_PROPERTY("Constant", &nap::ConstantFillPolicyMat4::mConstant, nap::rtti::EPropertyMetaData::Default, "Mat4 fill value")
RTTI_END_CLASS

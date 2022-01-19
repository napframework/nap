/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "valuebufferfillpolicy.h"
#include "uniform.h"

// External Includes
#include <nap/core.h>

RTTI_DEFINE_BASE(nap::BaseValueBufferFillPolicy)

RTTI_DEFINE_BASE(nap::IntBufferFillPolicy)
RTTI_DEFINE_BASE(nap::FloatBufferFillPolicy)
RTTI_DEFINE_BASE(nap::Vec2BufferFillPolicy)
RTTI_DEFINE_BASE(nap::Vec3BufferFillPolicy)
RTTI_DEFINE_BASE(nap::Vec4BufferFillPolicy)
RTTI_DEFINE_BASE(nap::Mat4BufferFillPolicy)

RTTI_BEGIN_CLASS(nap::ConstantUIntBufferFillPolicy)
	RTTI_PROPERTY("Constant", &nap::ConstantUIntBufferFillPolicy::mConstant, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::ConstantIntBufferFillPolicy)
	RTTI_PROPERTY("Constant", &nap::ConstantIntBufferFillPolicy::mConstant, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::ConstantFloatBufferFillPolicy)
	RTTI_PROPERTY("Constant", &nap::ConstantFloatBufferFillPolicy::mConstant, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::ConstantVec2BufferFillPolicy)
	RTTI_PROPERTY("Constant", &nap::ConstantVec2BufferFillPolicy::mConstant, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::ConstantVec3BufferFillPolicy)
	RTTI_PROPERTY("Constant", &nap::ConstantVec3BufferFillPolicy::mConstant, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::ConstantVec4BufferFillPolicy)
	RTTI_PROPERTY("Constant", &nap::ConstantVec4BufferFillPolicy::mConstant, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::ConstantMat4BufferFillPolicy)
	RTTI_PROPERTY("Constant", &nap::ConstantMat4BufferFillPolicy::mConstant, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

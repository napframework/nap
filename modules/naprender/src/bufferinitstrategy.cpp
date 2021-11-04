/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "bufferinitstrategy.h"

// External Includes
#include <nap/core.h>

RTTI_DEFINE_BASE(nap::BaseValueBufferInitStrategy)

RTTI_DEFINE_BASE(nap::IntBufferInitStrategy)
RTTI_DEFINE_BASE(nap::FloatBufferInitStrategy)
RTTI_DEFINE_BASE(nap::Vec2BufferInitStrategy)
RTTI_DEFINE_BASE(nap::Vec3BufferInitStrategy)
RTTI_DEFINE_BASE(nap::Vec4BufferInitStrategy)
RTTI_DEFINE_BASE(nap::Mat4BufferInitStrategy)


//////////////////////////////////////////////////////////////////////////
// UniformRandomBufferInitStrategy
//////////////////////////////////////////////////////////////////////////

RTTI_BEGIN_CLASS(nap::UniformRandomIntBufferInitStrategy)
	RTTI_PROPERTY("Lowerbound", &nap::UniformRandomIntBufferInitStrategy::mLowerBound, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("UpperBound", &nap::UniformRandomIntBufferInitStrategy::mUpperBound, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniformRandomFloatBufferInitStrategy)
	RTTI_PROPERTY("Lowerbound", &nap::UniformRandomFloatBufferInitStrategy::mLowerBound, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("UpperBound", &nap::UniformRandomFloatBufferInitStrategy::mUpperBound, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniformRandomIntBufferInitStrategy)
	RTTI_PROPERTY("Lowerbound", &nap::UniformRandomIntBufferInitStrategy::mLowerBound, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("UpperBound", &nap::UniformRandomIntBufferInitStrategy::mUpperBound, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniformRandomVec2BufferInitStrategy)
	RTTI_PROPERTY("Lowerbound", &nap::UniformRandomVec2BufferInitStrategy::mLowerBound, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("UpperBound", &nap::UniformRandomVec2BufferInitStrategy::mUpperBound, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniformRandomVec3BufferInitStrategy)
	RTTI_PROPERTY("Lowerbound", &nap::UniformRandomVec3BufferInitStrategy::mLowerBound, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("UpperBound", &nap::UniformRandomVec3BufferInitStrategy::mUpperBound, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniformRandomVec4BufferInitStrategy)
	RTTI_PROPERTY("Lowerbound", &nap::UniformRandomVec4BufferInitStrategy::mLowerBound, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("UpperBound", &nap::UniformRandomVec4BufferInitStrategy::mUpperBound, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniformRandomMat4BufferInitStrategy)
	RTTI_PROPERTY("Lowerbound", &nap::UniformRandomMat4BufferInitStrategy::mLowerBound, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("UpperBound", &nap::UniformRandomMat4BufferInitStrategy::mUpperBound, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS


//////////////////////////////////////////////////////////////////////////
// ConstantBufferInitStrategy
//////////////////////////////////////////////////////////////////////////

RTTI_BEGIN_CLASS(nap::ConstantIntBufferInitStrategy)
	RTTI_PROPERTY("Constant", &nap::ConstantIntBufferInitStrategy::mConstant, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::ConstantFloatBufferInitStrategy)
	RTTI_PROPERTY("Constant", &nap::ConstantFloatBufferInitStrategy::mConstant, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::ConstantIntBufferInitStrategy)
	RTTI_PROPERTY("Constant", &nap::ConstantIntBufferInitStrategy::mConstant, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::ConstantVec2BufferInitStrategy)
	RTTI_PROPERTY("Constant", &nap::ConstantVec2BufferInitStrategy::mConstant, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::ConstantVec3BufferInitStrategy)
	RTTI_PROPERTY("Constant", &nap::ConstantVec3BufferInitStrategy::mConstant, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::ConstantVec4BufferInitStrategy)
	RTTI_PROPERTY("Constant", &nap::ConstantVec4BufferInitStrategy::mConstant, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::ConstantMat4BufferInitStrategy)
	RTTI_PROPERTY("Constant", &nap::ConstantMat4BufferInitStrategy::mConstant, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

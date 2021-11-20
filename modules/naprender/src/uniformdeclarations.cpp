/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "uniformdeclarations.h"
#include "uniform.h"

RTTI_BEGIN_ENUM(nap::EBufferObjectType)
	RTTI_ENUM_VALUE(nap::EBufferObjectType::Uniform, "Uniform"),
	RTTI_ENUM_VALUE(nap::EBufferObjectType::Storage, "Storage")
RTTI_END_ENUM

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::UniformDeclaration)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::UniformValueDeclaration)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::UniformStructDeclaration)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::UniformStructArrayDeclaration)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::UniformValueArrayDeclaration)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::UniformBufferObjectDeclaration)
RTTI_END_CLASS


namespace nap
{
	UniformDeclaration::UniformDeclaration(const std::string& name, int offset, int size) :
		mName(name),
		mOffset(offset),
		mSize(size)
	{
	}

	//////////////////////////////////////////////////////////////////////////

	UniformValueDeclaration::UniformValueDeclaration(const std::string& name, int offset, int size, EUniformValueType type) :
		UniformDeclaration(name, offset, size),
		mType(type)
	{
	}

	//////////////////////////////////////////////////////////////////////////


	UniformStructDeclaration::UniformStructDeclaration(const std::string& name, EBufferObjectType type, int offset, int size) :
		UniformDeclaration(name, offset, size),
		mBufferObjectType(type)
	{
	}


	UniformStructDeclaration::~UniformStructDeclaration()
	{
	}


	const UniformDeclaration* UniformStructDeclaration::findMember(const std::string& name) const
	{
		for (auto& member : mMembers)
			if (member->mName == name)
				return member.get();

		return nullptr;
	}


	UniformStructDeclaration::UniformStructDeclaration(UniformStructDeclaration&& inRHS) :
		UniformDeclaration(std::move(inRHS)),
		mMembers(std::move(inRHS.mMembers)),
		mBufferObjectType(std::move(inRHS.mBufferObjectType))
	{
	}


	UniformStructDeclaration& UniformStructDeclaration::operator=(UniformStructDeclaration&& inRHS)
	{
		mMembers = std::move(inRHS.mMembers);
		mBufferObjectType = std::move(inRHS.mBufferObjectType);
		UniformDeclaration::operator=(inRHS);
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////

	UniformStructArrayDeclaration::UniformStructArrayDeclaration(const std::string& name, int offset, int size) :
		UniformDeclaration(name, offset, size)
	{
	}

	//////////////////////////////////////////////////////////////////////////

	UniformStructBufferDeclaration::UniformStructBufferDeclaration(const std::string& name, int offset, int size, int stride, int numElements) :
		UniformDeclaration(name, offset, size),
		mNumElements(numElements),
		mStride(stride)
	{
	}

	//////////////////////////////////////////////////////////////////////////

	UniformValueArrayDeclaration::UniformValueArrayDeclaration(const std::string& name, int offset, int size, int stride, EUniformValueType elementType, int numElements) :
		UniformDeclaration(name, offset, size),
		mElementType(elementType),
		mNumElements(numElements),
		mStride(stride)
	{
	}

	//////////////////////////////////////////////////////////////////////////

	UniformBufferObjectDeclaration::UniformBufferObjectDeclaration(const std::string& name, int binding, VkShaderStageFlagBits inStage, EBufferObjectType type, int size) :
		UniformStructDeclaration(name, type, 0, size),
		mBinding(binding),
		mStage(inStage)
	{
	}


	UniformBufferObjectDeclaration::UniformBufferObjectDeclaration(UniformBufferObjectDeclaration&& inRHS) :
		UniformStructDeclaration(std::move(inRHS)),
		mBinding(inRHS.mBinding),
		mStage(inRHS.mStage)
	{
	}


	UniformBufferObjectDeclaration& UniformBufferObjectDeclaration::operator=(UniformBufferObjectDeclaration&& inRHS)
	{
		mBinding = inRHS.mBinding;
		mStage = inRHS.mStage;
		UniformStructDeclaration::operator=(std::move(inRHS));

		return *this;
	}
}

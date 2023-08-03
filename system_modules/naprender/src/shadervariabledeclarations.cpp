/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "shadervariabledeclarations.h"
#include "uniform.h"

// External includes
#include <assert.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ShaderVariableDeclaration)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ShaderVariableValueDeclaration)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ShaderVariableStructDeclaration)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ShaderVariableStructArrayDeclaration)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ShaderVariableValueArrayDeclaration)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::BufferObjectDeclaration)
RTTI_END_CLASS


namespace nap
{
	ShaderVariableDeclaration::ShaderVariableDeclaration(const std::string& name, int offset, int size) :
		mName(name),
		mOffset(offset),
		mSize(size)
	{
	}

	//////////////////////////////////////////////////////////////////////////

	ShaderVariableValueDeclaration::ShaderVariableValueDeclaration(const std::string& name, int offset, int size, EShaderVariableValueType type) :
		ShaderVariableDeclaration(name, offset, size),
		mType(type)
	{
	}

	//////////////////////////////////////////////////////////////////////////


	ShaderVariableStructDeclaration::ShaderVariableStructDeclaration(const std::string& name, EDescriptorType descriptorType, int offset, int size) :
		ShaderVariableDeclaration(name, offset, size),
		mDescriptorType(descriptorType)
	{
	}


	ShaderVariableStructDeclaration::~ShaderVariableStructDeclaration()
	{
	}


	const ShaderVariableDeclaration* ShaderVariableStructDeclaration::findMember(const std::string& name) const
	{
		for (auto& member : mMembers)
			if (member->mName == name)
				return member.get();

		return nullptr;
	}


	ShaderVariableStructDeclaration::ShaderVariableStructDeclaration(ShaderVariableStructDeclaration&& inRHS) :
		ShaderVariableDeclaration(std::move(inRHS)),
		mMembers(std::move(inRHS.mMembers)),
		mDescriptorType(std::move(inRHS.mDescriptorType))
	{ }


	ShaderVariableStructDeclaration& ShaderVariableStructDeclaration::operator=(ShaderVariableStructDeclaration&& inRHS)
	{
		mMembers = std::move(inRHS.mMembers);
		mDescriptorType = std::move(inRHS.mDescriptorType);
		ShaderVariableDeclaration::operator=(std::move(inRHS));
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////

	ShaderVariableStructArrayDeclaration::ShaderVariableStructArrayDeclaration(const std::string& name, int offset, int size) :
		ShaderVariableDeclaration(name, offset, size)
	{
	}

	//////////////////////////////////////////////////////////////////////////

	ShaderVariableStructBufferDeclaration::ShaderVariableStructBufferDeclaration(const std::string& name, int offset, int size, int stride, int numElements) :
		ShaderVariableDeclaration(name, offset, size),
		mNumElements(numElements),
		mStride(stride)
	{
	}

	//////////////////////////////////////////////////////////////////////////

	ShaderVariableValueArrayDeclaration::ShaderVariableValueArrayDeclaration(const std::string& name, int offset, int size, int stride, EShaderVariableValueType elementType, int numElements) :
		ShaderVariableDeclaration(name, offset, size),
		mElementType(elementType),
		mNumElements(numElements),
		mStride(stride)
	{
	}

	//////////////////////////////////////////////////////////////////////////

	BufferObjectDeclaration::BufferObjectDeclaration(const std::string& name, int binding, VkShaderStageFlags inStages, nap::EDescriptorType descriptorType, int size) :
		ShaderVariableStructDeclaration(name, descriptorType, 0, size),
		mBinding(binding),
		mStages(inStages)
	{
	}


	BufferObjectDeclaration::BufferObjectDeclaration(BufferObjectDeclaration&& inRHS) :
		ShaderVariableStructDeclaration(std::move(inRHS)),
		mBinding(inRHS.mBinding),
		mStages(inRHS.mStages)
	{
	}


	BufferObjectDeclaration& BufferObjectDeclaration::operator=(BufferObjectDeclaration&& inRHS)
	{
		mBinding = inRHS.mBinding;
		mStages = inRHS.mStages;
		ShaderVariableStructDeclaration::operator=(std::move(inRHS));

		return *this;
	}


	const ShaderVariableDeclaration& BufferObjectDeclaration::getBufferDeclaration() const
	{
		assert(!mMembers.empty()); return *mMembers[0];
	}
}

/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "uniformcontainer.h"
#include "texture.h"

// External Includes
#include <rtti/rttiutilities.h>

RTTI_BEGIN_CLASS(nap::UniformContainer)
	RTTI_FUNCTION("findUniform", (nap::UniformStructInstance* (nap::UniformContainer::*)(const std::string&)) &nap::UniformContainer::findUniform)
	RTTI_FUNCTION("findBinding", (nap::BufferBindingInstance* (nap::UniformContainer::*)(const std::string&)) &nap::UniformContainer::findBinding)
	RTTI_FUNCTION("findSampler", (nap::SamplerInstance* (nap::UniformContainer::*)(const std::string&)) &nap::UniformContainer::findSampler)
RTTI_END_CLASS


namespace nap
{
	UniformStructInstance* UniformContainer::findUniform(const std::string& name) const
	{
		for (auto& instance : mUniformRootStructs)
			if (instance->getDeclaration().mName == name)
				return instance.get();
		return nullptr;
	}


	SamplerInstance* UniformContainer::findSampler(const std::string& name) const
	{
		for (auto& sampler : mSamplerInstances)
			if (sampler->getDeclaration().mName == name)
				return sampler.get();
		return nullptr;
	}


	BufferBindingInstance* UniformContainer::findBinding(const std::string& name) const
	{
		for (auto& instance : mBindingInstances)
			if (instance->getBindingName() == name)
				return instance.get();
		return nullptr;
	}


	ShaderConstantInstance* UniformContainer::findConstant(const std::string& name) const
	{
		for (auto& constant : mConstantInstances)
			if (constant->getDeclaration().mName == name)
				return constant.get();
		return nullptr;
	}


	BufferBindingInstance& UniformContainer::addBindingInstance(std::unique_ptr<BufferBindingInstance> instance)
	{
		return *mBindingInstances.emplace_back(std::move(instance));
	}


	SamplerInstance& UniformContainer::addSamplerInstance(std::unique_ptr<SamplerInstance> instance)
	{
		return *mSamplerInstances.emplace_back(std::move(instance));
	}


	ShaderConstantInstance& UniformContainer::addConstantInstance(std::unique_ptr<ShaderConstantInstance> instance)
	{
		return *mConstantInstances.emplace_back(std::move(instance));
	}


	UniformStructInstance& UniformContainer::createUniformRootStruct(const ShaderVariableStructDeclaration& declaration, const UniformCreatedCallback& uniformCreatedCallback)
	{
		std::unique_ptr<UniformStructInstance> instance = std::make_unique<UniformStructInstance>(declaration, uniformCreatedCallback);
		UniformStructInstance* result = instance.get();
		mUniformRootStructs.emplace_back(std::move(instance));
		return *result;
	}
}

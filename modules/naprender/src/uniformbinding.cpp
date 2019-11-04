#include "uniformbinding.h"
#include <rtti/rttiutilities.h>

RTTI_BEGIN_CLASS(nap::UniformContainer)
	//RTTI_FUNCTION("findUniform", (nap::Uniform* (nap::UniformContainer::*)(const std::string&)) &nap::UniformContainer::findUniform)
RTTI_END_CLASS


namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// UniformContainer
	//////////////////////////////////////////////////////////////////////////

	Uniform* UniformContainer::findUniform(const std::string& name)
	{
		auto texture_binding = mUniformSamplers.find(name);
		if (texture_binding != mUniformSamplers.end())
			return texture_binding->second.get();

		auto value_binding = mUniformValues.find(name);
		if (value_binding != mUniformValues.end())
			return value_binding->second.get();

		return nullptr;
	}

	Uniform& UniformContainer::addUniformValue(std::unique_ptr<UniformValue> uniform)
	{
		auto inserted = mUniformValues.emplace(std::make_pair(uniform->getDeclaration().mName, std::move(uniform)));
		return *inserted.first->second;
	}

	Uniform& UniformContainer::addUniformSampler(std::unique_ptr<UniformTexture> uniform)
	{
		auto inserted = mUniformSamplers.emplace(std::make_pair(uniform->getDeclaration().mName, std::move(uniform)));
		return *inserted.first->second;
	}
}
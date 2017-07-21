#include "uniformbinding.h"
#include <rtti/rttiutilities.h>

namespace nap
{

	UniformBinding::~UniformBinding()
	{
		mUniform = nullptr;
	}


	UniformBinding::UniformBinding(std::unique_ptr<Uniform>&& uniform, const opengl::UniformDeclaration& declaration) :
		mUniform(std::move(uniform)),
		mDeclaration(&declaration)
	{}


	UniformBinding::UniformBinding(UniformBinding&& other) :
		mUniform(std::move(other.mUniform)),
		mDeclaration(other.mDeclaration)
	{}


	UniformBinding::UniformBinding(const UniformBinding& other) : mDeclaration(other.mDeclaration)
	{
		rtti::TypeInfo other_type = other.mUniform->get_type();
		assert(other_type.is_derived_from(RTTI_OF(nap::Uniform)));
		
		nap::Uniform* target = other_type.create<nap::Uniform>();
		nap::rtti::copyObject(*other.mUniform, *target);
		mUniform.reset(target);
	}


	UniformBinding& UniformBinding::operator=(UniformBinding&& other)
	{
		mUniform = std::move(other.mUniform);
		mDeclaration = other.mDeclaration;
		return *this;
	}


	UniformBinding& UniformBinding::operator=(const UniformBinding& other)
	{
		rtti::TypeInfo other_type = other.mUniform->get_type();
		assert(other_type.is_derived_from(RTTI_OF(nap::Uniform)));
		
		nap::Uniform* target = other_type.create<nap::Uniform>();
		nap::rtti::copyObject(*other.mUniform, *target);
		mUniform.reset(target);
		return *this;
	}


	//////////////////////////////////////////////////////////////////////////
	// UniformContainer
	//////////////////////////////////////////////////////////////////////////


	Uniform& UniformContainer::AddUniform(std::unique_ptr<Uniform> uniform, const opengl::UniformDeclaration& declaration)
	{
		// Create association between uniform and declaration. At the same time, split between textures and values
		// as texture have a slightly different interface.
		std::unique_ptr<UniformTexture> texture_uniform = rtti_cast<UniformTexture>(uniform);
		if (texture_uniform == nullptr)
		{
			std::unique_ptr<UniformValue> value_uniform = rtti_cast<UniformValue>(uniform);
			assert(value_uniform);
			auto inserted = mUniformValueBindings.emplace(std::make_pair(declaration.mName, UniformBinding(std::move(value_uniform), declaration)));
			return *inserted.first->second.mUniform;
		}
		else
		{
			auto inserted = mUniformTextureBindings.emplace(std::make_pair(declaration.mName, UniformBinding(std::move(texture_uniform), declaration)));
			return *inserted.first->second.mUniform;
		}
	}
}
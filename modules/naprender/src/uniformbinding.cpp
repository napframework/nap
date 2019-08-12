#include "uniformbinding.h"
#include <rtti/rttiutilities.h>

RTTI_BEGIN_CLASS(nap::UniformContainer)
	RTTI_FUNCTION("findUniform", (nap::Uniform* (nap::UniformContainer::*)(const std::string&)) &nap::UniformContainer::findUniform)
RTTI_END_CLASS


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

	Uniform* UniformContainer::findUniform(const std::string& name)
	{
		auto texture_binding = mUniformTextureBindings.find(name);
		if (texture_binding != mUniformTextureBindings.end())
			return texture_binding->second.mUniform.get();

		auto value_binding = mUniformValueBindings.find(name);
		if (value_binding != mUniformValueBindings.end())
			return value_binding->second.mUniform.get();

		return nullptr;
	}


	const nap::UniformBinding* UniformContainer::findUniformBinding(const std::string& name) const
	{
		auto value_it = mUniformValueBindings.find(name);
		if (value_it != mUniformValueBindings.end())
			return &(value_it->second);

		auto text_it = mUniformTextureBindings.find(name);
		if (text_it != mUniformTextureBindings.end())
			return &(text_it->second);

		return nullptr;
	}


	const nap::UniformBinding& UniformContainer::getUniformBinding(const std::string& name) const
	{
		auto binding = findUniformBinding(name);
		assert(binding != nullptr);
		return *binding;
	}


	Uniform& UniformContainer::addUniform(std::unique_ptr<Uniform> uniform, const opengl::UniformDeclaration& declaration)
	{
		// The name in the declaration is the name as returned by OpenGL. This means it's the full 'path' to the uniform.
		// The path also includes the array specifier. For example, if you have the following in your shader:
		//
		// uniform float inputs[2]
		// 
		// The name of this uniform will be returned as 'inputs[0]'. However, this is not what we want; we want the user to be able to address
		// this uniform simply by using its name, in this case 'inputs' without the array specifier.
		//
		// We can't just simply strip off all array specifiers, as there might be array elements earlier on the path; we can only strip off the trailing array specifier.
		// For example, if you have the following in your shader:
		//
		// struct SomeStruct
		// {
		//     float mValues[2];
		// }
		//
		// uniform SomeStruct structs[2];
		//
		// And, assuming you use these uniforms, the uniforms returned will be:
		//
		// structs[0].mValues[0]
		// structs[1].mValues[0]
		//
		// In this case, we need to ensure we only strip off the trailing array specifier; the array specifier after the 'structs' name is important information about which 
		// array element is being indexed.
		std::string name = declaration.mName;

		// Determine where we should start searching for the array specifier: if this is a path with multiple elements, we start at the start of the last element.
		// If it's only a single element path, we search from the start.
		size_t bracket_start_search_pos = name.find_last_of('.');
		if (bracket_start_search_pos == std::string::npos)
			bracket_start_search_pos = 0;

		// Find the array specifier starting at the position we found above; if found, strip it off.
		size_t bracket_pos = name.find_first_of('[', bracket_start_search_pos);
		if (bracket_pos != std::string::npos)
			name = name.substr(0, bracket_pos);

		// Create association between uniform and declaration. At the same time, split between textures and values
		// as texture have a slightly different interface.
		std::unique_ptr<UniformTexture> texture_uniform = rtti_cast<UniformTexture>(uniform);
		if (texture_uniform == nullptr)
		{
			std::unique_ptr<UniformValue> value_uniform = rtti_cast<UniformValue>(uniform);
			assert(value_uniform);
			auto inserted = mUniformValueBindings.emplace(std::make_pair(name, UniformBinding(std::move(value_uniform), declaration)));
			return *inserted.first->second.mUniform;
		}
		else
		{
			auto inserted = mUniformTextureBindings.emplace(std::make_pair(name, UniformBinding(std::move(texture_uniform), declaration)));
			return *inserted.first->second.mUniform;
		}
	}
}
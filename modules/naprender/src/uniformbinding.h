#pragma once

// External Includes
#include <nshaderutils.h>

// Local Includes
#include "uniforms.h"
#include "texture2dfromfile.h"

namespace nap
{
	/**
	* Binds the Uniform data to the declaration from the shader. Together
	* they can be used to push the uniform.
	*/
	struct NAPAPI UniformBinding
	{
		~UniformBinding();

		UniformBinding(std::unique_ptr<Uniform>&& uniform, const opengl::UniformDeclaration& declaration);

		UniformBinding(UniformBinding&& other);

		UniformBinding& operator=(UniformBinding&& other);

		UniformBinding(const UniformBinding& other);

		UniformBinding& operator=(const UniformBinding& other);

		std::unique_ptr<Uniform>			mUniform;		//< Unique ptr to the actual uniform value
		const opengl::UniformDeclaration*	mDeclaration;	//< OpenGL Uniform declaration
	};

	using UniformTextureBindings = std::unordered_map<std::string, UniformBinding>;
	using UniformValueBindings = std::unordered_map<std::string, UniformBinding>;

	//////////////////////////////////////////////////////////////////////////

	/**
	* Base class for both Material and MaterialInstance. Basic support for uniforms.
	*/
	class NAPAPI UniformContainer
	{
		RTTI_ENABLE()
	public:

		/**
		* @return a uniform texture object that can be used to set a texture or value.
		* If the uniform is not found, returns nullptr.
		*/
		Uniform* findUniform(const std::string& name);

		/**
		* @return a uniform texture object that can be used to set a texture or value.
		* If the uniform is not found, returns nullptr.
		*/
		template<typename T>
		T* findUniform(const std::string& name);

		/**
		* @return a uniform object that can be used to set a texture or value.
		* If the uniform is not found it will assert.
		*/
		template<typename T>
		T& getUniform(const std::string& name);

		/**
		* @return All texture uniform bindings.
		*/
		const UniformTextureBindings& getTextureBindings()	{ return mUniformTextureBindings; }

		/**
		* @return All value uniform bindings.
		*/
		const UniformValueBindings& getValueBindings()		{ return mUniformValueBindings; }

	protected:
		/**
		* Puts the uniform into either the texture or value mapping.
		* @param uniform: the uniform to add. Ownership is transferred.
		* @param declaration: the shader uniform declaration to bind the uniform to.
		* @return reference to the newly added uniform.
		*/
		Uniform& AddUniform(std::unique_ptr<Uniform> uniform, const opengl::UniformDeclaration& declaration);

	private:
		UniformTextureBindings		mUniformTextureBindings;	///< Runtime map of texture uniforms (superset of texture uniforms in mUniforms due to default uniforms).
		UniformValueBindings		mUniformValueBindings;		///< Runtime map of value uniforms (superset of value uniforms in mUniforms due to default uniforms).
	};

	//////////////////////////////////////////////////////////////////////////
	// Template Definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename T>
	T* UniformContainer::findUniform(const std::string& name)
	{
		UniformTextureBindings::iterator texture_binding = mUniformTextureBindings.find(name);
		if (texture_binding != mUniformTextureBindings.end() && texture_binding->second.mUniform->get_type() == RTTI_OF(T))
			return (T*)texture_binding->second.mUniform.get();

		UniformValueBindings::iterator value_binding = mUniformValueBindings.find(name);
		if (value_binding != mUniformValueBindings.end() && value_binding->second.mUniform->get_type() == RTTI_OF(T))
			return (T*)value_binding->second.mUniform.get();

		return nullptr;
	}


	template<typename T>
	T& UniformContainer::getUniform(const std::string& name)
	{
		T* result = findUniform<T>(name);
		assert(result);
		return *result;
	}

} // nap

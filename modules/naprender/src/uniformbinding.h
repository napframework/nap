#pragma once

// External Includes
#include <nshaderutils.h>

// Local Includes
#include "uniforms.h"
#include "imagefromfile.h"

namespace nap
{
	using UniformSamplers = std::unordered_map<std::string, std::unique_ptr<UniformTexture>>;
	using UniformValues = std::unordered_map<std::string, std::unique_ptr<UniformValue>>;

	//////////////////////////////////////////////////////////////////////////

	/**
	 * Manages uniform values and declarations. A uniform value is always tied to a declaration.
	 * Multiple uniform values can be associated with the same declaration.
	 * The uniform declaration is the actual interface to a uniform slot on a shader. 
	 * The uniform value is the actual value uploaded to that slot.
	 * Both the Material and MaterialInstance are a uniform container.
	 */
	class NAPAPI UniformContainer
	{
		RTTI_ENABLE()
	public:

        UniformContainer() = default;
		UniformContainer(const UniformContainer&) = delete;

        virtual ~UniformContainer() = default;
        
		UniformContainer& operator=(const UniformContainer&) = delete;

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
		const UniformSamplers& getUniformSamplers()	{ return mUniformSamplers; }

		/**
		* @return All value uniform bindings.
		*/
		const UniformValues& getUniformValues()		{ return mUniformValues; }

	protected:
		/**
		* Puts the uniform into either the texture or value mapping.
		* @param uniform: the uniform to add. Ownership is transferred.
		* @param declaration: the shader uniform declaration to bind the uniform to.
		* @return reference to the newly added uniform.
		*/
		Uniform& addUniformValue(std::unique_ptr<UniformValue> uniform);
		Uniform& addUniformSampler(std::unique_ptr<UniformTexture> uniform);

	private:
		UniformSamplers		mUniformSamplers;	///< Runtime map of sampler uniforms (superset of texture uniforms in mUniforms due to default uniforms).
		UniformValues		mUniformValues;		///< Runtime map of value uniforms (superset of value uniforms in mUniforms due to default uniforms).
	};

	//////////////////////////////////////////////////////////////////////////
	// Template Definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename T>
	T* UniformContainer::findUniform(const std::string& name)
	{
		return rtti_cast<T>(findUniform(name));
	}


	template<typename T>
	T& UniformContainer::getUniform(const std::string& name)
	{
		T* result = findUniform<T>(name);
		assert(result);
		return *result;
	}

} // nap

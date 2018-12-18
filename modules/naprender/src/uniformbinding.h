#pragma once

// External Includes
#include <nshaderutils.h>

// Local Includes
#include "uniforms.h"
#include "imagefromfile.h"

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
        virtual ~UniformContainer() = default;
        
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
		 * Search for a uniform shader binding with the given name. Use the binding to gain access to the uniform declaration.
		 * A uniform value binding represents the coupling between a uniform value and GLSL uniform declaration
		 * The declaration is the interface to the uniform on the shader, where the uniform value is the actual value that is uploaded.
		 * @param name the name of the uniform to find the binding for.
		 * @return a uniform value binding with the given name, nullptr if not found.
		 */
		const UniformBinding* findUniformBinding(const std::string& name) const;

		/**
		 * Gets a uniform shader binding with the given name. Use the binding to gain access to the uniform declaration.
		 * This call asserts if a binding with the given names does not exists.
		 * A uniform value binding represents the coupling between a uniform value and GLSL uniform declaration
		 * The declaration is the interface to the uniform on the shader, where the uniform value is the actual value that is uploaded.
		 * @param name the name of the uniform to find the binding for.
		 * @return a uniform value binding with the given name.
		 */
		const UniformBinding& getUniformBinding(const std::string& name) const;

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

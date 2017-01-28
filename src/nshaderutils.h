#pragma once

// External Includes
#include <string>
#include <GL/glew.h>
#include <vector>
#include <unordered_map>
#include <functional>

namespace opengl
{
	/**
	 * All available OpenGL shader uniform types
	 * All uniform types need to have a set factory
	 * function associated with it
	 */
	enum class UniformType : uint8_t
	{
		Float   = 0,
		Int     = 1,
		UInt    = 2,
		Vec2F   = 3,
		Vec3F   = 4,
		Vec4F   = 5,
		Mat2    = 6,
		Mat3    = 7,
		Mat4    = 8,
	};

	/**
	 * Uniform set function, where void* is the data, GLint the location and
	 * GLSizei the number of elements
	 */
	using UniformSetterFunction = std::function<void(const void*, const GLint&, const GLsizei&)>;

	/**
	 * @return the uniform set function for the associated type
	 * returns nullptr if there's no setter found
	 */
	UniformSetterFunction* getUniformSetter(UniformType type);

	/**
	* Represents an opengl shader attribute
	*/
	struct ShaderInput
	{
		// Constructor
		ShaderInput(GLuint shaderProgram, std::string& name, GLenum type, GLint location);
		ShaderInput() = delete;

		std::string		mName;				// Name of the shader attribute
		GLenum			mType;				// Type of the shader attribute
		GLint			mLocation;			// Location of the shader attribute
		GLuint			mShaderProgram;		// Shader this uniform is associated with
	};

	// Typedefs
	using ShaderUniform		= ShaderInput;
	using ShaderAttribute	= ShaderInput;
	using ShaderUniforms	= std::unordered_map<std::string, ShaderUniform>;
	using ShaderAttributes	= std::unordered_map<std::string, ShaderAttribute>;

	/**
	 * Given part of a shader (say vertex shader), validateShader will get information from OpenGl 
	 * on whether or not the shader was compiled successfully
	 * Note that the shader part must be loaded
	 * @return if the shader part is loaded correctly
	 */
	bool validateShader(GLuint shader);


	/**
	 * Given a shader program, validateProgram will request from OpenGL, any information
	 * related to the validation or linking of the program with it's attached shaders. It will
	 * then output any issues that have occurred.
	 * Note that the shader must be loaded
	 * @return if the shader program is loaded correctly without errors
	 */
	bool validateShaderProgram(GLuint program);


	/**
	 * Extracts all shader uniforms, note that the program must be loaded
	 * @param program: The shader program to extract uniform inputs from
	 * @param uniforms: The populated list of uniforms
	 */
	void extractShaderUniforms(GLuint program, ShaderUniforms& outUniforms);


	/**
	 * Extracts all shader attributes, note that the program must be loaded
	 * @param program: The shader program to extract attribute inputs from
	 * @param attributes: The populated list of shader attributes
	 */
	void extractShaderAttributes(GLuint program, ShaderAttributes& outAttributes);
}

//////////////////////////////////////////////////////////////////////////
// Hash
//////////////////////////////////////////////////////////////////////////
namespace std
{
	template<>
	struct hash<opengl::ShaderInput> 
	{
		size_t operator()(const opengl::ShaderInput &k) const 
		{
			return hash<std::string>()(k.mName);
		}
	};
}

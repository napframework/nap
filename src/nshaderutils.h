#pragma once

// External Includes
#include <string>
#include <GL/glew.h>
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>

namespace opengl
{
	/**
	 * All available OpenGL shader uniform types
	 * All uniform types need to have a set factory
	 * function associated with it
	 */
	enum class GLSLType : uint8_t
	{
		Unknown = 0,
		Float   = 1,
		Int     = 2,
		UInt    = 3,
		Vec2	= 4,
		Vec3	= 5,
		Vec4	= 6,
		Mat2    = 7,
		Mat3    = 8,
		Mat4    = 9,
		Tex2D	= 10,
	};

	/**
	 * Result of OpenGL shader validation
	 */
	enum class EShaderValidationResult : uint8_t
	{
		SUCCESS,
		WARNING,
		ERROR
	};

	/**
	 * @return the uniform type based on the opengl uniform type
	 * @param glType: opengl internal type that describes a uniform
	 */
	GLSLType getGLSLType(GLenum glType);

	/**
	* Represents an opengl shader attribute
	*/
	class ShaderInput
	{
	public:
		// Constructor
		ShaderInput(GLuint shaderProgram, const std::string& name, GLenum type, GLint location, GLint size);
		ShaderInput() = delete;

		std::string		mName;							// Name of the shader attribute
		GLenum			mType =	GL_INVALID_ENUM;		// OpenGL Type of the shader attribute
		GLSLType		mGLSLType = GLSLType::Unknown;	// System GLSL type
		GLint			mLocation = -1;					// Location of the shader attribute
		GLuint			mShaderProgram = 0;				// Shader this uniform is associated with
		GLint			mSize = 0;						// Number of elements in array

		/**
		 * @return if this shader input is an array or single value
		 */
		bool isArray() const							{ return mSize > 1; }
	};


	// Typedefs
	using UniformDeclaration = ShaderInput;
	using UniformDeclarations = std::unordered_map<std::string, std::unique_ptr<UniformDeclaration>>;

	using VertexAttribute = ShaderInput;
	using VertexAttributes = std::unordered_map<std::string, std::unique_ptr<VertexAttribute>>;

	/**
	 * Given part of a shader (say vertex shader), validateShader will get information from OpenGl 
	 * on whether or not the shader was compiled successfully
	 * Note that the shader part must be loaded
	 * @return if the shader part is loaded correctly
	 */
	EShaderValidationResult validateShader(GLuint shader, std::string& validationMessage);


	/**
	 * Given a shader program, validateProgram will request from OpenGL, any information
	 * related to the validation or linking of the program with it's attached shaders. It will
	 * then output any issues that have occurred.
	 * Note that the shader must be loaded
	 * @return if the shader program is loaded correctly without errors
	 */
	EShaderValidationResult validateShaderProgram(GLuint program, std::string& validationMessage);


	/**
	 * Extracts all shader uniforms, note that the program must be loaded
	 * @param program: The shader program to extract uniform inputs from
	 * @param uniforms: The populated list of uniforms
	 */
	void extractShaderUniforms(GLuint program, UniformDeclarations& outUniforms);


	/**
	 * Extracts all shader attributes, note that the program must be loaded
	 * @param program: The shader program to extract attribute inputs from
	 * @param attributes: The populated list of shader attributes
	 */
	void extractShaderAttributes(GLuint program, VertexAttributes& outAttributes);
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

	template <>
	struct hash<opengl::GLSLType>
	{
		size_t operator()(const opengl::GLSLType& v) const
		{
			return hash<uint8_t>()(static_cast<uint8_t>(v));
		}
	};
}

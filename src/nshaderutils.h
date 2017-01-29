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
	 * Uniform set function, where void* is the data, GLint the location and
	 * GLSizei the number of elements
	 */
	using UniformSetterFunction = std::function<void(const void*, const GLint&, const GLsizei&)>;

	/**
	 * @return the uniform set function for the associated type
	 * returns nullptr if there's no setter found
	 */
	UniformSetterFunction* getUniformSetter(GLSLType type);

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
		ShaderInput(GLuint shaderProgram, std::string& name, GLenum type, GLint location);
		ShaderInput() = delete;

		std::string		mName;				// Name of the shader attribute
		GLenum			mType;				// OpenGL Type of the shader attribute
		GLSLType		mGLSLType;			// System GLSL type
		GLint			mLocation;			// Location of the shader attribute
		GLuint			mShaderProgram;		// Shader this uniform is associated with
	};


	/**
	 * Represents a GLSL uniform variable
	 */
	class UniformVariable : public ShaderInput
	{
	public:
		// Constructor
		UniformVariable(GLuint shaderProgram, std::string& name, GLenum type, GLint location);

		/**
		 * Sets the uniform variable, note that the shader this uniform belongs
		 * to needs be bound before setting it. 
		 * @param data: pointer to data in memory to set. This data needs to be align
		 * with the GLSL type associated with this uniform
		 * @param count: the length of the array
		 */
		void set(const void* data, int count) const;
	};


	/**
	 * Represents a GLSL vertex attribute
	 */
	class VertexAttribute : public ShaderInput
	{
	public:
		// Constructor
		VertexAttribute(GLuint shaderProgram, std::string& name, GLenum type, GLint location);
	};


	// Typedefs
	using UniformVariables = std::unordered_map<std::string, std::unique_ptr<UniformVariable>>;
	using VertexAttributes = std::unordered_map<std::string, std::unique_ptr<VertexAttribute>>;

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
	void extractShaderUniforms(GLuint program, UniformVariables& outUniforms);


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
}

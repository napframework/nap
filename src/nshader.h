#pragma once

// External Includes
#include <string>

namespace opengl
{
	/**
	Shader is a class designed to allow us to load and use a GLSL shader program in
	our OpenGL application. It allows us to provide filenames for the vertex and
	fragment shaders, and then creates the shader. It also lets us bind and
	unbind the GLSL shader program as required.
	*/
	class Shader
	{
	public:
		// Default constructor / destructor
		Shader() = default;

		// Constructor using file
		Shader(const char *vsFile, const char *fsFile); // Constructor for creating a shader from two shader filenames
		
		// Destructor
		virtual ~Shader();

		/**
		 * Creates a vertex and fragment shader program
		 * This call will also check for shader compilation issues
		 * Use isAllocated() to check if the shader has been created
		 * @param vsFile the vertex shader file on disk
		 * @param fsFile the vertex shader file on disk
		 */
		void init(const std::string& vsFile, const std::string& fsFile);

		/**
		 * Binds the GLSL shader program
		 */
		bool bind();

		/**
		 * Unbinds the GLSL shader program
		 */
		bool unbind();

		/**
		 * @return the shader program identifier 
		 */
		unsigned int getId() const;

		/**
		 * @return if the shader program has been allocated
		 */
		bool isAllocated() const		{ return mShaderId != 0; }

		/**
		 * Associate a generic vertex attribute index with a named attribute variable
		 * Note that this call links the shader program again
		 * When not explicetly binding vertex locations the location will be bound automatically
		 * @param index specifies the index of the generic vertex attribute to be bound
		 * @param name the name of the vertex shader attribute variable to which index is to be bound.
		 */
		void bindVertexAttribute(unsigned int index, const std::string& name);

	private:
		unsigned int mShaderId = 0;		// The shader program identifier
		unsigned int mShaderVp = 0;		// The vertex shader identifier
		unsigned int mShaderFp = 0;		// The fragment shader identifier

		/**
		 * Collects all shader program attributes
		 */
		void sampleAttributes();

		/**
		 * Collects all shader program uniforms
		 */
		void sampleUniforms();
	};
}	// opengl
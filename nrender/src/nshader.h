#pragma once

// Local Includes
#include "nshaderutils.h"

// External Includes
#include <string>
#include <sstream>

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
		/**
		 * Known vertex attribute IDs in the system, used for loading/creating meshes with well-known attributes.
		 */
		struct VertexAttributeIDs
		{
			/**
			 * @return Default position shader vertex attribute name: "in_Position"
			 */
			static const std::string getPositionVertexAttr();

			/**
			 * @return Default normal shader vertex attribute name: "in_Normals"
			 */
			static const std::string getNormalVertexAttr();
			
			/**
			 * @return Default UV shader vertex attribute name: "in_UV#"
			 */
			static const std::string getUVVertexAttr(int uvChannel);
			
			/**
			 * @return Default color shader vertex attribute name: "in_Color#"
			 */
			static const std::string getColorVertexAttr(int colorChannel);

			/**
			 * @return Default tangent shader vertex attribute name: "in_Tangent"
			 */
			static const std::string getTangentVertexAttr();

			/**
			 * @return Default bi-tangent shader vertex attribute name: "in_Bitangent"
			 */
			static const std::string getBitangentVertexAttr();
		};



		/**
		 * Shader state, everything above 0 is an error
		 * By default the shader is not loaded
		 */
		enum class State : int8_t
		{
			NotLoaded		= -1,
			Linked			= 0,
			FileError		= 1,
			VertexError		= 2,
			FragmentError	= 3,
			LinkError		= 4
		};

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
		bool isAllocated() const							{ return mShaderId != 0; }

		/**
		 * @return if the shader program is successfully linked
		 * When the program is linked the shaders got loaded successfully
		 * and linked correctly in to the shader program this object manages
		 */
		bool isLinked() const								{ return mState == State::Linked; }

		/**
		 * Sets a uniform variable based on the given type, note that
		 * you need to bind the shader before calling this function
		 * @param type:  the uniform type
		 * @param name:  name of the uniform variable
		 * @param data:  pointer to data in memory
		 * @param count: number of elements to set
		 */
		void setUniform(EGLSLType type, const std::string& name, const void* data);

		/**
		 * Sets a uniform variable based on it's name, note that you need
		 * to bind the shader before calling this function. The type
		 * is automatically retrieved from the associated uniform
		 * 
		 */
		void setUniform(const std::string& name, const void* data);

		/**
		 * @return shader uniform input attribute with associated name, 
		 * nullptr if the uniform is not found
		 * @param name: Name of the uniform attribute to get
		 */
		const UniformDeclaration* getUniform(const std::string& name) const;

		/**
		 * @return all vertex shader attributes
		 */
		const ShaderVertexAttributes& getAttributes() const		{ return mShaderAttributes; }

		/**
		 * @return shader vertex attribute with given name
		 * nullptr if the attribute is not found
		 * @param name: Name of the vertex attribute
		 */
		const ShaderVertexAttribute* getAttribute(const std::string& name) const;

		/**
		 * @return all uniform shader attributes
		 */
		const UniformDeclarations& getUniformDeclarations() const			{ return mUniformDeclarations; }

	private:
		unsigned int mShaderId = 0;				// The shader program identifier
		unsigned int mShaderVp = 0;				// The vertex shader identifier
		unsigned int mShaderFp = 0;				// The fragment shader identifier

		UniformDeclarations mUniformDeclarations;	// Shader program uniform attributes
		ShaderVertexAttributes mShaderAttributes;		// Shader program vertex attribute inputs
		State mState = State::NotLoaded;		// Holds current state of shader program
	};
}	// opengl
// Local Includes
#include "nshaderutils.h"
#include "nglutils.h"

// External Includes
#include <unordered_map>
#include <regex>

namespace opengl
{
	// Holds all the opengl uniform to GL renderer uniform types
	using GLToGLSLMap = std::unordered_map<GLenum, GLSLType>;
	const static GLToGLSLMap glToGLSLMap =
	{
		{ GL_FLOAT,			GLSLType::Float	},
		{ GL_INT,			GLSLType::Int	},
		{ GL_UNSIGNED_INT,	GLSLType::UInt	},
		{ GL_FLOAT_VEC2,	GLSLType::Vec2	},
		{ GL_FLOAT_VEC3,	GLSLType::Vec3	},
		{ GL_FLOAT_VEC4,	GLSLType::Vec4	},
		{ GL_FLOAT_MAT2,	GLSLType::Mat2	},
		{ GL_FLOAT_MAT3,	GLSLType::Mat3	},
		{ GL_FLOAT_MAT4,	GLSLType::Mat4	},
		{ GL_SAMPLER_2D,	GLSLType::Tex2D	}
	};

	// Uniform set functions
	void setFloatData(const void* data, const GLint& location, const GLsizei& count)		
	{
		glUniform1fv(location, count, static_cast<const float*>(data));
	}

	// Set int value
	void setIntData(const void* data, const GLint& location, const GLsizei& count)			
	{
		glUniform1iv(location, count, static_cast<const GLint*>(data));
	}

	// Set unsigned int value
	void setUIntData(const void* data, const GLint& location, const GLsizei& count)			
	{
		glUniform1uiv(location, count, static_cast<const GLuint*>(data));
	}

	// Set vector 2 data, 2 floats
	void setVec2FData(const void* data, const GLint& location, const GLsizei& count)		
	{
		glUniform2fv(location, count, static_cast<const float*>(data));
	}

	// Set vector 3 data, 3 floats
	void setVec3FData(const void* data, const GLint& location, const GLsizei& count)		
	{
		glUniform3fv(location, count, static_cast<const float*>(data));
	}

	// Set vector 4 data, 4 floats
	void setVec4FData(const void* data, const GLint& location, const GLsizei& count)		
	{
		glUniform4fv(location, count, static_cast<const float*>(data));
	}

	// Set matrix 2 data, 2x2 floats
	void setMat2Data(const void* data, const GLint& location, const GLsizei& count)			
	{
		glUniformMatrix2fv(location, count, false, static_cast<const float*>(data));
	}

	// Set matrix 3 data, 3x3 floats
	void setMat3Data(const void* data, const GLint& location, const GLsizei& count)			
	{
		glUniform3fv(location, count, static_cast<const float*>(data));
	}

	// Setter for 4D matrix data, 4x4 floats
	void setMat4Data(const void* data, const GLint& location, const GLsizei& count)			
	{ 
		glUniformMatrix4fv(location, count, GL_FALSE, static_cast<const GLfloat*>(data));
	}

	// Setter for 2D texture data, count isn't used here
	void setTex2dData(const void* data, const GLint& location, const GLsizei& count)
	{
		glUniform1i(location, *(static_cast<const GLint*>(data)));
	}

	// Returns the uniform set function for the opengl 
	// TODO: Protect population with Mutex!
	UniformSetterFunction* getUniformSetter(GLSLType type)
	{
		// Holds all uniform setters
		static std::unordered_map<GLSLType, UniformSetterFunction> uniformSetters;
		if (uniformSetters.size() == 0)
		{
			uniformSetters[GLSLType::Float]	= setFloatData;
			uniformSetters[GLSLType::Int]	= setIntData;
			uniformSetters[GLSLType::UInt]	= setUIntData;
			uniformSetters[GLSLType::Vec2]	= setVec2FData;
			uniformSetters[GLSLType::Vec3]	= setVec3FData;
			uniformSetters[GLSLType::Vec4]	= setVec4FData;
			uniformSetters[GLSLType::Mat2]	= setMat2Data;
			uniformSetters[GLSLType::Mat3]	= setMat3Data;
			uniformSetters[GLSLType::Mat4]	= setMat4Data;
			uniformSetters[GLSLType::Tex2D]  = setTex2dData;
		}

		// Find setter for type
		auto it = uniformSetters.find(type);
		if (it == uniformSetters.end())
		{
			printMessage(MessageType::WARNING, "unable to find uniform set function for uniform type: %d", type);
			return nullptr;
		}

		// Return set function
		return &(it->second);
	}


	// Returns supported uniform type for associated gl type
	opengl::GLSLType getGLSLType(GLenum glType)
	{
		auto it = glToGLSLMap.find(glType);
		if (it == glToGLSLMap.end())
		{
			printMessage(MessageType::ERROR, "unable to find supported uniform for gl type: %d", glType);
			return GLSLType::Unknown;
		}
		return it->second;
	}


	// Constructor
	ShaderInput::ShaderInput(GLuint shaderProgram, std::string& name, GLenum type, GLint location, GLint size) :
		mName(name),
		mType(type),
		mLocation(location),
		mShaderProgram(shaderProgram),
		mSize(size)
	{
		mGLSLType = getGLSLType(mType);
	}


	// Validates part of the shader
	bool validateShader(GLuint shader)
	{
		const unsigned int BUFFER_SIZE = 512;
		char buffer[BUFFER_SIZE];
		memset(buffer, 0, BUFFER_SIZE);
		GLsizei length = 0;

		// If there's info to display do so
		glGetShaderInfoLog(shader, BUFFER_SIZE, &length, buffer); // Ask OpenGL to give us the log associated with the shader
		if (length > 0)
		{
			printMessage(MessageType::ERROR, "shader compile error: %s", buffer);
			return false;
		}
		return true;
	}


	// Validates an entire shader program
	bool validateShaderProgram(GLuint program)
	{
		const unsigned int BUFFER_SIZE = 512;
		char buffer[BUFFER_SIZE];
		memset(buffer, 0, BUFFER_SIZE);
		GLsizei length = 0;

		glGetProgramInfoLog(program, BUFFER_SIZE, &length, buffer); // Ask OpenGL to give us the log associated with the program
		if (length > 0) // If we have any information to display
		{
			printMessage(MessageType::ERROR, "shader program: %d link error: %s", program, buffer);
			return false;
		}

		glValidateProgram(program); // Get OpenGL to try validating the program
		GLint status;
		glGetProgramiv(program, GL_VALIDATE_STATUS, &status); // Find out if the shader program validated correctly
		if (status == GL_FALSE) // If there was a problem validating
		{
			printMessage(MessageType::ERROR, "can't validate shader: %d", program);
			return false;
		}
		return true;
	}


	// Extracts all shader uniforms
	void extractShaderUniforms(GLuint program, UniformVariables& outUniforms)
	{
		outUniforms.clear();

		GLint uniform_count;			// total number of attributes;
		GLint size;						// size of the variable
		GLenum type;					// type of the variable (float, vec3 or mat4, etc)
		const GLsizei bufSize = 256;	// maximum name length
		GLchar name[bufSize];			// variable name in GLSL
		GLsizei length;					// name length

		glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &uniform_count);

		// Sample info shader program info
		for (auto i = 0; i < uniform_count; i++)
		{
			glGetActiveUniform(program, static_cast<GLint>(i), bufSize, &length, &size, &type, name);
			int location = glGetUniformLocation(program, name);
			if (location < 0)
			{
				printMessage(MessageType::ERROR, "unable to query uniform location: %s", name);
				continue;
			}

			// Check if the uniform is supported, ie: has a setter associated with it
			if (getGLSLType(type) == GLSLType::Unknown)
			{
				printMessage(MessageType::WARNING, "unsupported uniform of type: %d", type);
			}

			// Remove possible brackets
			std::string unique_name = std::regex_replace(std::string(name), std::regex("\\[.*\\]"), "");

			// Add
			printMessage(MessageType::INFO, "Uniform: %d, type: %d, name: %s, location: %d", i, (unsigned int)type, name, location);
			outUniforms.emplace(std::make_pair(unique_name, std::make_unique<UniformVariable>(program, std::string(name), type, location, size)));
		}
	}


	// Extract all shader program attributes
	void extractShaderAttributes(GLuint program, VertexAttributes& outAttributes)
	{
		outAttributes.clear();

		GLint attribute_count;			// total number of attributes;
		GLint size;						// size of the variable
		GLenum type;					// type of the variable (float, vec3 or mat4, etc)
		const GLsizei bufSize = 256;	// maximum name length
		GLchar name[bufSize];			// variable name in GLSL
		GLsizei length;					// name length

										// Get number of active attributes
		glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &attribute_count);

		// Sample info shader program info
		for (auto i = 0; i < attribute_count; i++)
		{
			glGetActiveAttrib(program, static_cast<GLint>(i), bufSize, &length, &size, &type, name);
			int location = glGetAttribLocation(program, name);
			if (location < 0)
			{
				printMessage(MessageType::ERROR, "unable to query attribute location: %s", name);
				continue;
			}

			// Add
			printMessage(MessageType::INFO, "Attribute: %d, type: %d, name: %s, location: %d", i, (unsigned int)type, name, location);
			outAttributes.emplace(name, std::make_unique<VertexAttribute>(program, std::string(name), type, location, size));
		}
	}


	// Uniform variable constructor
	UniformVariable::UniformVariable(GLuint shaderProgram, std::string& name, GLenum type, GLint location, GLint size) :
		ShaderInput(shaderProgram, name, type, location, size)	{ }


	// Set uniform
	void UniformVariable::set(const void* data) const
	{
		// Get type
		if (mGLSLType == GLSLType::Unknown)
		{
			printMessage(MessageType::WARNING, "can't set shader uniform: %s, unsupported type: %d", mName.c_str(), mType);
			return;
		}

		// Get set function
		UniformSetterFunction* setter = getUniformSetter(mGLSLType);
		if (setter == nullptr)
		{
			printMessage(MessageType::WARNING, "unable to set uniform: %s, no setter found for type: %d", mName.c_str(), mGLSLType);
			return;
		}

		// Call function
		(*setter)(data, mLocation, mSize);
	}

	
	// Vertex attribute constructor
	VertexAttribute::VertexAttribute(GLuint shaderProgram, std::string& name, GLenum type, GLint location, GLint size) :
		ShaderInput(shaderProgram, name, type, location, size) { }

}
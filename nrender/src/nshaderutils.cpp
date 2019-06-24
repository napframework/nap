// Local Includes
#include "nshaderutils.h"
#include "nglutils.h"

// External Includes
#include <unordered_map>
#include <regex>
#include <assert.h>

namespace opengl
{
	// Holds all the opengl uniform to GL renderer uniform types
	using GLToGLSLMap = std::unordered_map<GLenum, EGLSLType>;
	const static GLToGLSLMap glToGLSLMap =
	{
		{ GL_FLOAT,			EGLSLType::Float	},
		{ GL_INT,			EGLSLType::Int	},
		{ GL_UNSIGNED_INT,	EGLSLType::UInt	},
		{ GL_FLOAT_VEC2,	EGLSLType::Vec2	},
		{ GL_FLOAT_VEC3,	EGLSLType::Vec3	},
		{ GL_FLOAT_VEC4,	EGLSLType::Vec4	},
		{ GL_FLOAT_MAT2,	EGLSLType::Mat2	},
		{ GL_FLOAT_MAT3,	EGLSLType::Mat3	},
		{ GL_FLOAT_MAT4,	EGLSLType::Mat4	},
		{ GL_SAMPLER_2D,	EGLSLType::Tex2D	}
	};


	// Returns supported uniform type for associated gl type
	opengl::EGLSLType getGLSLType(GLenum glType)
	{
		auto it = glToGLSLMap.find(glType);
		if (it == glToGLSLMap.end())
		{
			printMessage(EGLSLMessageType::Error, "unable to find supported uniform for gl type: %d", glType);
			return EGLSLType::Unknown;
		}
		return it->second;
	}


	// Constructor
	ShaderInput::ShaderInput(GLuint shaderProgram, const std::string& name, GLenum type, GLint location, GLint size) :
		mName(name),
		mType(type),
		mLocation(location),
		mShaderProgram(shaderProgram),
		mSize(size)
	{
		mGLSLType = getGLSLType(mType);
	}


	// Validates part of the shader
	EShaderValidationResult validateShader(GLuint shader, std::string& validationMessage)
	{
		const unsigned int BUFFER_SIZE = 1024;
		char message[BUFFER_SIZE];
		memset(message, 0, BUFFER_SIZE);
		GLsizei message_length = 0;

		// Get the shader compilation log
		glGetShaderInfoLog(shader, BUFFER_SIZE, &message_length, message);
		validationMessage = message;

		// Get the compilation status
		GLint compile_status;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);

		if (compile_status == GL_FALSE)
			return EShaderValidationResult::Error;			// Compilation failed
		else if (message_length > 0)
			return EShaderValidationResult::Warning;		// Compilation succeeded, but there were messages
		else
			return EShaderValidationResult::Success;		// Compilation succeeded
	}


	// Validates an entire shader program
	EShaderValidationResult validateShaderProgram(GLuint program, std::string& validationMessage)
	{
		const unsigned int BUFFER_SIZE = 1024;
		char message[BUFFER_SIZE];
		memset(message, 0, BUFFER_SIZE);
		GLsizei message_length = 0;

		// Get the shader link log
		glGetProgramInfoLog(program, BUFFER_SIZE, &message_length, message);
		validationMessage = message;

		// Validate the program
		glValidateProgram(program); 

		// Get the validation status
		GLint validation_status;
		glGetProgramiv(program, GL_VALIDATE_STATUS, &validation_status);
		
		if (validation_status == GL_FALSE)
			return EShaderValidationResult::Error;			// Validation failed
		else if (message_length > 0)
			return EShaderValidationResult::Warning;		// Validation succeeded, but there were messages
		else
			return EShaderValidationResult::Success;		// Validation succeeded
	}


	// Extracts all shader uniforms
	void extractShaderUniforms(GLuint program, UniformDeclarations& outUniforms)
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
				printMessage(EGLSLMessageType::Error, "unable to query uniform location: %s", name);
				continue;
			}

			// Check if the uniform is supported, ie: has a setter associated with it
			if (getGLSLType(type) == EGLSLType::Unknown)
			{
				printMessage(EGLSLMessageType::Warning, "unsupported uniform of type: %d", type);
			}

			// Remove possible brackets
			//std::string unique_name = std::regex_replace(std::string(name), std::regex("\\[.*\\]"), "");

			// Add
			printMessage(EGLSLMessageType::Info, "Uniform: %d, type: %d, name: %s, location: %d", i, (unsigned int)type, name, location);
			outUniforms.emplace(std::make_pair(name, std::make_unique<UniformDeclaration>(program, std::string(name), type, location, size)));
		}
	}


	// Extract all shader program attributes
	void extractShaderAttributes(GLuint program, ShaderVertexAttributes& outAttributes)
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
				printMessage(EGLSLMessageType::Error, "unable to query attribute location: %s", name);
				continue;
			}

			// Add
			printMessage(EGLSLMessageType::Info, "Attribute: %d, type: %d, name: %s, location: %d", i, (unsigned int)type, name, location);
			outAttributes.emplace(name, std::make_unique<ShaderVertexAttribute>(program, std::string(name), type, location, size));
		}
	}

}

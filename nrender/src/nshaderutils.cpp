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

			// The name in the declaration is the name as returned by OpenGL. This means it's the full 'path' to the uniform.
			// The path also includes the array specifier. For example, if you have the following in your shader:
			//
			// uniform float inputs[2]
			// 
			// The name of this uniform will be returned as 'inputs[0]'. However, this is not what we want; we want the user to be able to address
			// this uniform simply by using its name, in this case 'inputs' without the array specifier.
			//
			// We can't just simply strip off all array specifiers, as there might be array elements earlier on the path; we can only strip off the trailing array specifier.
			// For example, if you have the following in your shader:
			//
			// struct SomeStruct
			// {
			//     float mValues[2];
			// }
			//
			// uniform SomeStruct structs[2];
			//
			// And, assuming you use these uniforms, the uniforms returned will be:
			//
			// structs[0].mValues[0]
			// structs[1].mValues[0]
			//
			// In this case, we need to ensure we only strip off the trailing array specifier; the array specifier after the 'structs' name is important information about which 
			// array element is being indexed.
			
			std::string unique_name = name;

			// Determine where we should start searching for the array specifier: if this is a path with multiple elements, we start at the start of the last element.
			// If it's only a single element path, we search from the start.
			size_t bracket_start_search_pos = unique_name.find_last_of('.');
			if (bracket_start_search_pos == std::string::npos)
				bracket_start_search_pos = 0;

			// Find the array specifier starting at the position we found above; if found, strip it off.
			size_t bracket_pos = unique_name.find_first_of('[', bracket_start_search_pos);
			if (bracket_pos != std::string::npos)
				unique_name = unique_name.substr(0, bracket_pos);

			// Add
			printMessage(EGLSLMessageType::Info, "Uniform: %d, type: %d, name: %s, location: %d", i, (unsigned int)type, unique_name, location);
			outUniforms.emplace(std::make_pair(unique_name, std::make_unique<UniformDeclaration>(program, std::string(unique_name), type, location, size)));
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

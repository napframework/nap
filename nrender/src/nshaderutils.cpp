// Local Includes
#include "nshaderutils.h"
#include "nglutils.h"

// External Includes
#include <unordered_map>
#include <regex>
#include <assert.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(opengl::UniformDeclaration)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(opengl::UniformValueDeclaration)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(opengl::UniformStructDeclaration)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(opengl::UniformStructArrayDeclaration)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(opengl::UniformValueArrayDeclaration)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(opengl::UniformBufferObjectDeclaration)
RTTI_END_CLASS


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
		{ GL_FLOAT_MAT4,	EGLSLType::Mat4	}
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
	ShaderInput::ShaderInput(const std::string& name, int location, VkFormat format) :
		mName(name),
		mLocation(location),
		mFormat(format)
	{
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
}

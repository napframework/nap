// Local Includes
#include "nshader.h"
#include "nglutils.h"

// External Includes
#include <string>
#include <fstream>
#include <GL/glew.h>
#include <iostream>

using namespace std; // Include the standard namespace

/**
 textFileRead loads in a standard text file from a given filename and
 then returns it as a string.
 */
static string textFileRead(const std::string& fileName)
{
	string fileString = string(); // A string for storing the file contents
	string line = string(); // A string for holding the current line

	ifstream file(fileName); // Open an input stream with the selected file
	if (file.is_open())
	{ // If the file opened successfully
		while (!file.eof())
		{ // While we are not at the end of the file
			getline(file, line); // Get the current line
			fileString.append(line); // Append the line to our file string
			fileString.append("\n"); // Appand a new line character
		}
		file.close(); // Close the file
	}

	return fileString; // Return our string
}

namespace opengl
{
	/**
	Given a shader and the filename associated with it, validateShader will
	then get information from OpenGl on whether or not the shader was compiled successfully
	and if it wasn't, it will output the file with the problem, as well as the problem.
	*/
	static void validateShader(GLuint shader, const std::string& file = 0)
	{
		const unsigned int BUFFER_SIZE = 512;
		char buffer[BUFFER_SIZE];
		memset(buffer, 0, BUFFER_SIZE);
		GLsizei length = 0;

		// If there's info to display do so
		glGetShaderInfoLog(shader, BUFFER_SIZE, &length, buffer); // Ask OpenGL to give us the log associated with the shader
		if (length > 0) 
			printMessage(MessageType::ERROR, "shader: %s compile error: %s", file.c_str(), buffer);
	}


	/**
	 Given a shader program, validateProgram will request from OpenGL, any information
	 related to the validation or linking of the program with it's attached shaders. It will
	 then output any issues that have occurred.
	 */
	static bool validateProgram(GLuint program)
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

	/**
	 Constructor for a Shader object which creates a GLSL shader based on a given
	 vertex and fragment shader file.
	 */
	Shader::Shader(const char *vsFile, const char *fsFile) 
	{
		init(vsFile, fsFile); // Initialize the shader
	}


	/**
	 init will take a vertex shader file and fragment shader file, and then attempt to create a valid
	 shader program from these. It will also check for any shader compilation issues along the way.
	 */
	void Shader::init(const std::string& vsFile, const std::string& fsFile) 
	{
		if (isAllocated())
		{
			printMessage(MessageType::WARNING, "shader already allocated: %s, %s", vsFile.c_str(), fsFile.c_str());
			return;
		}

		std::string vsText = textFileRead(vsFile); // Read in the vertex shader
		std::string fsText = textFileRead(fsFile); // Read in the fragment shader

		// If either the vertex or fragment shader wouldn't load
		if (vsText.empty() || fsText.empty())
		{
			printMessage(MessageType::ERROR, "either vertex shader or fragment shader file not found");
			return;
		}

		const char *vertexText = vsText.c_str();
		const char *fragmentText = fsText.c_str();

		mShaderVp = glCreateShader(GL_VERTEX_SHADER);		// Create a vertex shader
		glShaderSource(mShaderVp, 1, &vertexText, 0);		// Set the source for the vertex shader to the loaded text
		glCompileShader(mShaderVp);							// Compile the vertex shader
		validateShader(mShaderVp, vsFile.c_str());			// Validate the vertex shader

		mShaderFp = glCreateShader(GL_FRAGMENT_SHADER);		// Create a fragment shader
		glShaderSource(mShaderFp, 1, &fragmentText, 0);		// Set the source for the fragment shader to the loaded text
		glCompileShader(mShaderFp);							// Compile the fragment shader
		validateShader(mShaderFp, fsFile.c_str());			// Validate the fragment shader

		mShaderId = glCreateProgram();						// Create a GLSL program
		glAttachShader(mShaderId, mShaderVp);				// Attach a vertex shader to the program
		glAttachShader(mShaderId, mShaderFp);				// Attach the fragment shader to the program

		glLinkProgram(mShaderId);							// Link the vertex and fragment shaders in the program
		if (!validateProgram(mShaderId))
		{
			printMessage(MessageType::ERROR, "unable to validate shader program: %s, %s", vsFile.c_str(), fsFile.c_str());
			return;
		}

		// Sample all program attributes
		printMessage(MessageType::INFO, "sampling shader program attributes: %s", vsFile.c_str());
		sampleAttributes();
		printMessage(MessageType::INFO, "sampling shader program uniforms: %s", vsFile.c_str());
		sampleUniforms();
	}


	 // Destructor for the Shader object which cleans up by detaching the shaders, deleting them
	 // and finally deleting the GLSL program.
	Shader::~Shader() 
	{
		if (!isAllocated())
			return;
			
		glDetachShader(mShaderId, mShaderFp); // Detach the fragment shader
		glDetachShader(mShaderId, mShaderVp); // Detach the vertex shader

		glDeleteShader(mShaderFp);				// Delete the fragment shader
		glDeleteShader(mShaderVp);				// Delete the vertex shader
		glDeleteProgram(mShaderId);				// Delete the shader program
	}


	// returns the integer value associated with the shader program
	unsigned int Shader::getId() const
	{
		return mShaderId;
	}


	// associate a generic vertex attribute index with a named attribute variable
	void Shader::bindVertexAttribute(unsigned int index, const std::string& name)
	{
		if (!isAllocated())
		{
			printMessage(MessageType::ERROR, "unable to bind vertex attribute to shader, shader not allocated");
			return;
		}

		// Bind attribute location
		glBindAttribLocation(getId(), index, name.c_str());

		// Re-link program
		glLinkProgram(mShaderId);
	}


	// Gather all shader attributes
	void Shader::sampleAttributes()
	{
		GLint attribute_count;			// total number of attributes;
		GLint size;						// size of the variable
		GLenum type;					// type of the variable (float, vec3 or mat4, etc)
		const GLsizei bufSize = 256;	// maximum name length
		GLchar name[bufSize];			// variable name in GLSL
		GLsizei length;					// name length

		// Get number of active attributes
		glGetProgramiv(getId(), GL_ACTIVE_ATTRIBUTES, &attribute_count);
		
		// Sample info shader program info
		for (auto i = 0; i < attribute_count; i++)
		{
			glGetActiveAttrib(getId(), static_cast<GLint>(i), bufSize, &length, &size, &type, name);
			int location = glGetAttribLocation(getId(), name);
			printMessage(MessageType::INFO, "Attribute: %d, type: %d, name: %s, location: %d", i, (unsigned int)type, name, location);
		}
	}


	// Sampling uniforms
	void Shader::sampleUniforms()
	{
		GLint uniform_count;			// total number of attributes;
		GLint size;						// size of the variable
		GLenum type;					// type of the variable (float, vec3 or mat4, etc)
		const GLsizei bufSize = 256;	// maximum name length
		GLchar name[bufSize];			// variable name in GLSL
		GLsizei length;					// name length

		glGetProgramiv(getId(), GL_ACTIVE_UNIFORMS, &uniform_count);

		// Sample info shader program info
		for (auto i = 0; i < uniform_count; i++)
		{
			glGetActiveUniform(getId(), static_cast<GLint>(i), bufSize, &length, &size, &type, name);
			int location = glGetUniformLocation(getId(), name);
			printMessage(MessageType::INFO, "Uniform: %d, type: %d, name: %s, location: %d", i, (unsigned int)type, name, location);
		}
	}


	// bind attaches the shader program for successive OpenGL calls
	bool Shader::bind()
	{
		if (!isAllocated())
		{
			printMessage(MessageType::ERROR, "unable to bind shader, shader not allocated");
			return false;
		}

		glUseProgram(mShaderId);
		return true;
	}


	// unbind detaches the shader program for further OpenGL calls
	bool Shader::unbind()
	{
		if (!isAllocated())
		{
			printMessage(MessageType::ERROR, "unable to unbind shader, shader not allocated");
			return false;
		}

		glUseProgram(0);
		return true;
	}
}
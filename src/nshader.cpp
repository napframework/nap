// Local Includes
#include "nshader.h"
#include "nglutils.h"

// External Includes
#include <string>
#include <fstream>
#include <GL/glew.h>
#include <iostream>
#include <assert.h>

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
		// Set state
		mState = State::NotLoaded;

		// Clear attributes
		mShaderAttributes.clear();
		mShaderUniforms.clear();

		// Create GPU side shader resources
		if (!isAllocated())
		{
			mShaderVp = glCreateShader(GL_VERTEX_SHADER);		// Create vertex shader
			mShaderFp = glCreateShader(GL_FRAGMENT_SHADER);		// Create fragment shader
			mShaderId = glCreateProgram();						// Create shader program
			glAttachShader(mShaderId, mShaderVp);				// Attach a vertex shader to the program
			glAttachShader(mShaderId, mShaderFp);				// Attach the fragment shader to the program
		}

		std::string vsText = textFileRead(vsFile); // Read in the vertex shader
		std::string fsText = textFileRead(fsFile); // Read in the fragment shader

		// If either the vertex or fragment shader wouldn't load
		if (vsText.empty() || fsText.empty())
		{
			printMessage(MessageType::ERROR, "either vertex shader or fragment shader file not found");
			mState = State::FileError;
			return;
		}

		const char *vertexText   = vsText.c_str();
		const char *fragmentText = fsText.c_str();

		// Load vertex shader
		glShaderSource(mShaderVp, 1, &vertexText, 0);		// Set the source for the vertex shader to the loaded text
		glCompileShader(mShaderVp);							// Compile the vertex shader
		if (!validateShader(mShaderVp))						// Validate the vertex shader
		{
			printMessage(MessageType::ERROR, "unable to validate vertex shader: %s", vertexText);
			mState = State::VertexError;
			return;
		}

		// Load fragment shader
		glShaderSource(mShaderFp, 1, &fragmentText, 0);		// Set the source for the fragment shader to the loaded text
		glCompileShader(mShaderFp);							// Compile the fragment shader
		if (!validateShader(mShaderFp))
		{
			printMessage(MessageType::ERROR, "unable to validate fragment shader: %s", fragmentText);
			mState = State::FragmentError;
			return;
		}													// Validate the fragment shader

		// Link program
		glLinkProgram(mShaderId);							// Link the vertex and fragment shaders in the program
		if (!validateShaderProgram(mShaderId))
		{
			printMessage(MessageType::ERROR, "unable to validate shader program: %s, %s", vsFile.c_str(), fsFile.c_str());
			mState = State::LinkError;
			return;
		}

		// Extract all program vertex attributes
		printMessage(MessageType::INFO, "sampling shader program attributes: %s", vsFile.c_str());
		extractShaderAttributes(mShaderId, mShaderAttributes);

		// Extract all program uniform attributes
		printMessage(MessageType::INFO, "sampling shader program uniforms: %s", vsFile.c_str());
		extractShaderUniforms(mShaderId, mShaderUniforms);
		
		// Successfully loaded shader
		mState = State::Linked;
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

		// Get current location
		GLint current_attr_location = glGetAttribLocation(getId(), name.c_str());
		if (current_attr_location == -1)
		{
			printMessage(MessageType::WARNING, "unable to bind vertex attribute, attribute: %s not in shader", name.c_str());
			return;
		}

		// Check if location is different, if not don't update
		if (current_attr_location == index)
		{
			return;
		}

		// Bind attribute location
		glBindAttribLocation(getId(), index, name.c_str());

		// Cache change
		mAttributeLocationChanged = true;
	}


	// Sets the uniform value in shader based on type
	void Shader::setUniform(GLSLType type, const std::string& name, const void* data)
	{
		// Make sure shader is linked
		if (!isLinked())
		{
			printMessage(MessageType::ERROR, "unable to set shader uniform: %s, shader not linked", name.c_str());
			return;
		}

		// Get uniform
		const UniformVariable* uniform_binding = getUniform(name);
		if (uniform_binding == nullptr)
			return;

		// Make sure uniform types match
		if (uniform_binding->mGLSLType != type)
		{
			printMessage(MessageType::WARNING, "shader uniform: %s types don't match", name.c_str());
			return;
		}

		uniform_binding->set(data);
	}


	// Set uniform using auto type resolving
	void Shader::setUniform(const std::string& name, const void* data)
	{
		// Make sure shader is linked
		if (!isLinked())
		{
			printMessage(MessageType::ERROR, "unable to set shader uniform: %s, shader not linked", name.c_str());
			return;
		}

		// Get uniform
		const UniformVariable* uniform_binding = getUniform(name);
		if (uniform_binding == nullptr)
			return;

		uniform_binding->set(data);
	}


	// Returns a uniform shader input with name
	const UniformVariable* Shader::getUniform(const std::string& name) const
	{
		// Find uniform with name
		auto it = mShaderUniforms.find(name);
		if (it == mShaderUniforms.end())
		{
			printMessage(MessageType::WARNING, "shader has no active uniform with name: %s", name.c_str());
			return nullptr;
		}

		// Store
		return it->second.get();
	}


	// Returns a vertex attribute with name
	const VertexAttribute* Shader::getAttribute(const std::string& name) const
	{
		auto it = mShaderAttributes.find(name);
		if (it == mShaderAttributes.end())
		{
			printMessage(MessageType::WARNING, "shader has no active vertex attribute with name: %s", name.c_str());
			return nullptr;
		}
		return it->second.get();
	}


	// bind attaches the shader program for successive OpenGL calls
	bool Shader::bind()
	{
		if (!isLinked())
		{
			printMessage(MessageType::ERROR, "attempting to bind unresolved shader");
			return false;
		}

		// Re-link if attribute location changed
		if (mAttributeLocationChanged)
		{
			glLinkProgram(mShaderId);
			mAttributeLocationChanged = false;
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
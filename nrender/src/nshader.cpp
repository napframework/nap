// Local Includes
#include "nshader.h"
#include "nglutils.h"

// External Includes
#include <string>
#include <fstream>
#include <GL/glew.h>
#include <iostream>
#include <assert.h>
#include "spirv_reflect.h"

using namespace std; // Include the standard namespace

static bool tryReadFile(const std::string& filename, std::vector<char>& outBuffer)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open())
		return false;

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	outBuffer = std::move(buffer);

	return true;
}

VkShaderModule createShaderModule(const std::vector<char>& code, VkDevice device) 
{
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
		return nullptr;

	return shaderModule;
}


namespace opengl
{
	const std::string Shader::VertexAttributeIDs::getPositionVertexAttr()		{ return "in_Position"; }
	const std::string Shader::VertexAttributeIDs::getNormalVertexAttr()			{ return "in_Normals"; }
	const std::string Shader::VertexAttributeIDs::getTangentVertexAttr()		{ return "in_Tangent"; }
	const std::string Shader::VertexAttributeIDs::getBitangentVertexAttr()		{ return "in_Bitangent"; }

	const std::string Shader::VertexAttributeIDs::getUVVertexAttr(int uvChannel)
	{
		std::ostringstream stream;
		stream << "in_UV" << uvChannel;
		return stream.str();
	}

	const std::string Shader::VertexAttributeIDs::getColorVertexAttr(int colorChannel)
	{
		std::ostringstream stream;
		stream << "in_Color" << colorChannel;
		return stream.str();
	}


	/**
	 init will take a vertex shader file and fragment shader file, and then attempt to create a valid
	 shader program from these. It will also check for any shader compilation issues along the way.
	 */
	bool Shader::init(VkDevice device, const std::string& vsFile, const std::string& fsFile, nap::utility::ErrorState& errorState)
	{
		std::vector<char> vertexShaderData;
		if (!errorState.check(tryReadFile(vsFile, vertexShaderData), "Unable to read vertex shader file %s", vsFile.c_str()))
			return false;

		mVertexModule = createShaderModule(vertexShaderData, device);
		if (!errorState.check(mVertexModule != nullptr, "Unable to load vertex shader module %s", vsFile.c_str()))
			return false;

		std::vector<char> fragmentShaderData;
		if (!errorState.check(tryReadFile(fsFile, fragmentShaderData), "Unable to read fragment shader file %s", fsFile.c_str()))
			return false;

		mFragmentModule = createShaderModule(fragmentShaderData, device);
		if (!errorState.check(mFragmentModule != nullptr, "Unable to load fragment shader module %s", fsFile.c_str()))
			return false;

		SpvReflectShaderModule vertexShaderReflectModule;
		if (!errorState.check(spvReflectCreateShaderModule(vertexShaderData.size(), vertexShaderData.data(), &vertexShaderReflectModule) == SPV_REFLECT_RESULT_SUCCESS, "Failed to create shader reflection module"))
			return false;

		uint32_t count = 0;
		if (!errorState.check(spvReflectEnumerateInputVariables(&vertexShaderReflectModule, &count, NULL) == SPV_REFLECT_RESULT_SUCCESS, "Failed to enumerate shader input variables"))
			return false;		

		std::vector<SpvReflectInterfaceVariable*> input_vars(count);
		if (!errorState.check(spvReflectEnumerateInputVariables(&vertexShaderReflectModule, &count, input_vars.data()) == SPV_REFLECT_RESULT_SUCCESS, "Failed to enumerate shader input variables"))
			return false;
		
		for (const SpvReflectInterfaceVariable* refl_var : input_vars)
			mShaderAttributes[refl_var->name] = std::make_unique<ShaderInput>(refl_var->name, refl_var->location, refl_var->format);

		spvReflectDestroyShaderModule(&vertexShaderReflectModule);

		/*
		// Set state
		mState = State::NotLoaded;

		// Clear attributes
		mShaderAttributes.clear();
		mUniformDeclarations.clear();

		// Create GPU side shader resources
		if (!isAllocated())
		{
			mShaderVp = glCreateShader(GL_VERTEX_SHADER);		// Create vertex shader
			glAssert();
			
			mShaderFp = glCreateShader(GL_FRAGMENT_SHADER);		// Create fragment shader
			glAssert();
			
			mShaderId = glCreateProgram();						// Create shader program
			glAssert();

			glAttachShader(mShaderId, mShaderVp);				// Attach a vertex shader to the program
			glAssert();
			
			glAttachShader(mShaderId, mShaderFp);				// Attach the fragment shader to the program
			glAssert();
		}

		std::string vsText = textFileRead(vsFile); // Read in the vertex shader
		std::string fsText = textFileRead(fsFile); // Read in the fragment shader

		// If either the vertex or fragment shader wouldn't load
		if (vsText.empty() || fsText.empty())
		{
			printMessage(EGLSLMessageType::Error, "either vertex shader or fragment shader file not found");
			mState = State::FileError;
			return;
		}

		const char *vertexText   = vsText.c_str();
		const char *fragmentText = fsText.c_str();

		// Load vertex shader
		glShaderSource(mShaderVp, 1, &vertexText, 0);		// Set the source for the vertex shader to the loaded text
		glAssert();
		
		glCompileShader(mShaderVp);							// Compile the vertex shader

		// Validate the vertex shader
		std::string vertex_compile_message;
		EShaderValidationResult vertex_compile_result = validateShader(mShaderVp, vertex_compile_message);
		if (vertex_compile_result == EShaderValidationResult::Error)
		{
			printMessage(EGLSLMessageType::Error, "Unable to compile vertex shader (%s): %s \r\n%s", vsFile.c_str(), vertex_compile_message.c_str(), vertexText);
			mState = State::VertexError;
			return;
		}
		else if (vertex_compile_result == EShaderValidationResult::Warning)
		{
			printMessage(EGLSLMessageType::Warning, "Compilation of vertex shader (%s) succeeded, but with warnings: %s", vsFile.c_str(), vertex_compile_message.c_str());
		}
		
		// Load fragment shader
		glShaderSource(mShaderFp, 1, &fragmentText, 0);		// Set the source for the fragment shader to the loaded text
		glAssert();
		
		glCompileShader(mShaderFp);							// Compile the fragment shader
		
		// Validate the fragment shader
		std::string fragment_compile_message;
		EShaderValidationResult fragment_compile_result = validateShader(mShaderFp, fragment_compile_message);
		if (fragment_compile_result == EShaderValidationResult::Error)
		{
			printMessage(EGLSLMessageType::Error, "Unable to compile fragment shader (%s): %s \r\n%s", fsFile.c_str(), fragment_compile_message.c_str(), fragmentText);
			mState = State::FragmentError;
			return;
		}													
		else if (fragment_compile_result == EShaderValidationResult::Warning)
		{
			printMessage(EGLSLMessageType::Warning, "Compilation of fragment shader (%s) succeeded, but with warnings: %s", fsFile.c_str(), fragment_compile_message.c_str());
		}

		// Link the vertex and fragment shaders in the program
		glLinkProgram(mShaderId);
		std::string program_validation_message;

		// Extract all program vertex attributes
		printMessage(EGLSLMessageType::Info, "sampling shader program attributes: %s", vsFile.c_str());
		extractShaderAttributes(mShaderId, mShaderAttributes);

		// Extract all program uniform attributes
		printMessage(EGLSLMessageType::Info, "sampling shader program uniforms: %s", vsFile.c_str());
		extractShaderUniforms(mShaderId, mUniformDeclarations);
		
		// Successfully loaded shader
		mState = State::Linked;
		*/

		return true;
	}


	 // Destructor for the Shader object which cleans up by detaching the shaders, deleting them
	 // and finally deleting the GLSL program.
	Shader::~Shader() 
	{/*
		if (!isAllocated())
			return;
			
		glDetachShader(mShaderId, mShaderFp); // Detach the fragment shader
		glDetachShader(mShaderId, mShaderVp); // Detach the vertex shader

		glDeleteShader(mShaderFp);				// Delete the fragment shader
		glDeleteShader(mShaderVp);				// Delete the vertex shader
		glDeleteProgram(mShaderId);				// Delete the shader program
		*/
	}


	// returns the integer value associated with the shader program
	unsigned int Shader::getId() const
	{
		return mShaderId;
	}


	// Returns a uniform shader input with name
	const UniformDeclaration* Shader::getUniform(const std::string& name) const
	{
		// Find uniform with name
		auto it = mUniformDeclarations.find(name);
		if (it == mUniformDeclarations.end())
		{
			printMessage(EGLSLMessageType::Warning, "shader has no active uniform with name: %s", name.c_str());
			return nullptr;
		}

		// Store
		return it->second.get();
	}


	// Returns a vertex attribute with name
	const ShaderVertexAttribute* Shader::getAttribute(const std::string& name) const
	{
		auto it = mShaderAttributes.find(name);
		if (it == mShaderAttributes.end())
		{
			printMessage(EGLSLMessageType::Warning, "shader has no active vertex attribute with name: %s", name.c_str());
			return nullptr;
		}
		return it->second.get();
	}


	// bind attaches the shader program for successive OpenGL calls
	bool Shader::bind()
	{
		if (!isLinked())
		{
			printMessage(EGLSLMessageType::Error, "attempting to bind unresolved shader");
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
			printMessage(EGLSLMessageType::Error, "unable to unbind shader, shader not allocated");
			return false;
		}

		glUseProgram(0);
		return true;
	}
}

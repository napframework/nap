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
#include "utility/stringutils.h"

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
	const std::string Shader::VertexAttributeIDs::getPositionVertexAttr() { return "in_Position"; }
	const std::string Shader::VertexAttributeIDs::getNormalVertexAttr() { return "in_Normals"; }
	const std::string Shader::VertexAttributeIDs::getTangentVertexAttr() { return "in_Tangent"; }
	const std::string Shader::VertexAttributeIDs::getBitangentVertexAttr() { return "in_Bitangent"; }

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



	bool Shader::parseUniforms(const SpvReflectShaderModule& inShaderModule, nap::utility::ErrorState& errorState)
	{
		uint32_t count = 0;
		if (!errorState.check(spvReflectEnumerateDescriptorBindings(&inShaderModule, &count, NULL) == SPV_REFLECT_RESULT_SUCCESS, "Failed to enumerate shader bindings"))
			return false;

		std::vector<SpvReflectDescriptorBinding*> bindings(count);
		if (!errorState.check(spvReflectEnumerateDescriptorBindings(&inShaderModule, &count, bindings.data()) == SPV_REFLECT_RESULT_SUCCESS, "Failed to enumerate shader bindings"))
			return false;

		for (const SpvReflectDescriptorBinding* binding : bindings)
		{
			if (binding->type_description->op != SpvOpTypeStruct)
				continue;

			UniformBufferObjectDeclaration uniform_buffer_object(binding->name, binding->binding, inShaderModule.vulkan_shader_stage, binding->block.size);

			for (int member_index = 0; member_index < binding->block.member_count; ++member_index)
			{
				SpvReflectBlockVariable& member = binding->block.members[member_index];

				EGLSLType type;
				if (member.type_description->op == SpvOpTypeInt)
				{
					if (member.type_description->traits.numeric.scalar.signedness == 1)
						type = EGLSLType::Int;
					else
						type = EGLSLType::UInt;
				}
				else if (member.type_description->op == SpvOpTypeFloat)
				{
					type = EGLSLType::Float;
				}
				else if (member.type_description->op == SpvOpTypeVector)
				{
					int component_count = member.type_description->traits.numeric.vector.component_count;

					if (!errorState.check(component_count >= 2 && component_count <= 4, "Encountered vector uniform with unsupported number of component"))
						return false;

					if (component_count == 2)
						type = EGLSLType::Vec2;
					else if (component_count == 3)
						type = EGLSLType::Vec3;
					else
						type = EGLSLType::Vec4;
				}
				else if (member.type_description->op == SpvOpTypeMatrix)
				{
					int rows = member.type_description->traits.numeric.matrix.row_count;
					int columns = member.type_description->traits.numeric.matrix.column_count;
					if (!errorState.check(rows == columns, "Non-square matrix uniforms are not supported"))
						return false;

					if ((!errorState.check(rows >= 2 && rows <= 4, "Encountered matrix uniform with unsupported dimensions")))
						return false;

					if (rows == 2)
						type = EGLSLType::Mat2;
					else if (rows == 3)
						type = EGLSLType::Mat3;
					else
						type = EGLSLType::Mat4;
				}
				else
				{
					errorState.fail("Encountered unknown uniform type");
					return false;
				}

				std::string name = nap::utility::stringFormat("%s.%s", binding->name, member.name);

				std::unique_ptr<UniformDeclaration> uniform_declaration = std::make_unique<UniformDeclaration>(name, member.offset, member.size, type);
				mUniformDeclarations[name] = uniform_declaration.get();
				uniform_buffer_object.mDeclarations.emplace_back(std::move(uniform_declaration));
			}

			mUniformBufferObjectDeclarations.emplace_back(std::move(uniform_buffer_object));
		}

		return true;
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
		if (!errorState.check(spvReflectCreateShaderModule(vertexShaderData.size(), vertexShaderData.data(), &vertexShaderReflectModule) == SPV_REFLECT_RESULT_SUCCESS, "Failed to create vertex shader reflection module"))
			return false;

		{
			uint32_t count = 0;
			if (!errorState.check(spvReflectEnumerateInputVariables(&vertexShaderReflectModule, &count, NULL) == SPV_REFLECT_RESULT_SUCCESS, "Failed to enumerate shader input variables"))
				return false;

			std::vector<SpvReflectInterfaceVariable*> input_vars(count);
			if (!errorState.check(spvReflectEnumerateInputVariables(&vertexShaderReflectModule, &count, input_vars.data()) == SPV_REFLECT_RESULT_SUCCESS, "Failed to enumerate shader input variables"))
				return false;

			for (const SpvReflectInterfaceVariable* refl_var : input_vars)
				mShaderAttributes[refl_var->name] = std::make_unique<ShaderInput>(refl_var->name, refl_var->location, refl_var->format);
		}

		if (!parseUniforms(vertexShaderReflectModule, errorState))
			return false;

		SpvReflectShaderModule fragmentShaderReflectModule;
		if (!errorState.check(spvReflectCreateShaderModule(fragmentShaderData.size(), fragmentShaderData.data(), &fragmentShaderReflectModule) == SPV_REFLECT_RESULT_SUCCESS, "Failed to create fragment shader reflection module"))
			return false;

		if (!parseUniforms(fragmentShaderReflectModule, errorState))
			return false;

		spvReflectDestroyShaderModule(&vertexShaderReflectModule);

		return true;
	}


	// Destructor for the Shader object which cleans up by detaching the shaders, deleting them
	// and finally deleting the GLSL program.
	Shader::~Shader()
	{
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
		return it->second;
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
}

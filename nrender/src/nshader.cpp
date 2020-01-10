// Local Includes
#include "nshader.h"
#include "nglutils.h"

// External Includes
#include <string>
#include <fstream>
#include <GL/glew.h>
#include <iostream>
#include <assert.h>
#include "utility/stringutils.h"
#include "spirv_cross/spirv_cross.hpp"
#include "spirv_cross/spirv_parser.hpp"

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


	EGLSLType getGLSLType(spirv_cross::SPIRType type)
	{
		switch (type.basetype)
		{
		case spirv_cross::SPIRType::Int:
			return EGLSLType::Int;
		case spirv_cross::SPIRType::UInt:
			return EGLSLType::UInt;
		case spirv_cross::SPIRType::Float:
			if (type.vecsize == 1 && type.columns == 1)
				return EGLSLType::Float;
			else if (type.vecsize == 2 && type.columns == 1)
				return EGLSLType::Vec2;
			else if (type.vecsize == 3 && type.columns == 1)
				return EGLSLType::Vec3;
			else if (type.vecsize == 4 && type.columns == 1)
				return EGLSLType::Vec4;
			else if (type.vecsize == 2 && type.columns == 2)
				return EGLSLType::Mat2;
			else if (type.vecsize == 3 && type.columns == 3)
				return EGLSLType::Mat3;
			else if (type.vecsize == 4 && type.columns == 4)
				return EGLSLType::Mat4;
			else
				return EGLSLType::Unknown;
		default:
			return EGLSLType::Unknown;
		}
	}

	VkFormat getFormatFromType(spirv_cross::SPIRType type)
	{
		switch (type.basetype)
		{
		case spirv_cross::SPIRType::Int:
			return VK_FORMAT_R32_SINT;
		case spirv_cross::SPIRType::SByte:
			return VK_FORMAT_R8_SINT;
		case spirv_cross::SPIRType::Float:
			if (type.vecsize == 1 && type.columns == 1)
				return VK_FORMAT_R32_SFLOAT;
			else if (type.vecsize == 2 && type.columns == 1)
				return VK_FORMAT_R32G32_SFLOAT;
			else if (type.vecsize == 3 && type.columns == 1)
				return VK_FORMAT_R32G32B32_SFLOAT;
			else if (type.vecsize == 4 && type.columns == 1)
				return VK_FORMAT_R32G32B32A32_SFLOAT;
			else
				return VK_FORMAT_UNDEFINED;
		case spirv_cross::SPIRType::Double:
			return VK_FORMAT_R64_SFLOAT;
		default:
			return VK_FORMAT_UNDEFINED;
		}
	}

	bool addUniformsRecursive(UniformStructDeclaration& parentStruct, spirv_cross::Compiler& compiler, const spirv_cross::SPIRType& type, int parentOffset, const std::string& path, nap::utility::ErrorState& errorState)
	{
		assert(type.basetype == spirv_cross::SPIRType::Struct);

		for (int index = 0; index < type.member_types.size(); ++index)
		{
			spirv_cross::SPIRType member_type = compiler.get_type(type.member_types[index]);

			std::string name = compiler.get_member_name(type.self, index);
			int absoluteOffset = parentOffset + compiler.type_struct_member_offset(type, index);
			size_t member_size = compiler.get_declared_struct_member_size(type, index);

			std::string full_path = path + "." + name;

			if (!errorState.check(member_type.array.size() <= 1, "Multidimensional arrays are not supported"))
				return false;

			bool isArray = !member_type.array.empty();
			if (isArray)
			{
				int num_elements = member_type.array[0];

				if (member_type.basetype == spirv_cross::SPIRType::Struct)
				{
					size_t stride = compiler.type_struct_member_array_stride(type, index);
					size_t struct_size = compiler.get_declared_struct_size(member_type);

					std::unique_ptr<UniformStructArrayDeclaration> array_declaration = std::make_unique<UniformStructArrayDeclaration>(name, absoluteOffset, member_size);

					for (int array_index = 0; array_index < num_elements; ++array_index)
					{
						std::string array_path = nap::utility::stringFormat("%s[%d]", full_path.c_str(), array_index);

						std::unique_ptr<UniformStructDeclaration> struct_declaration = std::make_unique<UniformStructDeclaration>(name, absoluteOffset, struct_size);
						if (!addUniformsRecursive(*struct_declaration, compiler, member_type, absoluteOffset, array_path, errorState))
							return false;

						array_declaration->mElements.emplace_back(std::move(struct_declaration));
						absoluteOffset += stride;
					}

					parentStruct.mMembers.emplace_back(std::move(array_declaration));
				}
				else
				{
					EGLSLType element_type = getGLSLType(member_type);
					if (!errorState.check(element_type != EGLSLType::Unknown, "Encountered unknown uniform type"))
						return false;

					std::unique_ptr<UniformValueArrayDeclaration> array_declaration = std::make_unique<UniformValueArrayDeclaration>(name, absoluteOffset, member_size, element_type, num_elements);
					parentStruct.mMembers.emplace_back(std::move(array_declaration));
				}
			}
			else
			{
				if (member_type.basetype == spirv_cross::SPIRType::Struct)
				{
					size_t struct_size = compiler.get_declared_struct_size(member_type);

					std::unique_ptr<UniformStructDeclaration> struct_declaration = std::make_unique<UniformStructDeclaration>(name, absoluteOffset, struct_size);
					if (!addUniformsRecursive(*struct_declaration, compiler, member_type, absoluteOffset, name, errorState))
						return false;

					parentStruct.mMembers.emplace_back(std::move(struct_declaration));
				}
				else
				{
					EGLSLType value_type = getGLSLType(member_type);
					if (!errorState.check(value_type != EGLSLType::Unknown, "Encountered unknown uniform type"))
						return false;

					std::unique_ptr<UniformValueDeclaration> value_declaration = std::make_unique<UniformValueDeclaration>(name, absoluteOffset, member_size, value_type);
					parentStruct.mMembers.emplace_back(std::move(value_declaration));
				}
			}
		}

		return true;
	}


	bool parseUniforms(spirv_cross::Compiler& compiler, VkShaderStageFlagBits inStage, std::vector<UniformBufferObjectDeclaration>& uboDeclarations, std::vector<SamplerDeclaration>& samplerDeclarations, nap::utility::ErrorState& errorState)
	{
		spirv_cross::ShaderResources shader_resources = compiler.get_shader_resources();

		for (const spirv_cross::Resource& resource : shader_resources.uniform_buffers)
		{
			spirv_cross::SPIRType type = compiler.get_type(resource.type_id);

			uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			size_t struct_size = compiler.get_declared_struct_size(type);

			UniformBufferObjectDeclaration uniform_buffer_object(resource.name, binding, inStage, struct_size);

			if (!addUniformsRecursive(uniform_buffer_object, compiler, type, 0, resource.name, errorState))
				return false;

			uboDeclarations.emplace_back(std::move(uniform_buffer_object));
		}

		for (const spirv_cross::Resource& sampled_image : shader_resources.sampled_images)
		{
			spirv_cross::SPIRType sampler_type = compiler.get_type(sampled_image.type_id);

			if (!errorState.check(sampler_type.array.size() <= 1, "Multidimensional arrays are not supported"))
				return false;

			bool is_array = !sampler_type.array.empty();
			int num_elements = 1;
			if (is_array)
				num_elements = sampler_type.array[0];

			SamplerDeclaration::EType type;
			if (sampler_type.image.dim == spv::Dim1D)
			{
				type = SamplerDeclaration::EType::Type_1D;
			}
			else if (sampler_type.image.dim == spv::Dim2D)
			{
				type = SamplerDeclaration::EType::Type_2D;
			}
			else if (sampler_type.image.dim == spv::Dim3D)
			{
				type = SamplerDeclaration::EType::Type_3D;
			}
			else
			{
				errorState.fail("Unsupported sampler type encountered");
				return false;
			}

			uint32_t binding = compiler.get_decoration(sampled_image.id, spv::DecorationBinding);
			samplerDeclarations.emplace_back(SamplerDeclaration(sampled_image.name, binding, inStage, type, num_elements));
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

		spirv_cross::Compiler vertex_shader_compiler((uint32_t*)vertexShaderData.data(), vertexShaderData.size() / sizeof(uint32_t));

		if (!parseUniforms(vertex_shader_compiler, VK_SHADER_STAGE_VERTEX_BIT, mUBODeclarations, mSamplerDeclarations, errorState))
			return false;

		for (const spirv_cross::Resource& stage_input : vertex_shader_compiler.get_shader_resources().stage_inputs)
		{
			spirv_cross::SPIRType input_type = vertex_shader_compiler.get_type(stage_input.type_id);

			VkFormat format = getFormatFromType(input_type);
			if (!errorState.check(format != VK_FORMAT_UNDEFINED, "Encountered unsupported vertex attribute type"))
				return false;

			uint32_t location = vertex_shader_compiler.get_decoration(stage_input.id, spv::DecorationLocation);
			mShaderAttributes[stage_input.name] = std::make_unique<ShaderVertexAttribute>(stage_input.name, location, format);
		}

		std::vector<char> fragmentShaderData;
		if (!errorState.check(tryReadFile(fsFile, fragmentShaderData), "Unable to read fragment shader file %s", fsFile.c_str()))
			return false;

		mFragmentModule = createShaderModule(fragmentShaderData, device);
		if (!errorState.check(mFragmentModule != nullptr, "Unable to load fragment shader module %s", fsFile.c_str()))
			return false;

		spirv_cross::Compiler fragment_shader_compiler((uint32_t*)fragmentShaderData.data(), fragmentShaderData.size() / sizeof(uint32_t));

		if (!parseUniforms(fragment_shader_compiler, VK_SHADER_STAGE_FRAGMENT_BIT, mUBODeclarations, mSamplerDeclarations, errorState))
			return false;

		return initLayout(device, errorState);

		return true;
	}

	bool Shader::initLayout(VkDevice device, nap::utility::ErrorState& errorState)
	{
		std::vector<VkDescriptorSetLayoutBinding> descriptor_set_layouts;
		for (const opengl::UniformBufferObjectDeclaration& declaration : mUBODeclarations)
		{
			VkDescriptorSetLayoutBinding uboLayoutBinding = {};
			uboLayoutBinding.binding = declaration.mBinding;
			uboLayoutBinding.descriptorCount = 1;
			uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			uboLayoutBinding.pImmutableSamplers = nullptr;
			uboLayoutBinding.stageFlags = declaration.mStage;

			descriptor_set_layouts.push_back(uboLayoutBinding);
		}

		for (const opengl::SamplerDeclaration& declaration : mSamplerDeclarations)
		{
			VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
			samplerLayoutBinding.binding = declaration.mBinding;
			samplerLayoutBinding.descriptorCount = 1;
			samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			samplerLayoutBinding.pImmutableSamplers = nullptr;
			samplerLayoutBinding.stageFlags = declaration.mStage;

			descriptor_set_layouts.push_back(samplerLayoutBinding);
		}

		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = (int)descriptor_set_layouts.size();
		layoutInfo.pBindings = descriptor_set_layouts.data();

		return errorState.check(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &mDescriptorSetLayout) == VK_SUCCESS, "Failed to create descriptor set layout");
	}


	// Destructor for the Shader object which cleans up by detaching the shaders, deleting them
	// and finally deleting the GLSL program.
	Shader::~Shader()
	{
	}

}

// Local Includes
#include "shader.h"
#include "material.h"

// External Includes
#include <utility/fileutils.h>
#include <nap/logger.h>
#include "renderservice.h"
#include <fstream>
#include <assert.h>
#include "utility/stringutils.h"
#include "spirv_cross/spirv_cross.hpp"
#include "spirv_cross/spirv_parser.hpp"
#include "glslang/Public/ShaderLang.h"
#include "glslang/SPIRV/GlslangToSpv.h"


RTTI_BEGIN_CLASS(nap::Shader)
	RTTI_PROPERTY_FILELINK("mVertShader", &nap::Shader::mVertPath, nap::rtti::EPropertyMetaData::Required, nap::rtti::EPropertyFileType::VertShader)
	RTTI_PROPERTY_FILELINK("mFragShader", &nap::Shader::mFragPath, nap::rtti::EPropertyMetaData::Required, nap::rtti::EPropertyFileType::FragShader)
	RTTI_PROPERTY("OutputFormat", &nap::Shader::mOutputFormat, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

using namespace std; // Include the standard namespace


					 // From https://github.com/KhronosGroup/glslang/blob/master/StandAlone/ResourceLimits.cpp
const TBuiltInResource defaultResource =
{
	/* .MaxLights = */ 32,
	/* .MaxClipPlanes = */ 6,
	/* .MaxTextureUnits = */ 32,
	/* .MaxTextureCoords = */ 32,
	/* .MaxVertexAttribs = */ 64,
	/* .MaxVertexUniformComponents = */ 4096,
	/* .MaxVaryingFloats = */ 64,
	/* .MaxVertexTextureImageUnits = */ 32,
	/* .MaxCombinedTextureImageUnits = */ 80,
	/* .MaxTextureImageUnits = */ 32,
	/* .MaxFragmentUniformComponents = */ 4096,
	/* .MaxDrawBuffers = */ 32,
	/* .MaxVertexUniformVectors = */ 128,
	/* .MaxVaryingVectors = */ 8,
	/* .MaxFragmentUniformVectors = */ 16,
	/* .MaxVertexOutputVectors = */ 16,
	/* .MaxFragmentInputVectors = */ 15,
	/* .MinProgramTexelOffset = */ -8,
	/* .MaxProgramTexelOffset = */ 7,
	/* .MaxClipDistances = */ 8,
	/* .MaxComputeWorkGroupCountX = */ 65535,
	/* .MaxComputeWorkGroupCountY = */ 65535,
	/* .MaxComputeWorkGroupCountZ = */ 65535,
	/* .MaxComputeWorkGroupSizeX = */ 1024,
	/* .MaxComputeWorkGroupSizeY = */ 1024,
	/* .MaxComputeWorkGroupSizeZ = */ 64,
	/* .MaxComputeUniformComponents = */ 1024,
	/* .MaxComputeTextureImageUnits = */ 16,
	/* .MaxComputeImageUniforms = */ 8,
	/* .MaxComputeAtomicCounters = */ 8,
	/* .MaxComputeAtomicCounterBuffers = */ 1,
	/* .MaxVaryingComponents = */ 60,
	/* .MaxVertexOutputComponents = */ 64,
	/* .MaxGeometryInputComponents = */ 64,
	/* .MaxGeometryOutputComponents = */ 128,
	/* .MaxFragmentInputComponents = */ 128,
	/* .MaxImageUnits = */ 8,
	/* .MaxCombinedImageUnitsAndFragmentOutputs = */ 8,
	/* .MaxCombinedShaderOutputResources = */ 8,
	/* .MaxImageSamples = */ 0,
	/* .MaxVertexImageUniforms = */ 0,
	/* .MaxTessControlImageUniforms = */ 0,
	/* .MaxTessEvaluationImageUniforms = */ 0,
	/* .MaxGeometryImageUniforms = */ 0,
	/* .MaxFragmentImageUniforms = */ 8,
	/* .MaxCombinedImageUniforms = */ 8,
	/* .MaxGeometryTextureImageUnits = */ 16,
	/* .MaxGeometryOutputVertices = */ 256,
	/* .MaxGeometryTotalOutputComponents = */ 1024,
	/* .MaxGeometryUniformComponents = */ 1024,
	/* .MaxGeometryVaryingComponents = */ 64,
	/* .MaxTessControlInputComponents = */ 128,
	/* .MaxTessControlOutputComponents = */ 128,
	/* .MaxTessControlTextureImageUnits = */ 16,
	/* .MaxTessControlUniformComponents = */ 1024,
	/* .MaxTessControlTotalOutputComponents = */ 4096,
	/* .MaxTessEvaluationInputComponents = */ 128,
	/* .MaxTessEvaluationOutputComponents = */ 128,
	/* .MaxTessEvaluationTextureImageUnits = */ 16,
	/* .MaxTessEvaluationUniformComponents = */ 1024,
	/* .MaxTessPatchComponents = */ 120,
	/* .MaxPatchVertices = */ 32,
	/* .MaxTessGenLevel = */ 64,
	/* .MaxViewports = */ 16,
	/* .MaxVertexAtomicCounters = */ 0,
	/* .MaxTessControlAtomicCounters = */ 0,
	/* .MaxTessEvaluationAtomicCounters = */ 0,
	/* .MaxGeometryAtomicCounters = */ 0,
	/* .MaxFragmentAtomicCounters = */ 8,
	/* .MaxCombinedAtomicCounters = */ 8,
	/* .MaxAtomicCounterBindings = */ 1,
	/* .MaxVertexAtomicCounterBuffers = */ 0,
	/* .MaxTessControlAtomicCounterBuffers = */ 0,
	/* .MaxTessEvaluationAtomicCounterBuffers = */ 0,
	/* .MaxGeometryAtomicCounterBuffers = */ 0,
	/* .MaxFragmentAtomicCounterBuffers = */ 1,
	/* .MaxCombinedAtomicCounterBuffers = */ 1,
	/* .MaxAtomicCounterBufferSize = */ 16384,
	/* .MaxTransformFeedbackBuffers = */ 4,
	/* .MaxTransformFeedbackInterleavedComponents = */ 64,
	/* .MaxCullDistances = */ 8,
	/* .MaxCombinedClipAndCullDistances = */ 8,
	/* .MaxSamples = */ 4,
	/* .maxMeshOutputVerticesNV = */ 256,
	/* .maxMeshOutputPrimitivesNV = */ 512,
	/* .maxMeshWorkGroupSizeX_NV = */ 32,
	/* .maxMeshWorkGroupSizeY_NV = */ 1,
	/* .maxMeshWorkGroupSizeZ_NV = */ 1,
	/* .maxTaskWorkGroupSizeX_NV = */ 32,
	/* .maxTaskWorkGroupSizeY_NV = */ 1,
	/* .maxTaskWorkGroupSizeZ_NV = */ 1,
	/* .maxMeshViewCountNV = */ 4,

	/* .limits = */{
		/* .nonInductiveForLoops = */ 1,
		/* .whileLoops = */ 1,
		/* .doWhileLoops = */ 1,
		/* .generalUniformIndexing = */ 1,
		/* .generalAttributeMatrixVectorIndexing = */ 1,
		/* .generalVaryingIndexing = */ 1,
		/* .generalSamplerIndexing = */ 1,
		/* .generalVariableIndexing = */ 1,
		/* .generalConstantMatrixVectorIndexing = */ 1,
	}
};

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


VkShaderModule createShaderModule(const std::vector<unsigned int>& code, VkDevice device)
{
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size() * sizeof(unsigned int);
	createInfo.pCode = code.data();

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
		return nullptr;

	return shaderModule;
}


bool compileShader(VkDevice device, const std::string& file, EShLanguage stage, std::vector<unsigned int>& spirv, nap::utility::ErrorState& errorState)
{
	std::vector<char> shader_source;
	if (!errorState.check(tryReadFile(file, shader_source), "Unable to read shader file %s", file.c_str()))
		return false;

	const char* sources[] = { shader_source.data() };
	int source_sizes[] = { (int)shader_source.size() };
	const char* file_names[] = { file.data() };

	glslang::TShader shader(stage);
	shader.setStringsWithLengthsAndNames(sources, source_sizes, file_names, 1);
	shader.setAutoMapBindings(false);
	shader.setAutoMapLocations(false);

	// 110 = Vulkan 1.1
	int default_version = 110;
	shader.setEnvInput(glslang::EShSourceGlsl, stage, glslang::EShClientVulkan, default_version);
	shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_1);
	shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_3);

	EShMessages messages = EShMsgDefault;
	messages = (EShMessages)(messages | EShMsgSpvRules);
	messages = (EShMessages)(messages | EShMsgVulkanRules);

	bool result = shader.parse(&defaultResource, default_version, false, messages);
	if (!result)
	{
		errorState.fail("Failed to compile shader %s: %s", file.c_str(), shader.getInfoLog());
		return false;
	}

	glslang::SpvOptions spv_options;
	spv_options.generateDebugInfo = false;
	spv_options.disableOptimizer = false;
	spv_options.optimizeSize = false;
	spv_options.disassemble = false;
	spv_options.validate = true;

	spv::SpvBuildLogger logger;
	glslang::GlslangToSpv(*shader.getIntermediate(), spirv, &logger, &spv_options);

	if (!errorState.check(!spirv.empty(), "Failed to compile shader %s: %s", file.c_str(), logger.getAllMessages().c_str()))
		return false;

	return true;
}

nap::EUniformValueType getUniformValueType(spirv_cross::SPIRType type)
{
	switch (type.basetype)
	{
	case spirv_cross::SPIRType::Int:
		return nap::EUniformValueType::Int;
	case spirv_cross::SPIRType::UInt:
		return nap::EUniformValueType::UInt;
	case spirv_cross::SPIRType::Float:
		if (type.vecsize == 1 && type.columns == 1)
			return nap::EUniformValueType::Float;
		else if (type.vecsize == 2 && type.columns == 1)
			return nap::EUniformValueType::Vec2;
		else if (type.vecsize == 3 && type.columns == 1)
			return nap::EUniformValueType::Vec3;
		else if (type.vecsize == 4 && type.columns == 1)
			return nap::EUniformValueType::Vec4;
		else if (type.vecsize == 2 && type.columns == 2)
			return nap::EUniformValueType::Mat2;
		else if (type.vecsize == 3 && type.columns == 3)
			return nap::EUniformValueType::Mat3;
		else if (type.vecsize == 4 && type.columns == 4)
			return nap::EUniformValueType::Mat4;
		else
			return nap::EUniformValueType::Unknown;
	default:
		return nap::EUniformValueType::Unknown;
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


bool addUniformsRecursive(nap::UniformStructDeclaration& parentStruct, spirv_cross::Compiler& compiler, const spirv_cross::SPIRType& type, int parentOffset, const std::string& path, nap::utility::ErrorState& errorState)
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

				std::unique_ptr<nap::UniformStructArrayDeclaration> array_declaration = std::make_unique<nap::UniformStructArrayDeclaration>(name, absoluteOffset, member_size);

				for (int array_index = 0; array_index < num_elements; ++array_index)
				{
					std::string array_path = nap::utility::stringFormat("%s[%d]", full_path.c_str(), array_index);

					std::unique_ptr<nap::UniformStructDeclaration> struct_declaration = std::make_unique<nap::UniformStructDeclaration>(name, absoluteOffset, struct_size);
					if (!addUniformsRecursive(*struct_declaration, compiler, member_type, absoluteOffset, array_path, errorState))
						return false;

					array_declaration->mElements.emplace_back(std::move(struct_declaration));
					absoluteOffset += stride;
				}

				parentStruct.mMembers.emplace_back(std::move(array_declaration));
			}
			else
			{
				nap::EUniformValueType element_type = getUniformValueType(member_type);
				if (!errorState.check(element_type != nap::EUniformValueType::Unknown, "Encountered unknown uniform type"))
					return false;

				std::unique_ptr<nap::UniformValueArrayDeclaration> array_declaration = std::make_unique<nap::UniformValueArrayDeclaration>(name, absoluteOffset, member_size, element_type, num_elements);
				parentStruct.mMembers.emplace_back(std::move(array_declaration));
			}
		}
		else
		{
			if (member_type.basetype == spirv_cross::SPIRType::Struct)
			{
				size_t struct_size = compiler.get_declared_struct_size(member_type);

				std::unique_ptr<nap::UniformStructDeclaration> struct_declaration = std::make_unique<nap::UniformStructDeclaration>(name, absoluteOffset, struct_size);
				if (!addUniformsRecursive(*struct_declaration, compiler, member_type, absoluteOffset, name, errorState))
					return false;

				parentStruct.mMembers.emplace_back(std::move(struct_declaration));
			}
			else
			{
				nap::EUniformValueType value_type = getUniformValueType(member_type);
				if (!errorState.check(value_type != nap::EUniformValueType::Unknown, "Encountered unknown uniform type"))
					return false;

				std::unique_ptr<nap::UniformValueDeclaration> value_declaration = std::make_unique<nap::UniformValueDeclaration>(name, absoluteOffset, member_size, value_type);
				parentStruct.mMembers.emplace_back(std::move(value_declaration));
			}
		}
	}

	return true;
}


bool parseUniforms(spirv_cross::Compiler& compiler, VkShaderStageFlagBits inStage, std::vector<nap::UniformBufferObjectDeclaration>& uboDeclarations, std::vector<nap::SamplerDeclaration>& samplerDeclarations, nap::utility::ErrorState& errorState)
{
	spirv_cross::ShaderResources shader_resources = compiler.get_shader_resources();

	for (const spirv_cross::Resource& resource : shader_resources.uniform_buffers)
	{
		spirv_cross::SPIRType type = compiler.get_type(resource.type_id);

		uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
		size_t struct_size = compiler.get_declared_struct_size(type);

		nap::UniformBufferObjectDeclaration uniform_buffer_object(resource.name, binding, inStage, struct_size);

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

		nap::SamplerDeclaration::EType type;
		if (sampler_type.image.dim == spv::Dim1D)
		{
			type = nap::SamplerDeclaration::EType::Type_1D;
		}
		else if (sampler_type.image.dim == spv::Dim2D)
		{
			type = nap::SamplerDeclaration::EType::Type_2D;
		}
		else if (sampler_type.image.dim == spv::Dim3D)
		{
			type = nap::SamplerDeclaration::EType::Type_3D;
		}
		else
		{
			errorState.fail("Unsupported sampler type encountered");
			return false;
		}

		uint32_t binding = compiler.get_decoration(sampled_image.id, spv::DecorationBinding);
		samplerDeclarations.emplace_back(nap::SamplerDeclaration(sampled_image.name, binding, inStage, type, num_elements));
	}

	return true;
}


namespace nap
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


	Shader::Shader() :
		mRenderer(nullptr)
	{

	}

	Shader::Shader(RenderService& renderService) :
		mRenderer(&renderService.getRenderer())
	{

	}

	// Store path and create display names
	bool Shader::init(utility::ErrorState& errorState)
	{
		if (!errorState.check(!mVertPath.empty(), "Vertex shader path not set"))
			return false;

		if (!errorState.check(!mFragPath.empty(), "Fragment shader path not set"))
			return false;

		// Set display name
		mDisplayName = utility::getFileNameWithoutExtension(mVertPath);

		VkDevice device = mRenderer->getDevice();

		std::vector<unsigned int> vertexShaderData;
		if (!compileShader(device, mVertPath, EShLangVertex, vertexShaderData, errorState))
			return false;

		mVertexModule = createShaderModule(vertexShaderData, device);
		if (!errorState.check(mVertexModule != nullptr, "Unable to load vertex shader module %s", mVertPath.c_str()))
			return false;

		spirv_cross::Compiler vertex_shader_compiler(vertexShaderData.data(), vertexShaderData.size());
		if (!parseUniforms(vertex_shader_compiler, VK_SHADER_STAGE_VERTEX_BIT, mUBODeclarations, mSamplerDeclarations, errorState))
			return false;

		for (const spirv_cross::Resource& stage_input : vertex_shader_compiler.get_shader_resources().stage_inputs)
		{
			spirv_cross::SPIRType input_type = vertex_shader_compiler.get_type(stage_input.type_id);

			VkFormat format = getFormatFromType(input_type);
			if (!errorState.check(format != VK_FORMAT_UNDEFINED, "Encountered unsupported vertex attribute type"))
				return false;

			uint32_t location = vertex_shader_compiler.get_decoration(stage_input.id, spv::DecorationLocation);
			mShaderAttributes[stage_input.name] = std::make_unique<VertexAttributeDeclaration>(stage_input.name, location, format);
		}

		std::vector<unsigned int> fragmentShaderData;
		if (!compileShader(device, mFragPath, EShLangFragment, fragmentShaderData, errorState))
			return false;

		mFragmentModule = createShaderModule(fragmentShaderData, device);
		if (!errorState.check(mFragmentModule != nullptr, "Unable to load fragment shader module %s", mFragPath.c_str()))
			return false;

		spirv_cross::Compiler fragment_shader_compiler(fragmentShaderData.data(), fragmentShaderData.size());
		if (!parseUniforms(fragment_shader_compiler, VK_SHADER_STAGE_FRAGMENT_BIT, mUBODeclarations, mSamplerDeclarations, errorState))
			return false;

		return initLayout(device, errorState);

	}


	bool Shader::initLayout(VkDevice device, nap::utility::ErrorState& errorState)
	{
		std::vector<VkDescriptorSetLayoutBinding> descriptor_set_layouts;
		for (const UniformBufferObjectDeclaration& declaration : mUBODeclarations)
		{
			VkDescriptorSetLayoutBinding uboLayoutBinding = {};
			uboLayoutBinding.binding = declaration.mBinding;
			uboLayoutBinding.descriptorCount = 1;
			uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			uboLayoutBinding.pImmutableSamplers = nullptr;
			uboLayoutBinding.stageFlags = declaration.mStage;

			descriptor_set_layouts.push_back(uboLayoutBinding);
		}

		for (const SamplerDeclaration& declaration : mSamplerDeclarations)
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



}

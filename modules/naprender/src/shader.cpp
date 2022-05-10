/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "shader.h"
#include "material.h"
#include "renderservice.h"
#include "gpubuffer.h"
#include "formatutils.h"

// External Includes
#include <utility/fileutils.h>
#include <nap/logger.h>
#include <fstream>
#include <assert.h>
#include <utility/stringutils.h>
#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_parser.hpp>
#include <glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>
#include <glslang/MachineIndependent/iomapper.h>
#include <nap/core.h>
#include <nap/numeric.h>

RTTI_DEFINE_BASE(nap::BaseShader)
RTTI_DEFINE_BASE(nap::Shader)
RTTI_DEFINE_BASE(nap::ComputeShader)

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ShaderFromFile)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY_FILELINK("VertShader", &nap::ShaderFromFile::mVertPath, nap::rtti::EPropertyMetaData::Required, nap::rtti::EPropertyFileType::VertShader)
	RTTI_PROPERTY_FILELINK("FragShader", &nap::ShaderFromFile::mFragPath, nap::rtti::EPropertyMetaData::Required, nap::rtti::EPropertyFileType::FragShader)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ComputeShaderFromFile)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY_FILELINK("ComputeShader", &nap::ComputeShaderFromFile::mComputePath, nap::rtti::EPropertyMetaData::Required, nap::rtti::EPropertyFileType::ComputeShader)
RTTI_END_CLASS

using namespace std; // Include the standard namespace


//////////////////////////////////////////////////////////////////////////
// Static module members and functions
//////////////////////////////////////////////////////////////////////////

// From https://github.com/KhronosGroup/glslang/blob/master/StandAlone/ResourceLimits.cpp
static const TBuiltInResource defaultResource =
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


/**
 * The IO Resolver is used by glslang to determine the location/binding numbers when autoMapLocations/autoMapBindings is enabled.
 *
 * glslang contains a TDefaultGlslIoResolver which *almost* does what we want:
 * - It assigns locations to all inputs/outputs
 * - It correctly matches outputs with inputs based on the name of the input/output
 * - It does *not* correctly number bindings for our purposes: Vulkan expects the binding for each UBO to be unique across shader stages.
 *   However, the TDefaultGlslIoResolver follows GLSL semantics, where each type of resource is 'namespaced'. That is, there is a unique numbering for each type (UBOs, samplers, etc).
 *   This is incorrect for Vulkan, as this causes the bindings to clash.
 *
 * glslang also contains a TDefaultIoResolver (not exposed through the headers), which follows Vulkan semantics for the bindings (i.e. each resource gets its unique binding), but
 * does not correctly map input/output locations.
 *
 * So, our 'workaround': we make our own class that derives from TDefaultGlslIoResolver. This gives us the correct input/output mapping behavior.
 * Then, we override the resolveBinding function to replace the wrong binding numbering behavior from TDefaultGlslIoResolver with the correct one from TDefaultIoResolver.
 * 
 * Note that there is one important modification from the resolveBinding function from TDefaultGlslIoResolver: that version only assigns binding numbers to uniforms/samplers
 * if they're actually used by the shader. If they're unused, they get a default binding of 0, which can then clash with another uniform that *is* used but gets the first binding
 * slot (0) allocated to it. Our version instead just assigns binding numbers to *everything*, regardless of if they're used or not. This ensures all uniforms/samplers are always uniquely
 * numbered.
 *
 * The net result is that this allows us to compile GLSL shaders, without bindings/locations, to a valid SPIR-V module with correctly auto-numbered bindings/locations.
 * This is useful so that less technical users of nap don't have to manually specify bindings/locations in the shader, but can keep working as if it's GLSL.
 *
 * Note that it's always possible to manually specify the location/bindings in the shaders; in that case those locations/bindings will simply be used.
 */
struct BindingResolver : public glslang::TDefaultGlslIoResolver 
{
	BindingResolver(const glslang::TIntermediate& intermediate) : 
		glslang::TDefaultGlslIoResolver(intermediate) 
	{
	}

	int resolveBinding(EShLanguage /*stage*/, glslang::TVarEntryInfo& ent) override 
	{
		const glslang::TType& type = ent.symbol->getType();
		const int set = getLayoutSet(type);
		// On OpenGL arrays of opaque types take a separate binding for each element
		int numBindings = intermediate.getSpv().openGl != 0 && type.isSizedArray() ? type.getCumulativeArraySize() : 1;
		glslang::TResourceType resource = getResourceType(type);
		if (resource < glslang::EResCount) {
			if (type.getQualifier().hasBinding()) {
				return ent.newBinding = reserveSlot(
					set, getBaseBinding(resource, set) + type.getQualifier().layoutBinding, numBindings);
			}
			else if (doAutoBindingMapping()) {
				// find free slot, the caller did make sure it passes all vars with binding
				// first and now all are passed that do not have a binding and needs one
				return ent.newBinding = getFreeSlot(set, getBaseBinding(resource, set), numBindings);
			}
		}
		return ent.newBinding = -1;
	}
};


static VkShaderModule createShaderModule(const std::vector<nap::uint32>& code, VkDevice device)
{
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size() * sizeof(nap::uint32);
	createInfo.pCode = code.data();

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
		return VK_NULL_HANDLE;
	return shaderModule;
}


static std::unique_ptr<glslang::TShader> compileShader(VkDevice device, nap::uint32 vulkanVersion, const char* shaderCode, int shaderSize, const std::string& shaderName, EShLanguage stage, nap::utility::ErrorState& errorState)
{
	const char* sources[] = { shaderCode };
	int source_sizes[] = { shaderSize };
	const char* file_names[] = { shaderName.data() };

	std::unique_ptr<glslang::TShader> shader = std::make_unique<glslang::TShader>(stage);
	shader->setStringsWithLengthsAndNames(sources, source_sizes, file_names, 1);
	shader->setAutoMapBindings(true);
	shader->setAutoMapLocations(true);

	glslang::EShTargetClientVersion target_client_version;
	glslang::EShTargetLanguageVersion target_language_version;

	// The client/language version must match with the Vulkan device's version
	if (vulkanVersion >= VK_API_VERSION_1_1)
	{
		// For version 1.1 or higher, use Vulkan 1.1 with SPV 1.3
		target_client_version = glslang::EShTargetVulkan_1_1;
		target_language_version = glslang::EShTargetSpv_1_3;
	}
	else
	{
		// For version 1.0, use Vulkan 1.0 with SPV 1.0
		target_client_version = glslang::EShTargetVulkan_1_0;
		target_language_version = glslang::EShTargetSpv_1_0;
	}

	shader->setEnvClient(glslang::EShClientVulkan, target_client_version);
	shader->setEnvTarget(glslang::EShTargetSpv, target_language_version);

	EShMessages messages = EShMsgDefault;
	messages = (EShMessages)(messages | EShMsgSpvRules);
	messages = (EShMessages)(messages | EShMsgVulkanRules);

	// Version number taken from shaderc sources. 110 means "Desktop OpenGL", 100 means "OpenGL ES"
	int default_version = 110;
	bool result = shader->parse(&defaultResource, default_version, false, messages);
	if (!result)
	{
		errorState.fail("%s", shader->getInfoLog());
		return nullptr;
	}

	return shader;
}


static bool compileProgram(VkDevice device, nap::uint32 vulkanVersion, const char* vertSource, int vertSize, const char* fragSource, int fragSize, const std::string& shaderName, std::vector<nap::uint32>& vertexSPIRV, std::vector<unsigned int>& fragmentSPIRV, nap::utility::ErrorState& errorState)
{
	std::unique_ptr<glslang::TShader> vertex_shader = compileShader(device, vulkanVersion, vertSource, vertSize, shaderName, EShLangVertex, errorState);
	if (vertex_shader == nullptr)
	{
		errorState.fail("Unable to compile vertex shader");
		return false;
	}

	std::unique_ptr<glslang::TShader> fragment_shader = compileShader(device, vulkanVersion, fragSource, fragSize, shaderName, EShLangFragment, errorState);
	if (fragment_shader == nullptr)
	{
		errorState.fail("Unable to compile fragment shader");
		return false;
	}

	EShMessages messages = EShMsgDefault;
	messages = (EShMessages)(messages | EShMsgSpvRules);
	messages = (EShMessages)(messages | EShMsgVulkanRules);

	// We need to create a program containing both the vertex & fragment shaders so that bindings are correctly numbered and input/outputs are correctly mapped between the stages.
	glslang::TProgram program;
	program.addShader(vertex_shader.get());
	program.addShader(fragment_shader.get());

	// Link the program first. This merges potentially multiple compilation units within a single stage into a single intermediate representation.
	// Note that this usually doesn't do anything (1 compilation unit per stage is the most common case), but it is neccessary in order to execute the next step.
	// In addition, if this isn't called, glslang will not correctly resolve built-in variables such as gl_PerVertex, which will result in invalid SPIR-V being generated
	if (!errorState.check(program.link(messages), "Failed to link shader program: %s", program.getInfoLog()))
		return false;

	// We need to create our own IO mapper/resolver for automatic numbering of locations/bindings. See comment above BindingResolver at the top of this file.
	BindingResolver io_resolver(*program.getIntermediate(EShLangVertex));
	glslang::TGlslIoMapper io_mapper;

	// Call mapIO to automatically number the bindings and correctly map input/output locations in both stages
	if (!errorState.check(program.mapIO(&io_resolver, &io_mapper), "Failed to map input/outputs: %s", program.getInfoLog()))
		return false;

	glslang::SpvOptions spv_options;
	spv_options.generateDebugInfo = false;
	spv_options.disableOptimizer = false;
	spv_options.optimizeSize = false;
	spv_options.disassemble = false;
	spv_options.validate = true;

	// Generate the SPIR-V for the vertex shader
	{
		spv::SpvBuildLogger logger;
		glslang::GlslangToSpv(*program.getIntermediate(EShLangVertex), vertexSPIRV, &logger, &spv_options);

		if (!errorState.check(!vertexSPIRV.empty(), "Failed to compile vertex shader: %s", logger.getAllMessages().c_str()))
			return false;
	}

	// Generate the SPIR-V for the fragment shader
	{
		spv::SpvBuildLogger logger;
		glslang::GlslangToSpv(*program.getIntermediate(EShLangFragment), fragmentSPIRV, &logger, &spv_options);

		if (!errorState.check(!fragmentSPIRV.empty(), "Failed to compile fragment shader: %s", logger.getAllMessages().c_str()))
			return false;
	}

	return true;
}


static bool compileComputeProgram(VkDevice device, nap::uint32 vulkanVersion, const char* compSource, int compSize, const std::string& shaderName, std::vector<nap::uint32>& computeSPIRV, nap::utility::ErrorState& errorState)
{
	std::unique_ptr<glslang::TShader> compute_shader = compileShader(device, vulkanVersion, compSource, compSize, shaderName, EShLangCompute, errorState);
	if (compute_shader == nullptr)
	{
		errorState.fail("Unable to compile compute shader");
		return false;
	}

	EShMessages messages = EShMsgDefault;
	messages = (EShMessages)(messages | EShMsgSpvRules);
	messages = (EShMessages)(messages | EShMsgVulkanRules);

	// We need to create a program containing both the vertex & fragment shaders so that bindings are correctly numbered and input/outputs are correctly mapped between the stages.
	glslang::TProgram program;
	program.addShader(compute_shader.get());

	// Link the program first. This merges potentially multiple compilation units within a single stage into a single intermediate representation.
	// Note that this usually doesn't do anything (1 compilation unit per stage is the most common case), but it is neccessary in order to execute the next step.
	// In addition, if this isn't called, glslang will not correctly resolve built-in variables such as gl_PerVertex, which will result in invalid SPIR-V being generated
	if (!errorState.check(program.link(messages), "Failed to link shader program: %s", program.getInfoLog()))
		return false;

	// We need to create our own IO mapper/resolver for automatic numbering of locations/bindings. See comment above BindingResolver at the top of this file.
	BindingResolver io_resolver(*program.getIntermediate(EShLangCompute));
	glslang::TGlslIoMapper io_mapper;

	// Call mapIO to automatically number the bindings and correctly map input/output locations in both stages
	if (!errorState.check(program.mapIO(&io_resolver, &io_mapper), "Failed to map input/outputs: %s", program.getInfoLog()))
		return false;

	glslang::SpvOptions spv_options;
	spv_options.generateDebugInfo = false;
	spv_options.disableOptimizer = false;
	spv_options.optimizeSize = false;
	spv_options.disassemble = false;
	spv_options.validate = true;

	// Generate the SPIR-V for the compute shader
	{
		spv::SpvBuildLogger logger;
		glslang::GlslangToSpv(*program.getIntermediate(EShLangCompute), computeSPIRV, &logger, &spv_options);

		if (!errorState.check(!computeSPIRV.empty(), "Failed to compile compute shader: %s", logger.getAllMessages().c_str()))
			return false;
	}

	return true;
}


static nap::EShaderVariableValueType getShaderVariableValueType(spirv_cross::SPIRType type)
{
	switch (type.basetype)
	{
	case spirv_cross::SPIRType::Int:
		return nap::EShaderVariableValueType::Int;
	case spirv_cross::SPIRType::UInt:
		return nap::EShaderVariableValueType::UInt;
	case spirv_cross::SPIRType::Float:
		if (type.vecsize == 1 && type.columns == 1)
			return nap::EShaderVariableValueType::Float;
		else if (type.vecsize == 2 && type.columns == 1)
			return nap::EShaderVariableValueType::Vec2;
		else if (type.vecsize == 3 && type.columns == 1)
			return nap::EShaderVariableValueType::Vec3;
		else if (type.vecsize == 4 && type.columns == 1)
			return nap::EShaderVariableValueType::Vec4;
		else if (type.vecsize == 2 && type.columns == 2)
			return nap::EShaderVariableValueType::Mat2;
		else if (type.vecsize == 3 && type.columns == 3)
			return nap::EShaderVariableValueType::Mat3;
		else if (type.vecsize == 4 && type.columns == 4)
			return nap::EShaderVariableValueType::Mat4;
		else
			return nap::EShaderVariableValueType::Unknown;
	default:
		return nap::EShaderVariableValueType::Unknown;
	}
}


static VkFormat getFormatFromType(spirv_cross::SPIRType type)
{
	switch (type.basetype)
	{
	case spirv_cross::SPIRType::Int:
		return VK_FORMAT_R32_SINT;
	case spirv_cross::SPIRType::UInt:
		return VK_FORMAT_R32_UINT;
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


static bool addShaderVariablesRecursive(nap::ShaderVariableStructDeclaration& parentStruct, spirv_cross::Compiler& compiler, const spirv_cross::SPIRType& type, int parentOffset, const std::string& path, nap::EDescriptorType descriptorType, nap::utility::ErrorState& errorState)
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

				if (descriptorType == nap::EDescriptorType::Storage)
				{
					// ShaderVariableStructBufferDeclaration is a special shader variable declaration type exclusive to storage buffer descriptor types.
					// They are not built recursively as none of the values have to be assigned or accessed individually, but rather the buffer as a whole.
					// Therefore, we only store the declaration of a single struct item as a ShaderVariableStructDeclaration, and set it to the element member
					// of the ShaderVariableStructBufferDeclaration along with the element stride and count. All that matters is the size of the struct element,
					// which is already resolved by SPIR-V. Therefore, we do not have to traverse the struct recursively here.

					std::unique_ptr<nap::ShaderVariableStructBufferDeclaration> buffer_declaration = std::make_unique<nap::ShaderVariableStructBufferDeclaration>(name, absoluteOffset, member_size, stride, num_elements);
					std::unique_ptr<nap::ShaderVariableStructDeclaration> struct_declaration = std::make_unique<nap::ShaderVariableStructDeclaration>(name, parentStruct.mDescriptorType, absoluteOffset, struct_size);
					buffer_declaration->mElement = std::move(struct_declaration);

					parentStruct.mMembers.emplace_back(std::move(buffer_declaration));
				}
				else if (descriptorType == nap::EDescriptorType::Uniform)
				{
					std::unique_ptr<nap::ShaderVariableStructArrayDeclaration> array_declaration = std::make_unique<nap::ShaderVariableStructArrayDeclaration>(name, absoluteOffset, member_size);

					for (int array_index = 0; array_index < num_elements; ++array_index)
					{
						std::string array_path = nap::utility::stringFormat("%s[%d]", full_path.c_str(), array_index);

						std::unique_ptr<nap::ShaderVariableStructDeclaration> struct_declaration = std::make_unique<nap::ShaderVariableStructDeclaration>(name, parentStruct.mDescriptorType, absoluteOffset, struct_size);
						if (!addShaderVariablesRecursive(*struct_declaration, compiler, member_type, absoluteOffset, array_path, descriptorType, errorState))
							return false;

						array_declaration->mElements.emplace_back(std::move(struct_declaration));
						absoluteOffset += stride;
					}
					parentStruct.mMembers.emplace_back(std::move(array_declaration));
				} 
			}
			else
			{
				size_t stride = compiler.type_struct_member_array_stride(type, index);

				nap::EShaderVariableValueType element_type = getShaderVariableValueType(member_type);
				if (!errorState.check(element_type != nap::EShaderVariableValueType::Unknown, "Encountered unknown uniform type"))
					return false;

				std::unique_ptr<nap::ShaderVariableValueArrayDeclaration> array_declaration = std::make_unique<nap::ShaderVariableValueArrayDeclaration>(name, absoluteOffset, member_size, stride, element_type, num_elements);
				parentStruct.mMembers.emplace_back(std::move(array_declaration));
			}
		}
		else
		{
			if (member_type.basetype == spirv_cross::SPIRType::Struct)
			{
				size_t struct_size = compiler.get_declared_struct_size(member_type);

				std::unique_ptr<nap::ShaderVariableStructDeclaration> struct_declaration = std::make_unique<nap::ShaderVariableStructDeclaration>(name, parentStruct.mDescriptorType, absoluteOffset, struct_size);
				if (!addShaderVariablesRecursive(*struct_declaration, compiler, member_type, absoluteOffset, name, descriptorType, errorState))
					return false;

				parentStruct.mMembers.emplace_back(std::move(struct_declaration));
			}
			else
			{
				nap::EShaderVariableValueType value_type = getShaderVariableValueType(member_type);
				if (!errorState.check(value_type != nap::EShaderVariableValueType::Unknown, "Encountered unknown uniform type"))
					return false;

				std::unique_ptr<nap::ShaderVariableValueDeclaration> value_declaration = std::make_unique<nap::ShaderVariableValueDeclaration>(name, absoluteOffset, member_size, value_type);
				parentStruct.mMembers.emplace_back(std::move(value_declaration));
			}
		}
	}

	return true;
}


static bool parseShaderVariables(spirv_cross::Compiler& compiler, VkShaderStageFlagBits inStage, nap::BufferObjectDeclarationList& uboDeclarations, nap::BufferObjectDeclarationList& ssboDeclarations, nap::SamplerDeclarations& samplerDeclarations, nap::utility::ErrorState& errorState)
{
	spirv_cross::ShaderResources shader_resources = compiler.get_shader_resources();

	// Uniform buffers
	for (const spirv_cross::Resource& resource : shader_resources.uniform_buffers)
	{
		spirv_cross::SPIRType type = compiler.get_type(resource.type_id);

		nap::uint32 binding = compiler.get_decoration(resource.id, spv::DecorationBinding);

		size_t struct_size = compiler.get_declared_struct_size(type);
		nap::BufferObjectDeclaration uniform_buffer_object(resource.name, binding, inStage, nap::EDescriptorType::Uniform, struct_size);

		if (!addShaderVariablesRecursive(uniform_buffer_object, compiler, type, 0, resource.name, nap::EDescriptorType::Uniform, errorState))
			return false;

		uboDeclarations.emplace_back(std::move(uniform_buffer_object));
	}

	// Storage buffers
	for (const spirv_cross::Resource& resource : shader_resources.storage_buffers)
	{
		spirv_cross::SPIRType type = compiler.get_type(resource.type_id);

		nap::uint32 binding = compiler.get_decoration(resource.id, spv::DecorationBinding);

		size_t struct_size = compiler.get_declared_struct_size(type);
		nap::BufferObjectDeclaration storage_buffer_object(resource.name, binding, inStage, nap::EDescriptorType::Storage, struct_size);

		if (!addShaderVariablesRecursive(storage_buffer_object, compiler, type, 0, resource.name, nap::EDescriptorType::Storage, errorState))
			return false;

		ssboDeclarations.emplace_back(std::move(storage_buffer_object));
	}

	// Samplers
	for (const spirv_cross::Resource& sampled_image : shader_resources.sampled_images)
	{
		spirv_cross::SPIRType sampler_type = compiler.get_type(sampled_image.type_id);

		if (!errorState.check(sampler_type.array.size() <= 1, "Multidimensional arrays are not supported"))
			return false;

		auto existing_sampler = std::find_if(samplerDeclarations.begin(), samplerDeclarations.end(), [&sampled_image](const nap::SamplerDeclaration& declaration) { return declaration.mName == sampled_image.name; });
		if (!errorState.check(existing_sampler == samplerDeclarations.end(), "Encountered a sampler '%s' which has previously been declared for this shader; duplicate samplers are not supported.", sampled_image.name.c_str()))
			return false;

		bool is_array = !sampler_type.array.empty();
		int num_elements = is_array ? sampler_type.array[0] : 1;

		nap::SamplerDeclaration::EType type;
		switch(sampler_type.image.dim)
		{
			case spv::Dim1D:
				type = nap::SamplerDeclaration::EType::Type_1D;
				break;
			case spv::Dim2D:
				type = nap::SamplerDeclaration::EType::Type_2D;
				break;
			case spv::Dim3D:
				type = nap::SamplerDeclaration::EType::Type_3D;
				break;
			default:
				errorState.fail("Unsupported sampler type encountered");
				return false;
		}

		nap::uint32 binding = compiler.get_decoration(sampled_image.id, spv::DecorationBinding);
		samplerDeclarations.emplace_back(nap::SamplerDeclaration(sampled_image.name, binding, inStage, type, num_elements));
	}

	return true;
}


namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// Base Shader
	//////////////////////////////////////////////////////////////////////////

	BaseShader::BaseShader(Core& core) : mRenderService(core.getService<RenderService>())
	{ }

	BaseShader::~BaseShader()
	{
		// Remove all previously made requests and queue buffers for destruction.
		// If the service is not running, all objects are destroyed immediately.
		// Otherwise they are destroyed when they are guaranteed not to be in use by the GPU.
		mRenderService->queueVulkanObjectDestructor([descriptorSetLayout = mDescriptorSetLayout](RenderService& renderService)
		{
			if (descriptorSetLayout != VK_NULL_HANDLE)
				vkDestroyDescriptorSetLayout(renderService.getDevice(), descriptorSetLayout, nullptr);
		});
	}


	bool BaseShader::initLayout(VkDevice device, nap::utility::ErrorState& errorState)
	{
		std::vector<VkDescriptorSetLayoutBinding> descriptor_set_layouts;
		for (const BufferObjectDeclaration& declaration : mUBODeclarations)
		{
			VkDescriptorSetLayoutBinding uboLayoutBinding = {};
			uboLayoutBinding.binding = declaration.mBinding;
			uboLayoutBinding.descriptorCount = 1;
			uboLayoutBinding.pImmutableSamplers = nullptr;
			uboLayoutBinding.stageFlags = declaration.mStage;
			uboLayoutBinding.descriptorType = getVulkanDescriptorType(declaration.mDescriptorType);

			descriptor_set_layouts.push_back(uboLayoutBinding);
		}

		for (const BufferObjectDeclaration& declaration : mSSBODeclarations)
		{
			VkDescriptorSetLayoutBinding ssboLayoutBinding = {};
			ssboLayoutBinding.binding = declaration.mBinding;
			ssboLayoutBinding.descriptorCount = 1;
			ssboLayoutBinding.pImmutableSamplers = nullptr;
			ssboLayoutBinding.stageFlags = declaration.mStage;
			ssboLayoutBinding.descriptorType = getVulkanDescriptorType(declaration.mDescriptorType);

			descriptor_set_layouts.push_back(ssboLayoutBinding);
		}

		for (const SamplerDeclaration& declaration : mSamplerDeclarations)
		{
			VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
			samplerLayoutBinding.binding = declaration.mBinding;
			samplerLayoutBinding.descriptorCount = declaration.mNumArrayElements;
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


	bool BaseShader::verifyShaderVariableDeclarations(utility::ErrorState& errorState)
	{
		// We must check if SSBO declarations contain more than a single shader variable and exit early if this is the case.
		// The reason for this is that we want to associate a shader buffer resource binding point with single shader storage
		// buffer (VkBuffer), this is a typical use case for storage buffers and simplifies overall resource management. At the
		// same time we use regular shader variable declarations, that assume a list of member variables, to generate buffer bindings.
		for (const auto& declaration : mSSBODeclarations)
		{
			if (!errorState.check(declaration.mMembers.size() <= 1, utility::stringFormat(
				"SSBO '%s' contains more than 1 shader variable, which is currently not supported. Consider using multiple SSBO's or a struct array.", declaration.mName.c_str())))
				return false;
		}
		return true;
	}


	//////////////////////////////////////////////////////////////////////////
	// Shader
	//////////////////////////////////////////////////////////////////////////

	Shader::Shader(Core& core) : BaseShader(core)
	{ }


	Shader::~Shader()
	{
		// Remove all previously made requests and queue buffers for destruction.
		// If the service is not running, all objects are destroyed immediately.
		// Otherwise they are destroyed when they are guaranteed not to be in use by the GPU.
		mRenderService->queueVulkanObjectDestructor([vertexModule = mVertexModule, fragmentModule = mFragmentModule](RenderService& renderService)
		{
			if (vertexModule != VK_NULL_HANDLE)
				vkDestroyShaderModule(renderService.getDevice(), vertexModule, nullptr);

			if (fragmentModule != VK_NULL_HANDLE)
				vkDestroyShaderModule(renderService.getDevice(), fragmentModule, nullptr);
		});
	}


	bool Shader::load(const std::string& displayName, const char* vertShader, int vertSize, const char* fragShader, int fragSize, utility::ErrorState& errorState)
	{
		// Set display name
		assert(mRenderService->isInitialized());
		mDisplayName = displayName;

		VkDevice device = mRenderService->getDevice();
		nap::uint32 vulkan_version = mRenderService->getVulkanVersion();

		// Compile vertex & fragment shader into program and get resulting SPIR-V
		std::vector<nap::uint32> vertex_shader_spirv;
		std::vector<nap::uint32> fragment_shader_spirv;

		// Compile both vert and frag into single shader pipeline program
		if (!compileProgram(device, vulkan_version, vertShader, vertSize, fragShader, fragSize, mDisplayName, vertex_shader_spirv, fragment_shader_spirv, errorState))
			return false;

		// Create vertex shader module
		mVertexModule = createShaderModule(vertex_shader_spirv, device);
		if (!errorState.check(mVertexModule != VK_NULL_HANDLE, "Unable to load vertex shader module"))
			return false;

		// Create fragment shader module
		mFragmentModule = createShaderModule(fragment_shader_spirv, device);
		if (!errorState.check(mFragmentModule != VK_NULL_HANDLE, "Unable to load fragment shader module"))
			return false;

		// Extract vertex shader uniforms & inputs
		spirv_cross::Compiler vertex_shader_compiler(vertex_shader_spirv.data(), vertex_shader_spirv.size());
		if (!parseShaderVariables(vertex_shader_compiler, VK_SHADER_STAGE_VERTEX_BIT, mUBODeclarations, mSSBODeclarations, mSamplerDeclarations, errorState))
			return false;

		for (const spirv_cross::Resource& stage_input : vertex_shader_compiler.get_shader_resources().stage_inputs)
		{
			spirv_cross::SPIRType input_type = vertex_shader_compiler.get_type(stage_input.type_id);
			VkFormat format = getFormatFromType(input_type);
			if (!errorState.check(format != VK_FORMAT_UNDEFINED, "Encountered unsupported vertex attribute type"))
				return false;

			nap::uint32 location = vertex_shader_compiler.get_decoration(stage_input.id, spv::DecorationLocation);
			int element_size_bytes = (input_type.width/8) * input_type.vecsize * input_type.columns;
			mShaderAttributes[stage_input.name] = std::make_unique<VertexAttributeDeclaration>(stage_input.name, location, element_size_bytes, format);
		}

		// Extract fragment shader uniforms
		spirv_cross::Compiler fragment_shader_compiler(fragment_shader_spirv.data(), fragment_shader_spirv.size());
		if (!parseShaderVariables(fragment_shader_compiler, VK_SHADER_STAGE_FRAGMENT_BIT, mUBODeclarations, mSSBODeclarations, mSamplerDeclarations, errorState))
			return false;

		// Verify the shader variable declarations
		if (!verifyShaderVariableDeclarations(errorState))
			return false;

		return initLayout(device, errorState);
	}


	bool Shader::loadDefault(const std::string& displayName, utility::ErrorState& errorState)
	{
		std::string relative_path = utility::joinPath({ "shaders", utility::appendFileExtension(displayName, "vert") });
		const std::string vertex_shader_path = mRenderService->getModule().findAsset(relative_path);
		if (!errorState.check(!vertex_shader_path.empty(), "%s: Unable to find %s vertex shader %s", mRenderService->getModule().getName().c_str(), displayName.c_str(), vertex_shader_path.c_str()))
			return false;

		relative_path = utility::joinPath({ "shaders", utility::appendFileExtension(displayName, "frag") });
		const std::string fragment_shader_path = mRenderService->getModule().findAsset(relative_path);
		if (!errorState.check(!vertex_shader_path.empty(), "%s: Unable to find %s fragment shader %s", mRenderService->getModule().getName().c_str(), displayName.c_str(), fragment_shader_path.c_str()))
			return false;

		// Read vert shader file
		std::string vert_source;
		if (!errorState.check(utility::readFileToString(vertex_shader_path, vert_source, errorState), "Unable to read %s vertex shader file", displayName.c_str()))
			return false;

		// Read frag shader file
		std::string frag_source;
		if (!errorState.check(utility::readFileToString(fragment_shader_path, frag_source, errorState), "Unable to read %s fragment shader file", displayName.c_str()))
			return false;

		// Compile shader
		return this->load(displayName, vert_source.data(), vert_source.size(), frag_source.data(), frag_source.size(), errorState);
	}


	//////////////////////////////////////////////////////////////////////////
	// Compute Shader
	//////////////////////////////////////////////////////////////////////////

	ComputeShader::ComputeShader(Core& core) : BaseShader(core)
	{ }


	ComputeShader::~ComputeShader()
	{
		// Remove all previously made requests and queue buffers for destruction.
		// If the service is not running, all objects are destroyed immediately.
		// Otherwise they are destroyed when they are guaranteed not to be in use by the GPU.
		mRenderService->queueVulkanObjectDestructor([compModule = mComputeModule](RenderService& renderService)
		{
			if (compModule != VK_NULL_HANDLE)
				vkDestroyShaderModule(renderService.getDevice(), compModule, nullptr);
		});
	}


	bool ComputeShader::load(const std::string& displayName, const char* compShader, int compSize, utility::ErrorState& errorState)
	{
		// Set display name
		assert(mRenderService->isInitialized());
		mDisplayName = displayName;

		VkDevice device = mRenderService->getDevice();
		nap::uint32 vulkan_version = mRenderService->getVulkanVersion();

		// Compile vertex & fragment shader into program and get resulting SPIR-V
		std::vector<uint32> comp_shader_spirv;

		// Compile both vert and frag into single shader pipeline program
		if (!compileComputeProgram(device, vulkan_version, compShader, compSize, mDisplayName, comp_shader_spirv, errorState))
			return false;

		// Create compute shader module
		mComputeModule = createShaderModule(comp_shader_spirv, device);
		if (!errorState.check(mComputeModule != VK_NULL_HANDLE, "Unable to load compute shader module"))
			return false;

		// Extract shader variables
		spirv_cross::Compiler comp_shader_compiler(comp_shader_spirv.data(), comp_shader_spirv.size());
		if (!parseShaderVariables(comp_shader_compiler, VK_SHADER_STAGE_COMPUTE_BIT, mUBODeclarations, mSSBODeclarations, mSamplerDeclarations, errorState))
			return false;

		// Cache workgroup size specialization constants
		std::array<spirv_cross::SpecializationConstant, 3> spec_constants;
		comp_shader_compiler.get_work_group_size_specialization_constants(spec_constants[0], spec_constants[1], spec_constants[2]);

		// Set to invalid (-1) by default
		mWorkGroupSizeConstantIds.resize(3, -1);

		// Search for workgroup specialization constants
		for (uint i = 0; i < spec_constants.size(); i++)
		{
			if (spec_constants[i].id != spirv_cross::ID(0))
			{
				// Overwrite workgroup size with quaried maximum supported workgroup size
				mWorkGroupSizeConstantIds[i] = spec_constants[i].constant_id;
				mWorkGroupSize[i] = mRenderService->getPhysicalDeviceProperties().limits.maxComputeWorkGroupSize[i];
			}
			else
			{
				// Use reflection to store constant defined in shader source
				mWorkGroupSize[i] = comp_shader_compiler.get_execution_mode_argument(spv::ExecutionMode::ExecutionModeLocalSize, i);
			}
		}

		return initLayout(device, errorState);
	}


	//////////////////////////////////////////////////////////////////////////
	// Shader From File
	//////////////////////////////////////////////////////////////////////////

	ShaderFromFile::ShaderFromFile(Core& core) : Shader(core)
	{ }


	// Store path and create display names
	bool ShaderFromFile::init(utility::ErrorState& errorState)
	{
		// Ensure vertex shader exists
		if (!errorState.check(!mVertPath.empty(), "Vertex shader path not set"))
			return false;

		// Ensure fragment shader exists
		if (!errorState.check(!mFragPath.empty(), "Fragment shader path not set"))
			return false;

		// Read vert shader file
		std::string vert_source;
		if (!errorState.check(utility::readFileToString(mVertPath, vert_source, errorState), "Unable to read shader file %s", mVertPath.c_str()))
			return false;

		// Read frag shader file
		std::string frag_source;
		if (!errorState.check(utility::readFileToString(mFragPath, frag_source, errorState), "Unable to read shader file %s", mFragPath.c_str()))
			return false;
		
		// Compile shader
		std::string shader_name = utility::getFileNameWithoutExtension(mVertPath);
		return this->load(shader_name, vert_source.data(), vert_source.size(), frag_source.data(), frag_source.size(), errorState);
	}


	//////////////////////////////////////////////////////////////////////////
	// Compute Shader From File
	//////////////////////////////////////////////////////////////////////////

	ComputeShaderFromFile::ComputeShaderFromFile(Core& core) : ComputeShader(core) { }


	// Store path and create display names
	bool ComputeShaderFromFile::init(utility::ErrorState& errorState)
	{
		// Ensure compute shader exists
		if (!errorState.check(!mComputePath.empty(), "Compute shader path not set"))
			return false;

		// Read comp shader file
		std::string comp_source;
		if (!errorState.check(utility::readFileToString(mComputePath, comp_source, errorState), "Unable to read shader file %s", mComputePath.c_str()))
			return false;

		// Compile shader
		std::string shader_name = utility::getFileNameWithoutExtension(mComputePath);
		return this->load(shader_name, comp_source.data(), comp_source.size(), errorState);
	}
}

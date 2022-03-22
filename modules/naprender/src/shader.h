/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "rendertarget.h"
#include "vertexattributedeclaration.h"
#include "samplerdeclaration.h"
#include "shadervariabledeclarations.h"
#include "uniform.h"

// External Includes
#include <utility/dllexport.h>
#include <nap/resource.h>
#include <rtti/factory.h>

namespace nap
{
	// Forward Declares
	class RenderService;
	class Core;

	/**
	 * Base class of all shaders
	 */
	class NAPAPI BaseShader : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		BaseShader(Core& core);
		virtual ~BaseShader();

		/**
		 * @return all uniform shader attributes
		 */
		const SamplerDeclarations& getSamplerDeclarations() const { return mSamplerDeclarations; }

		/**
		 * @return all UniformBufferObject declarations.
		 */
		const std::vector<BufferObjectDeclaration>& getUBODeclarations() const { return mUBODeclarations; }

		/**
		 * @return all Shader Storage Buffer Object declarations.
		 */
		const std::vector<BufferObjectDeclaration>& getSSBODeclarations() const { return mSSBODeclarations; }

		/**
		 * @return shader display name
		 */
		const std::string& getDisplayName() const { return mDisplayName; }

		/**
		* @return Vulkan descriptorSetLayout.
		*/
		VkDescriptorSetLayout getDescriptorSetLayout() const { return mDescriptorSetLayout; }

	protected:
		RenderService*									mRenderService = nullptr;				///< Handle to render engine
		std::string										mDisplayName;							///< Filename of shader used as display name
		BufferObjectDeclarationList						mUBODeclarations;						///< All uniform buffer object declarations
		BufferObjectDeclarationList						mSSBODeclarations;						///< All storage buffer object declarations
		SamplerDeclarations								mSamplerDeclarations;					///< All sampler declarations
		VkDescriptorSetLayout							mDescriptorSetLayout = VK_NULL_HANDLE;	///< Descriptor set layout

		bool initLayout(VkDevice device, nap::utility::ErrorState& errorState);
	};


	/**
	 * Cross-Compiles GLSL vertex and fragment shader code to SPIR-V and creates a Vulkan shader module.
	 * All uniforms, samplers and attributes are extracted.
	 * Make sure to call init() on initialization of a derived shader class.
	 * A nap::Material links to a BaseShader. The shader is compiled on initialization.
	 * Use a nap::Material or nap::MaterialInstance to set / override uniforms and samplers.
	 */
	class NAPAPI Shader : public BaseShader
	{
	public:
		Shader(Core& core);
		~Shader();

		/**
		* @return all vertex shader attribute declarations.
		*/
		const VertexAttributeDeclarations& getAttributes() const { return mShaderAttributes; }

		/**
		 * @return Vulkan vertex module.
		 */
		VkShaderModule getVertexModule() const { return mVertexModule; }

		/**
		 * @return Vulkan fragment module.
		 */
		VkShaderModule getFragmentModule() const { return mFragmentModule; }

	protected:
		/**
		 * Compiles the GLSL shader code, creates the shader module and parses all the uniforms and samplers.
		 * Call this in a derived class on initialization.
		 * @param displayName the name of the shader.
		 * @param vertShader the vertex shader GLSL code.
		 * @param vertSize total number of characters in vertShader.
		 * @param fragShader the fragment shader GLSL code.
		 * @param fragSize total number of characters in fragShader.
		 * @param errorState contains the error if initialization fails.
		 * @return if initialization succeeded.
		 */
		 bool load(const std::string& displayName, const char* vertShader, int vertSize, const char* fragShader, int fragSize, utility::ErrorState& errorState);
		 
		 /**
		  * Loads a NAP default shader from disk.
		  * Compiles the GLSL shader code, creates the shader module and parses all the uniforms and samplers.
		  * Call this in a derived class on initialization.
		  * @param displayName the name of the NAP default shader.
		  * @param errorState contains the error if initialization fails.
		  * @return if initialization succeeded.
		  */
		 bool loadDefault(const std::string& displayName, utility::ErrorState& errorState);

	private:
		VertexAttributeDeclarations						mShaderAttributes;						///< Shader program vertex attribute inputs
		VkShaderModule									mVertexModule = VK_NULL_HANDLE;			///< Loaded vertex module
		VkShaderModule									mFragmentModule = VK_NULL_HANDLE;		///< Loaded fragment module
	};


	/**
	 * Cross-Compiles GLSL vertex and fragment shader code to SPIR-V and creates a Vulkan shader module.
	 * All uniforms, samplers and attributes are extracted.
	 * Make sure to call init() on initialization of a derived shader class.
	 * A nap::Material links to a BaseShader. The shader is compiled on initialization.
	 * Use a nap::Material or nap::MaterialInstance to set / override uniforms and samplers.
	 */
	class NAPAPI ComputeShader : public BaseShader
	{
		RTTI_ENABLE(Resource)
	public:
		ComputeShader(Core& core);
		~ComputeShader();

		/**
		 * @return Vulkan vertex module.
		 */
		VkShaderModule getComputeModule() const { return mComputeModule; }

		/**
		 * @return local work group size
		 */
		glm::u32vec3 getWorkGroupSize() const { return mWorkGroupSize; }

		/**
		 * Workgroup specialization constant IDs. 
		 * When a workgroup size specialization constant is detected, NAP automatically overwrites it with the
		 * maximum group size of the device on pipeline creation. Entries with the value -1 have no associated
		 * specialization constant defined in the compute shader.
		 * @return a vector of work group size specialization constant IDs
		 */
		const std::vector<int>& getWorkGroupSizeConstantIds() const { return mWorkGroupSizeConstantIds; }

	protected:
		/**
		 * Compiles the GLSL shader code, creates the shader module and parses all the uniforms and samplers.
		 * Call this in a derived class on initialization.
		 * @param displayName the name of the shader
		 * @param compShader the fragment shader GLSL code.
		 * @param compSize total number of characters in compShader.
		 * @param errorState contains the error if initialization fails.
		 * @return if initialization succeeded.
		 */
		virtual bool load(const std::string& displayName, const char* compShader, int compSize, utility::ErrorState& errorState);

	private:
		glm::u32vec3									mWorkGroupSize;
		VkShaderModule									mComputeModule = VK_NULL_HANDLE;		///< Loaded compute module

		std::vector<int>								mWorkGroupSizeConstantIds;				///< Workgroup size specialization constant IDs
	};


	/**
	 * Loads a GLSL shader from disk using the provided vertex and fragment shader paths.
	 * The shader cross compiles the loaded GLSL shader code to SPIR-V, creates the shader module and 
	 * parses all the uniforms and samplers.
	 */
	class NAPAPI ShaderFromFile : public Shader
	{
		RTTI_ENABLE(Shader)
	public:
		ShaderFromFile(Core& core);

		/**
		 * Cross compiles the loaded GLSL shader code to SPIR-V, creates the shader module and parses all the uniforms and samplers.
		 * @param error contains the error if initialization fails.
		 * @return if initialization succeeded.
		 */
		virtual bool init(utility::ErrorState& error) override;

		std::string mVertPath;							///< Property: 'mVertShader' path to the vertex shader on disk
		std::string	mFragPath;							///< Property: 'mFragShader' path to the fragment shader on disk
	};


	/**
	 * Loads a GLSL compute shader from disk using the provided compute shader path.
	 * The shader cross compiles the loaded GLSL shader code to SPIR-V, creates the shader module and
	 * parses all the uniforms and samplers.
	 */
	class NAPAPI ComputeShaderFromFile : public ComputeShader
	{
		RTTI_ENABLE(ComputeShader)
	public:
		ComputeShaderFromFile(Core& core);

		/**
		 * Cross compiles the loaded GLSL shader code to SPIR-V, creates the shader module and parses all the uniforms and samplers.
		 * @param error contains the error if initialization fails.
		 * @return if initialization succeeded.
		 */
		virtual bool init(utility::ErrorState& error) override;

		std::string mComputePath;							///< Property: 'ComputeShader' path to the vertex shader on disk
	};
}


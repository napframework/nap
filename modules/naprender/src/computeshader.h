/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "rendertarget.h"
#include "vertexattributedeclaration.h"
#include "samplerdeclaration.h"
#include "uniformdeclarations.h"

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
	 * Cross-Compiles GLSL vertex and fragment shader code to SPIR-V and creates a Vulkan shader module.
	 * All uniforms, samplers and attributes are extracted.
	 * Make sure to call init() on initialization of a derived shader class.
	 * A nap::Material links to a BaseShader. The shader is compiled on initialization.
	 * Use a nap::Material or nap::MaterialInstance to set / override uniforms and samplers.
	 */
	class NAPAPI ComputeShader : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		ComputeShader(Core& core);
		~ComputeShader() override;

		/**
		* @return all vertex shader attribute declarations.
		*/
		const VertexAttributeDeclarations& getAttributes() const { return mShaderAttributes; }

		/**
		* @return all uniform shader attributes
		*/
		const SamplerDeclarations& getSamplerDeclarations() const { return mSamplerDeclarations; }

		/**
		* @return all UniformBufferObject declarations.
		*/
		const std::vector<UniformBufferObjectDeclaration>& getUBODeclarations() const { return mUBODeclarations; }

		/**
		* @return Vulkan vertex module.
		*/
		VkShaderModule getComputeModule() const { return mComputeModule; }

		/**
		 * @return shader display name
		 */
		const std::string& getDisplayName() const { return mDisplayName; }

		/**
		* @return Vulkan descriptorSetLayout.
		*/
		VkDescriptorSetLayout getDescriptorSetLayout() const { return mDescriptorSetLayout; }

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

		RenderService* mRenderService = nullptr;				///< Handle to render engine

	private:
		bool initLayout(VkDevice device, nap::utility::ErrorState& errorState);

		std::string										mDisplayName;							///< Filename of shader used as display name
		std::vector<UniformBufferObjectDeclaration>		mUBODeclarations;						///< All uniform buffer object declarations
		SamplerDeclarations								mSamplerDeclarations;					///< All sampler declarations
		VertexAttributeDeclarations						mShaderAttributes;						///< Shader program vertex attribute inputs
		VkDescriptorSetLayout							mDescriptorSetLayout = VK_NULL_HANDLE;	///< Descriptor set layout
		VkShaderModule									mComputeModule = VK_NULL_HANDLE;		///< Loaded compute module
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


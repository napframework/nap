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
	class RenderService;
	class Core;

	/**
	 * Resource that loads and compiles a shader from disk using the provided vertex and fragment shader paths.
	 * A nap::Material links to a shader. The shader is compiled on initialization. 
	 * All uniforms, samplers and attributes are extracted. 
	 * Use a nap::Material or nap::MaterialInstance to set / override uniforms and samplers.
	 */
	class NAPAPI Shader : public Resource
	{
		friend class ShaderResourceLoader;
		RTTI_ENABLE(Resource)
	public:
		Shader(Core& core);
		~Shader() override;

		/**
		 * Compiles the shader.
		 * @param errorState contains the error if compilation fails.
		 * @return if compilation succeeded. 
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		* @return all vertex shader attribute declarations.
		*/
		const VertexAttributeDeclarations& getAttributes() const						{ return mShaderAttributes; }

		/**
		* @return all uniform shader attributes
		*/
		const SamplerDeclarations& getSamplerDeclarations() const						{ return mSamplerDeclarations; }

		/**
		* @return all UniformBufferObject declarations.
		*/
		const std::vector<UniformBufferObjectDeclaration>& getUBODeclarations() const	{ return mUBODeclarations; }

		/**
		* @return Vulkan vertex module.
		*/
		VkShaderModule getVertexModule() const											{ return mVertexModule; }

		/**
		* @return Vulkan fragment module.
		*/
		VkShaderModule getFragmentModule() const										{ return mFragmentModule; }

		/**
		* @return Vulkan descriptorSetLayout.
		*/
		VkDescriptorSetLayout getDescriptorSetLayout() const							{ return mDescriptorSetLayout; }

		std::string mVertPath;							///< Property: 'mVertShader' path to the vertex shader on disk
		std::string	mFragPath;							///< Property: 'mFragShader' path to the fragment shader on disk

	private:
		bool initLayout(VkDevice device, nap::utility::ErrorState& errorState);

	private:
		RenderService*									mRenderService;							///< Handle to render engine
		std::string										mDisplayName;							///< Filename of shader used as display name
		std::vector<UniformBufferObjectDeclaration>		mUBODeclarations;						///< All uniform buffer object declaration			s
		SamplerDeclarations								mSamplerDeclarations;					///< All sampler declarations
		VertexAttributeDeclarations						mShaderAttributes;						///< Shader program vertex attribute inputs
		VkDescriptorSetLayout							mDescriptorSetLayout = VK_NULL_HANDLE;	///< Descriptor set layout
		VkShaderModule									mVertexModule = VK_NULL_HANDLE;			///< Loaded vertex module
		VkShaderModule									mFragmentModule = VK_NULL_HANDLE;		///< Loaded fragment module
	};
}


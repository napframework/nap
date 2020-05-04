#pragma once

// External Includes
#include <utility/dllexport.h>
#include <nap/resource.h>
#include "rtti/factory.h"
#include "rendertarget.h"
#include "vertexattributedeclaration.h"
#include "samplerdeclaration.h"
#include "uniformdeclarations.h"

namespace nap
{
	class RenderService;
	class Core;

	/**
	 * Resource that loads and compiles a shader from disk using the provided vertex and fragment shader paths.
	 * A material and material instance link to a shader. The shader is compiled on initialization.
	 */
	class NAPAPI Shader : public Resource
	{
		friend class ShaderResourceLoader;
		RTTI_ENABLE(Resource)
	public:

		/**
		* Known vertex attribute IDs in the system, used for loading/creating meshes with well-known attributes.
		*/
		struct VertexAttributeIDs
		{
			/**
			* @return Default position shader vertex attribute name: "in_Position"
			*/
			static const std::string getPositionVertexAttr();

			/**
			* @return Default normal shader vertex attribute name: "in_Normals"
			*/
			static const std::string getNormalVertexAttr();

			/**
			* @return Default UV shader vertex attribute name: "in_UV#"
			*/
			static const std::string getUVVertexAttr(int uvChannel);

			/**
			* @return Default color shader vertex attribute name: "in_Color#"
			*/
			static const std::string getColorVertexAttr(int colorChannel);

			/**
			* @return Default tangent shader vertex attribute name: "in_Tangent"
			*/
			static const std::string getTangentVertexAttr();

			/**
			* @return Default bi-tangent shader vertex attribute name: "in_Bitangent"
			*/
			static const std::string getBitangentVertexAttr();
		};

		Shader();
		Shader(Core& core);
		~Shader();

		/**
		 * Creates and inits opengl shader.
		 */
		virtual bool init(utility::ErrorState& errorState);

		/**
		* @return all vertex shader attributes
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
		VkShaderModule getVertexModule() const { return mVertexModule; }

		/**
		* @return Vulkan fragment module.
		*/
		VkShaderModule getFragmentModule() const { return mFragmentModule; }

		/**
		* @return Vulkan descriptorSetLayout.
		*/
		VkDescriptorSetLayout getDescriptorSetLayout() const { return mDescriptorSetLayout; }

		std::string										mVertPath;									///< Property: 'mVertShader' path to the vertex shader on disk
		std::string										mFragPath;									///< Property: 'mFragShader' path to the fragment shader on disk

	private:
		bool initLayout(VkDevice device, nap::utility::ErrorState& errorState);

	private:
		RenderService*									mRenderService;
		std::string										mDisplayName;								///< Filename of shader used as displayname

		VkShaderModule									mVertexModule = nullptr;
		VkShaderModule									mFragmentModule = nullptr;
		std::vector<UniformBufferObjectDeclaration>		mUBODeclarations;
		SamplerDeclarations								mSamplerDeclarations;
		VertexAttributeDeclarations						mShaderAttributes;					// Shader program vertex attribute inputs
		VkDescriptorSetLayout							mDescriptorSetLayout = nullptr;
	};
}


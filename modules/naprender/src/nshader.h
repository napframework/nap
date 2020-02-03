#pragma once

// Local Includes
#include "nuniformdeclarations.h"
#include "nsamplerdeclaration.h"
#include "nvertexattributedeclaration.h"

// External includes
#include "vulkan/vulkan_core.h"
#include "utility/errorstate.h"
#include <string>

namespace opengl
{
	/**
	Shader is a class designed to allow us to load and use a Vulkan shader program in
	our application. It allows us to provide filenames for the vertex and
	fragment shaders, and then creates the shader.
	*/
	class NAPAPI Shader
	{
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


		// Default constructor / destructor
		Shader() = default;

		Shader(const Shader&) = delete;
		Shader& operator=(const Shader&) = delete;

		/**
		 * Creates a vertex and fragment shader program
		 * This call will also check for shader compilation issues
		 * @param vsFile the vertex shader file on disk
		 * @param fsFile the vertex shader file on disk
		 */
		bool init(VkDevice device, const std::string& vsFile, const std::string& fsFile, nap::utility::ErrorState& errorState);

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

	private:
		bool initLayout(VkDevice device, nap::utility::ErrorState& errorState);

	private:
		VkShaderModule									mVertexModule = nullptr;
		VkShaderModule									mFragmentModule = nullptr;
		std::vector<UniformBufferObjectDeclaration>		mUBODeclarations;
		SamplerDeclarations								mSamplerDeclarations;
		VertexAttributeDeclarations						mShaderAttributes;					// Shader program vertex attribute inputs
		VkDescriptorSetLayout							mDescriptorSetLayout = nullptr;
	};
}
#pragma once

// External Includes
#include <nshaderutils.h>

// Local Includes
#include "uniforms.h"
#include "imagefromfile.h"

namespace nap
{
	using UniformSamplers = std::unordered_map<std::string, std::unique_ptr<UniformSampler>>;
	using UniformValues = std::unordered_map<std::string, std::unique_ptr<UniformValue>>;

	//////////////////////////////////////////////////////////////////////////

	/**
	 * Manages uniform values and declarations. A uniform value is always tied to a declaration.
	 * Multiple uniform values can be associated with the same declaration.
	 * The uniform declaration is the actual interface to a uniform slot on a shader. 
	 * The uniform value is the actual value uploaded to that slot.
	 * Both the Material and MaterialInstance are a uniform container.
	 */
	class NAPAPI UniformContainer
	{
		RTTI_ENABLE()
	public:
		using UniformStructInstanceList = std::vector<std::unique_ptr<UniformStructInstance>>;
		using SamplerInstanceList = std::vector<std::unique_ptr<UniformSamplerInstance>>;

        UniformContainer() = default;
		UniformContainer(const UniformContainer&) = delete;

        virtual ~UniformContainer() = default;
        
		UniformContainer& operator=(const UniformContainer&) = delete;

		/**
		* @return a uniform texture object that can be used to set a texture or value.
		* If the uniform is not found, returns nullptr.
		*/
		UniformStructInstance* findUniform(const std::string& name);

		/**
		* @return a uniform object that can be used to set a texture or value.
		* If the uniform is not found it will assert.
		*/
		UniformStructInstance& getUniform(const std::string& name);

		const UniformStructInstanceList& getRootStructs() const { return mRootStructs; }
		const SamplerInstanceList& getSamplers() const { return mSamplerInstances; }

		UniformSamplerInstance* findSampler(const std::string& name) const;

	protected:
		UniformStructInstance& createRootStruct(const opengl::UniformStructDeclaration& declaration, const UniformCreatedCallback& uniformCreatedCallback);
		void addSamplerInstance(std::unique_ptr<UniformSamplerInstance> instance);

	private:
		std::vector<std::unique_ptr<UniformStructInstance>> mRootStructs;
		std::vector<std::unique_ptr<UniformSamplerInstance>> mSamplerInstances;
	};
} // nap

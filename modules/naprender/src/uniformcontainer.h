/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "samplerinstance.h"
#include "uniforminstance.h"
#include "storageuniforminstance.h"

namespace nap
{
	/**
	 * Manages uniform struct and sampler instances. Every struct and sampler is tied to a shader declaration.
	 * The declaration acts as the interface to a uniform struct or sampler on a shader. 
	 * Both the nap::Material and nap::MaterialInstance are a uniform container.
	 */
	class NAPAPI UniformContainer
	{
		RTTI_ENABLE()
	public:
		using UniformStructInstanceList = std::vector<std::unique_ptr<UniformStructInstance>>;
		using SamplerInstanceList = std::vector<std::unique_ptr<SamplerInstance>>;

        UniformContainer() = default;
		virtual ~UniformContainer() = default;

		UniformContainer(const UniformContainer&) = delete;
		UniformContainer& operator=(const UniformContainer&) = delete;

		/**
		 * Tries to find a uniform struct instance with the given name.
		 * @param name name of the uniform struct (ubo) as declared in the shader.
		 * @return a uniform struct instance (ubo), nullptr if not present.
		 */
		UniformStructInstance* findUniform(const std::string& name);

		/**
		 * Tries to find a buffer binding instance with the given name.
		 * @param name name of the buffer binding as declared in the shader.
		 * @return a buffer binding instance, nullptr if not present.
		 */
		BufferBindingInstance* findBinding(const std::string& name);

		/**
		 * Returns a uniform struct instance with the given name, the struct has to exist.
		 * @param name name of the uniform struct as declared in the shader.
		 * @return uniform struct instance with the given name, asserts if not present.
		 */
		UniformStructInstance& getUniform(const std::string& name);

		/**
		 * Returns a buffer binding instance with the given name. The binding must exist.
		 * @param name name of the buffer binding as declared in the shader.
		 * @return buffer binding instance with the given name, asserts if not present.
		 */
		BufferBindingInstance& getBinding(const std::string& name);

		/**
		 * @return all the uniforms sampler instances.
		 */
		const SamplerInstanceList& getSamplers() const								{ return mSamplerInstances; }

		/**
		 * Tries to find a sampler instance with the given name.
		 * @param name the name of the sampler as declared in the shader.
		 * @return the sampler with the given name, nullptr if not found.
		 */
		SamplerInstance* findSampler(const std::string& name) const;

	protected:
		UniformStructInstance& createUniformRootStruct(const ShaderVariableStructDeclaration& declaration, const UniformCreatedCallback& uniformCreatedCallback);
		BufferBindingInstance& addBindingInstance(std::unique_ptr<BufferBindingInstance> instance);
		SamplerInstance& addSamplerInstance(std::unique_ptr<SamplerInstance> instance);

	private:
		std::vector<std::unique_ptr<UniformStructInstance>> mUniformRootStructs;
		std::vector<std::unique_ptr<BufferBindingInstance>> mBindingInstances;
		std::vector<std::unique_ptr<SamplerInstance>> mSamplerInstances;
	};
}

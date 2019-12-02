#pragma once

// Local Includes
#include "nshaderutils.h"

// External Includes
#include <rtti/objectptr.h>
#include <glm/glm.hpp>
#include <utility/dllexport.h>
#include <nap/resource.h>
#include "uniforms.h"

namespace nap
{
	class Texture2D;
	class UniformInstance;

	using UniformCreatedCallback = std::function<void()>;

	std::unique_ptr<UniformInstance> NAPAPI createUniformFromDeclaration(const opengl::UniformDeclaration& declaration, const UniformCreatedCallback& uniformCreatedCallback);

	class NAPAPI UniformInstance
	{
		RTTI_ENABLE()

	public:
		virtual const opengl::UniformDeclaration& getDeclaration() const = 0;
	};

	class NAPAPI UniformStructInstance : public UniformInstance
	{
		RTTI_ENABLE(UniformInstance)

	public:
		UniformStructInstance(const opengl::UniformStructDeclaration& declaration, const UniformCreatedCallback& uniformCreatedCallback) :
			mUniformCreatedCallback(uniformCreatedCallback),
			mDeclaration(declaration)
		{
		}

		UniformStructInstance(const UniformStructInstance&) = delete;
		UniformStructInstance& operator=(const UniformStructInstance&) = delete;

		const std::vector<std::unique_ptr<UniformInstance>>& getUniforms() const { return mUniforms; }
		void addUniform(std::unique_ptr<UniformInstance> uniform);

		UniformInstance* findUniform(const std::string& name)
		{
			UniformInstance* instance = nullptr;
			for (auto& uniform_instance : mUniforms)
			{
				if (uniform_instance->getDeclaration().mName == name)
					return uniform_instance.get();
			}

			return nullptr;
		}

		template<typename T>
		T& getOrCreateUniform(const std::string& name)
		{
			UniformInstance* instance = findUniform(name);
			if (instance != nullptr)
			{
				assert(instance->get_type().is_derived_from<T>());
				return *rtti_cast<T>(instance);
			}

			const opengl::UniformDeclaration* declaration = mDeclaration.findMember(name);
			assert(declaration != nullptr);

			std::unique_ptr<UniformInstance> new_instance = createUniformFromDeclaration(*declaration, mUniformCreatedCallback);
			
			T* result = rtti_cast<T>(new_instance.get());
			assert(result != nullptr);
			
			mUniforms.emplace_back(std::move(new_instance));

			if (mUniformCreatedCallback)
				mUniformCreatedCallback();

			return *result;
		}

		virtual const opengl::UniformDeclaration& getDeclaration() const override { return mDeclaration; }

	private:
		UniformCreatedCallback mUniformCreatedCallback;
		const opengl::UniformStructDeclaration& mDeclaration;
		std::vector<std::unique_ptr<UniformInstance>> mUniforms;
	};

	class NAPAPI UniformStructArrayInstance : public UniformInstance
	{
		RTTI_ENABLE(UniformInstance)

	public:
		UniformStructArrayInstance(const opengl::UniformStructArrayDeclaration& declaration) :
			mDeclaration(declaration)
		{
		}

		UniformStructArrayInstance(const UniformStructArrayInstance&) = delete;
		UniformStructArrayInstance& operator=(const UniformStructArrayInstance&) = delete;

		const std::vector<std::unique_ptr<UniformStructInstance>>& getElements() const { return mElements; }
		void addElement(std::unique_ptr<UniformStructInstance> element);

		UniformStructInstance& getElement(int index)
		{
			assert(index < mElements.size());
			return *mElements[index];
		}

		virtual const opengl::UniformDeclaration& getDeclaration() const override { return mDeclaration; }

	private:
		const opengl::UniformStructArrayDeclaration&		mDeclaration;
		std::vector<std::unique_ptr<UniformStructInstance>> mElements;
	};

	class NAPAPI UniformValueInstance : public UniformInstance
	{
		RTTI_ENABLE(UniformInstance)

	public:
		UniformValueInstance(const opengl::UniformValueDeclaration& declaration) :
			mDeclaration(&declaration)
		{
		}

		virtual const opengl::UniformDeclaration& getDeclaration() const override { return *mDeclaration; }

		virtual void push(uint8_t* uniformBuffer) const = 0;

	protected:
		const opengl::UniformValueDeclaration*	mDeclaration = nullptr;
	};

	template<class T>
	class TypedUniformValueInstance : public UniformValueInstance
	{
		RTTI_ENABLE(UniformValueInstance)

	public:
		TypedUniformValueInstance(const opengl::UniformValueDeclaration& declaration) :
			UniformValueInstance(declaration)
		{
		}

		/**
		* @param value integer value to set.
		*/
		void setValue(T value) { mValue = value; }

		void set(const TypedUniformValue<T>& resource)
		{
			mValue = resource.mValue;
		}

		/**
		* Updates the uniform in the shader.
		* @param declaration: the uniform declaration from the shader that is used to set the value.
		*/
		virtual void push(uint8_t* uniformBuffer) const override
		{
			assert(sizeof(mValue) == mDeclaration->mSize);
			memcpy(uniformBuffer + mDeclaration->mOffset, &mValue, sizeof(mValue));
		}

	private:
		T mValue = T();
	};

	class NAPAPI UniformValueArrayInstance : public UniformInstance
	{
		RTTI_ENABLE(UniformInstance)

	public:
		UniformValueArrayInstance(const opengl::UniformValueArrayDeclaration& declaration) :
			mDeclaration(&declaration)
		{
		}

		virtual const opengl::UniformDeclaration& getDeclaration() const override { return *mDeclaration; }

		virtual void push(uint8_t* uniformBuffer) const = 0;

	protected:
		const opengl::UniformValueArrayDeclaration* mDeclaration;
	};

	template<typename T>
	class TypedUniformValueArrayInstance : public UniformValueArrayInstance
	{
		RTTI_ENABLE(UniformValueArrayInstance)

	public:
		TypedUniformValueArrayInstance(const opengl::UniformValueArrayDeclaration& declaration) :
			UniformValueArrayInstance(declaration)
		{
		}

		/**
		* Updates the uniform in the shader.
		* @param declaration: the uniform declaration from the shader that is used to set the value.
		*/
		virtual void push(uint8_t* uniformBuffer) const override
		{
			size_t size = mValues.size() * sizeof(T);
			assert(size == mDeclaration->mSize);
			memcpy(uniformBuffer + mDeclaration->mOffset, mValues.data(), size);
		}

		void set(const TypedUniformValueArray<T>& resource)
		{
			mValues = resource.mValues;
		}

		std::vector<T>& getValues() { return mValues; }

	private:
		std::vector<T> mValues;
	};

	using UniformIntInstance = TypedUniformValueInstance<int>;
	using UniformFloatInstance = TypedUniformValueInstance<float>;
	using UniformVec3Instance = TypedUniformValueInstance<glm::vec3>;
	using UniformVec4Instance = TypedUniformValueInstance<glm::vec4>;
	using UniformMat4Instance = TypedUniformValueInstance<glm::mat4>;

	using UniformIntArrayInstance = TypedUniformValueArrayInstance<int>;
	using UniformFloatArrayInstance = TypedUniformValueArrayInstance<float>;
	using UniformVec3ArrayInstance = TypedUniformValueArrayInstance<glm::vec3>;
	using UniformVec4ArrayInstance = TypedUniformValueArrayInstance<glm::vec4>;
	using UniformMat4ArrayInstance = TypedUniformValueArrayInstance<glm::mat4>;
}

#pragma once

// Local Includes
#include "nshaderutils.h"

// External Includes
#include <rtti/objectptr.h>
#include <glm/glm.hpp>
#include <utility/dllexport.h>
#include <nap/resource.h>

namespace nap
{
	class Texture2D;
	class UniformInstance;

	using UniformCreatedCallback = std::function<void()>;

	std::unique_ptr<UniformInstance> NAPAPI createUniformFromDeclaration(const opengl::UniformDeclaration& declaration, const UniformCreatedCallback& uniformCreatedCallback);

	/**
	 * Base class for all types of uniforms, whether texture or value.
	 */
	class NAPAPI Uniform : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		std::string mName;		///< Name of uniform as in shader
	};

	class NAPAPI UniformInstance
	{
		RTTI_ENABLE()

	public:
		virtual const opengl::UniformDeclaration& getDeclaration() const = 0;
	};

	/**
	 * Represents a 'struct' uniform. A struct can contain other uniforms, but does not directly map onto a uniform in the shader
	 */
	class NAPAPI UniformStruct : public Uniform
	{
		RTTI_ENABLE(Uniform)
	public:

		void addUniform(Uniform& uniform);
		Uniform* findUniform(const std::string& name);

	public:
		std::vector<rtti::ObjectPtr<Uniform>> mUniforms;
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

	/**
	 * Represents an 'array of structures' uniform. A UniformStructArray can contain UniformStructs, but does not directly map onto a uniform in the shader
	 */
	class NAPAPI UniformStructArray : public Uniform
	{
		RTTI_ENABLE(Uniform)
	public:
		void insertStruct(int index, UniformStruct& uniformStruct);

	public:
		std::vector<rtti::ObjectPtr<UniformStruct>> mStructs;
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

	/**
	* Represents a 'value' uniform, or: something that is not a texture.
	* Derived classes should store value data and implement push() to update the
	* value in the shader.
	*/
	class NAPAPI UniformValue : public Uniform
	{
		RTTI_ENABLE(Uniform)
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


	template<typename T>
	class TypedUniformValue : public UniformValue
	{
		RTTI_ENABLE(UniformValue)
	
	public:
		T mValue = T();			///< Data storage
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

	/**
	* Represents an array of 'value' uniforms.
	* Derived classes should return the correct amount of elements that are in the array.
	*/
	class NAPAPI UniformValueArray : public UniformValue
	{
		RTTI_ENABLE(UniformValue)

	public:
		/**
		 * Retrieve the number of elements in this array
		 *
		 * @return The number of elements in this array
		 */
		virtual int getCount() const = 0;
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
	class TypedUniformValueArray : public UniformValueArray
	{
		RTTI_ENABLE(UniformValueArray)

		virtual int getCount() const override { return mValues.size(); }

	public:
		std::vector<T> mValues;
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


	using UniformInt = TypedUniformValue<int>;
	using UniformFloat = TypedUniformValue<float>;
	using UniformVec3 = TypedUniformValue<glm::vec3>;
	using UniformVec4 = TypedUniformValue<glm::vec4>;
	using UniformMat4 = TypedUniformValue<glm::mat4>;

	using UniformIntInstance = TypedUniformValueInstance<int>;
	using UniformFloatInstance = TypedUniformValueInstance<float>;
	using UniformVec3Instance = TypedUniformValueInstance<glm::vec3>;
	using UniformVec4Instance = TypedUniformValueInstance<glm::vec4>;
	using UniformMat4Instance = TypedUniformValueInstance<glm::mat4>;

	using UniformIntArray = TypedUniformValueArray<int>;
	using UniformFloatArray = TypedUniformValueArray<float>;
	using UniformVec3Array = TypedUniformValueArray<glm::vec3>;
	using UniformVec4Array = TypedUniformValueArray<glm::vec4>;
	using UniformMat4Array = TypedUniformValueArray<glm::mat4>;

	using UniformIntArrayInstance = TypedUniformValueArrayInstance<int>;
	using UniformFloatArrayInstance = TypedUniformValueArrayInstance<float>;
	using UniformVec3ArrayInstance = TypedUniformValueArrayInstance<glm::vec3>;
	using UniformVec4ArrayInstance = TypedUniformValueArrayInstance<glm::vec4>;
	using UniformMat4ArrayInstance = TypedUniformValueArrayInstance<glm::mat4>;

	class NAPAPI Sampler : public Resource
	{
		RTTI_ENABLE(Resource)

	public:
		Sampler() = default;

		std::string mName;
	};


	/**
	 * Represents an array of 'texture' uniforms.
	 * Derived classes should return the correct amount of elements that are in the array.
 	 */
	class NAPAPI SamplerArray : public Sampler
	{
		RTTI_ENABLE(Sampler)

	public:
		/**
		 * Retrieve the number of elements in this array
		 *
		 * @return The number of elements in this array
		 */
		virtual int getNumElements() const = 0;
	};

	class NAPAPI SamplerInstance
	{
		RTTI_ENABLE()

	public:
		SamplerInstance(VkDevice device, const opengl::SamplerDeclaration& declaration);

		const opengl::SamplerDeclaration& getDeclaration() const { assert(mDeclaration != nullptr); return *mDeclaration; }
		VkSampler getSampler() const { return mSampler; }

	private:
		const opengl::SamplerDeclaration*	mDeclaration = nullptr;
		VkSampler									mSampler = nullptr;
	};

	/**
	 * Pointer to a single 2D Texture that can be pushed to a shader uniform.
	 */
	class NAPAPI Sampler2D : public Sampler
	{
		RTTI_ENABLE(Sampler)

	public:
		Sampler2D() = default;

		rtti::ObjectPtr<Texture2D> mTexture = nullptr;		///< Texture to use for this uniform
	};

	class NAPAPI Sampler2DInstance : public SamplerInstance
	{
		RTTI_ENABLE(SamplerInstance)

	public:
		Sampler2DInstance(VkDevice device, const opengl::SamplerDeclaration& declaration, const Sampler2D* sampler2D);

		/**
		* @param texture The texture resource to set for this uniform.
		*/
		void setTexture(Texture2D& texture) { mTexture2D = &texture; }

		rtti::ObjectPtr<Texture2D>					mTexture2D;
	};


	/**
	 * Stores and array of 2D textures.
	 * Number of textures must be equal or lower than the number of textures declared in the shader.
	 */
	class NAPAPI Sampler2DArray : public SamplerArray
	{
		RTTI_ENABLE(SamplerArray)

	public:

		Sampler2DArray() = default;

		/**
		 * Constructor to ensure the array has the correct size to match the shader
		 */
		Sampler2DArray(int inSize) :
			mTextures(inSize)
		{
		}

		/**
		 * Retrieve the number of elements in this array
		 *
		 * @return The number of elements in this array
		 */
		virtual int getNumElements() const override { return mTextures.size(); }

		std::vector<rtti::ObjectPtr<Texture2D>> mTextures;		///< Texture to use for this uniform
	};

	class NAPAPI Sampler2DArrayInstance : public SamplerInstance
	{
		RTTI_ENABLE(SamplerInstance)

	public:
		Sampler2DArrayInstance(VkDevice device, const opengl::SamplerDeclaration& declaration, const Sampler2DArray* sampler2DArray);

		std::vector<rtti::ObjectPtr<Texture2D>>				mTextures;
	};

}

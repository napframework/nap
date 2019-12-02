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

	class NAPAPI UniformSampler : public Resource
	{
		RTTI_ENABLE(Resource)

	public:
		UniformSampler() = default;

		std::string mName;
	};


	/**
	 * Represents an array of 'texture' uniforms.
	 * Derived classes should return the correct amount of elements that are in the array.
 	 */
	class NAPAPI UniformSamplerArray : public UniformSampler
	{
		RTTI_ENABLE(UniformSampler)

	public:
		/**
		 * Retrieve the number of elements in this array
		 *
		 * @return The number of elements in this array
		 */
		virtual int getNumElements() const = 0;
	};

	/**
	 * Represents a single int value that can be pushed to a shader uniform.
	 */
	class NAPAPI UniformInt : public UniformValue
	{
		RTTI_ENABLE(UniformValue)
	public:
		int mValue = 0;			///< Data storage
	};

	class NAPAPI UniformIntInstance : public UniformValueInstance
	{
		RTTI_ENABLE(UniformValueInstance)

	public:
		UniformIntInstance(const opengl::UniformValueDeclaration& declaration, const UniformInt* uniformInt) :
			UniformValueInstance(declaration)
		{
			if (uniformInt != nullptr)
				mValue = uniformInt->mValue;
		}

		/**
		* @param value integer value to set.
		*/
		void setValue(int value) { mValue = value; }

		/**
		* Updates the uniform in the shader.
		* @param declaration: the uniform declaration from the shader that is used to set the value.
		*/
		virtual void push(uint8_t* uniformBuffer) const override;

		int mValue = 0;
	};


	/**
	 * Represents a single float value that can be pushed to a shader uniform.
	 */
	class NAPAPI UniformFloat : public UniformValue
	{
		RTTI_ENABLE(UniformValue)

	public:

		float mValue = 0.0f;			///< Data storage
	};

	class NAPAPI UniformFloatInstance : public UniformValueInstance
	{
		RTTI_ENABLE(UniformValueInstance)

	public:
		UniformFloatInstance(const opengl::UniformValueDeclaration& declaration, const UniformFloat* uniformFloat) :
			UniformValueInstance(declaration)
		{
			if (uniformFloat != nullptr)
				mValue = uniformFloat->mValue;
		}

		/**
		* @param value integer value to set.
		*/
		void setValue(float value) { mValue = value; }

		/**
		* Updates the uniform in the shader.
		* @param declaration: the uniform declaration from the shader that is used to set the value.
		*/
		virtual void push(uint8_t* uniformBuffer) const override;

		float mValue = 0.0f;
	};

	/**
	 * Represents a single vec3 value that can be pushed to a shader uniform.
	 */
	class NAPAPI UniformVec3 : public UniformValue
	{
		RTTI_ENABLE(UniformValue)
	public:

		glm::vec3 mValue;		///< Data storage
	};

	class NAPAPI UniformVec3Instance : public UniformValueInstance
	{
		RTTI_ENABLE(UniformValueInstance)

	public:
		UniformVec3Instance(const opengl::UniformValueDeclaration& declaration, const UniformVec3* uniformVec3) :
			UniformValueInstance(declaration)
		{
			if (uniformVec3 != nullptr)
				mValue = uniformVec3->mValue;
		}

		/**
		* @param value vec4 value to set.
		*/
		void setValue(const glm::vec3& value) { mValue = value; }

		/**
		* Updates the uniform in the shader.
		* @param declaration: the uniform declaration from the shader that is used to set the value.
		*/
		virtual void push(uint8_t* uniformBuffer) const override;

		glm::vec3 mValue;
	};

	/**
	 * Represents a single vec4 value that can be pushed to a shader uniform.
	 */
	class NAPAPI UniformVec4 : public UniformValue
	{
		RTTI_ENABLE(UniformValue)
	public:

		glm::vec4 mValue;		///< Data storage
	};

	class NAPAPI UniformVec4Instance : public UniformValueInstance
	{
		RTTI_ENABLE(UniformValueInstance)

	public:
		UniformVec4Instance(const opengl::UniformValueDeclaration& declaration, const UniformVec4* uniformVec4) :
			UniformValueInstance(declaration)
		{
			if (uniformVec4 != nullptr)
				mValue = uniformVec4->mValue;
		}

		/**
		* @param value vec4 value to set.
		*/
		void setValue(const glm::vec4& value) { mValue = value; }

		/**
		* Updates the uniform in the shader.
		* @param declaration: the uniform declaration from the shader that is used to set the value.
		*/
		virtual void push(uint8_t* uniformBuffer) const override;

		glm::vec4 mValue;
	};

	/**
	 * Represents a single 4x4 matrix that can be pushed to a shader uniform.
	 */
	class NAPAPI UniformMat4 : public UniformValue
	{
		RTTI_ENABLE(UniformValue)

	public:
		glm::mat4 mValue;		///< Data storage
	};

	class NAPAPI UniformMat4Instance : public UniformValueInstance
	{
		RTTI_ENABLE(UniformValueInstance)

	public:
		UniformMat4Instance(const opengl::UniformValueDeclaration& declaration, const UniformMat4* uniformMat4) :
			UniformValueInstance(declaration)
		{
			if (uniformMat4 != nullptr)
				mValue = uniformMat4->mValue;
		}
		
		/**
		* @param value mat4 value to set.
		*/
		void setValue(const glm::mat4& value) { mValue = value; }

		/**
		* Updates the uniform in the shader.
		* @param declaration: the uniform declaration from the shader that is used to set the value.
		*/
		virtual void push(uint8_t* uniformBuffer) const override;

		glm::mat4 mValue;
	};

	class NAPAPI UniformSamplerInstance
	{
		RTTI_ENABLE()

	public:
		UniformSamplerInstance(VkDevice device, const opengl::UniformSamplerDeclaration& declaration);

		const opengl::UniformSamplerDeclaration& getDeclaration() const { assert(mDeclaration != nullptr); return *mDeclaration; }
		VkSampler getSampler() const { return mSampler; }

	private:
		const opengl::UniformSamplerDeclaration*	mDeclaration = nullptr;
		VkSampler									mSampler = nullptr;
	};

	/**
	 * Pointer to a single 2D Texture that can be pushed to a shader uniform.
	 */
	class NAPAPI UniformSampler2D : public UniformSampler
	{
		RTTI_ENABLE(UniformSampler)

	public:
		UniformSampler2D() = default;

		rtti::ObjectPtr<Texture2D> mTexture = nullptr;		///< Texture to use for this uniform
	};

	class NAPAPI UniformSampler2DInstance : public UniformSamplerInstance
	{
		RTTI_ENABLE(UniformSamplerInstance)

	public:
		UniformSampler2DInstance(VkDevice device, const opengl::UniformSamplerDeclaration& declaration, const UniformSampler2D* sampler2D);

		/**
		* @param texture The texture resource to set for this uniform.
		*/
		void setTexture(Texture2D& texture) { mTexture2D = &texture; }

		rtti::ObjectPtr<Texture2D>					mTexture2D;
	};
	
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Stores an array of integer data and is capable of updating the integer uniform in the shader.
	 * Number of elements must be equal or lower than the number of elements declared in the shader.
	 */
	class NAPAPI UniformIntArray : public UniformValueArray
	{
		RTTI_ENABLE(UniformValueArray)

	public:

		UniformIntArray() = default;

		/**
		 * Constructor to ensure the array has the correct size to match the shader
		 */
		UniformIntArray(int inSize) :
			mValues(inSize)
		{
		}

		/**
		 * Retrieve the number of elements in this array
		 *
		 * @return The number of elements in this array
		 */
		virtual int getCount() const override { return mValues.size(); }

		std::vector<int> mValues;			///< Data storage
	};

	class NAPAPI UniformIntArrayInstance : public UniformValueArrayInstance
	{
		RTTI_ENABLE(UniformValueArrayInstance)

	public:
		UniformIntArrayInstance(const opengl::UniformValueArrayDeclaration& declaration, const UniformIntArray* uniformIntArray) :
			UniformValueArrayInstance(declaration)
		{
			if (uniformIntArray != nullptr)
				mValues = uniformIntArray->mValues;
		}

		/**
		* Updates the uniform in the shader.
		* @param declaration: the uniform declaration from the shader that is used to set the value.
		*/
		virtual void push(uint8_t* uniformBuffer) const override;

		std::vector<int> mValues;			///< Data storage
	};


	/**
	 * Stores an array of float data and is capable of updating the integer uniform in the shader.
	 * Number of elements must be equal or lower than the number of elements declared in the shader.
	 */
	class NAPAPI UniformFloatArray : public UniformValueArray
	{
		RTTI_ENABLE(UniformValueArray)

	public:

		UniformFloatArray() = default;

		/**
		 * Constructor to ensure the array has the correct size to match the shader
		 */
		UniformFloatArray(int inSize) :
			mValues(inSize)
		{
		}

		/**
		 * Retrieve the number of elements in this array
		 *
		 * @return The number of elements in this array
		 */
		virtual int getCount() const override { return mValues.size(); }

		std::vector<float> mValues;			///< Data storage
	};

	class NAPAPI UniformFloatArrayInstance : public UniformValueArrayInstance
	{
		RTTI_ENABLE(UniformValueArrayInstance)

	public:
		UniformFloatArrayInstance(const opengl::UniformValueArrayDeclaration& declaration, const UniformFloatArray* uniformFloatArray) :
			UniformValueArrayInstance(declaration)
		{
			if (uniformFloatArray != nullptr)
				mValues = uniformFloatArray->mValues;
		}

		/**
		* Updates the uniform in the shader.
		* @param declaration: the uniform declaration from the shader that is used to set the value.
		*/
		virtual void push(uint8_t* uniformBuffer) const override;

		std::vector<float> mValues;			///< Data storage
	};

	/**
	 * Stores an array of vec3 data and is capable of updating the vec3 uniform in the shader.
	 * Number of elements must be equal or lower than the number of elements declared in the shader.
	 */
	class NAPAPI UniformVec3Array : public UniformValueArray
	{
		RTTI_ENABLE(UniformValueArray)

	public:

		UniformVec3Array() = default;

		/**
		 * Constructor to ensure the array has the correct size to match the shader
		 */
		UniformVec3Array(int inSize) :
			mValues(inSize)
		{
		}

		/**
		 * Retrieve the number of elements in this array
		 *
		 * @return The number of elements in this array
		 */
		virtual int getCount() const override { return mValues.size(); }

		std::vector<glm::vec3> mValues;		///< Data storage
	};

	class NAPAPI UniformVec3ArrayInstance : public UniformValueArrayInstance
	{
		RTTI_ENABLE(UniformValueArrayInstance)

	public:
		UniformVec3ArrayInstance(const opengl::UniformValueArrayDeclaration& declaration, const UniformVec3Array* uniformVec3Array) :
			UniformValueArrayInstance(declaration)
		{
			if (uniformVec3Array != nullptr)
				mValues = uniformVec3Array->mValues;
		}

		/**
		* Updates the uniform in the shader.
		* @param declaration: the uniform declaration from the shader that is used to set the value.
		*/
		virtual void push(uint8_t* uniformBuffer) const override;

		std::vector<glm::vec3> mValues;			///< Data storage
	};

	/**
	 * Stores an array of vec4 data and is capable of updating the vec4 uniform in the shader.
	 * Number of elements must be equal or lower than the number of elements declared in the shader.
	 */
	class NAPAPI UniformVec4Array : public UniformValueArray
	{
		RTTI_ENABLE(UniformValueArray)

	public:

		UniformVec4Array() = default;

		/**
		 * Constructor to ensure the array has the correct size to match the shader
		 */
		UniformVec4Array(int inSize) :
			mValues(inSize)
		{
		}

		/**
		 * Retrieve the number of elements in this array
		 *
		 * @return The number of elements in this array
		 */
		virtual int getCount() const override { return mValues.size(); }

		std::vector<glm::vec4> mValues;		///< Data storage
	};

	class NAPAPI UniformVec4ArrayInstance : public UniformValueArrayInstance
	{
		RTTI_ENABLE(UniformValueArrayInstance)

	public:
		UniformVec4ArrayInstance(const opengl::UniformValueArrayDeclaration& declaration, const UniformVec4Array* uniformVec4Array) :
			UniformValueArrayInstance(declaration)
		{
			if (uniformVec4Array != nullptr)
				mValues = uniformVec4Array->mValues;
		}

		/**
		* Updates the uniform in the shader.
		* @param declaration: the uniform declaration from the shader that is used to set the value.
		*/
		virtual void push(uint8_t* uniformBuffer) const override;

		std::vector<glm::vec4> mValues;			///< Data storage
	};

	/**
	 * Stores an array of 4x4 matrix data and is capable of updating the 4x4 matrix uniform in the shader.
	 * Number of elements must be equal or lower than the number of elements declared in the shader.
	 */
	class NAPAPI UniformMat4Array : public UniformValueArray
	{
		RTTI_ENABLE(UniformValueArray)

	public:

		UniformMat4Array() = default;

		/**
		 * Constructor to ensure the array has the correct size to match the shader
		 */
		UniformMat4Array(int inSize) :
			mValues(inSize)
		{
		}

		/**
		 * Retrieve the number of elements in this array
		 *
		 * @return The number of elements in this array
		 */
		virtual int getCount() const override { return mValues.size(); }

		std::vector<glm::mat4> mValues;		///< Data storage
	};

	class NAPAPI UniformMat4ArrayInstance : public UniformValueArrayInstance
	{
		RTTI_ENABLE(UniformValueArrayInstance)

	public:
		UniformMat4ArrayInstance(const opengl::UniformValueArrayDeclaration& declaration, const UniformMat4Array* uniformMat4Array) :
			UniformValueArrayInstance(declaration)
		{
			if (uniformMat4Array != nullptr)
				mValues = uniformMat4Array->mValues;
		}

		/**
		* Updates the uniform in the shader.
		* @param declaration: the uniform declaration from the shader that is used to set the value.
		*/
		virtual void push(uint8_t* uniformBuffer) const override;

		std::vector<glm::mat4> mValues;			///< Data storage
	};

	/**
	 * Stores and array of 2D textures.
	 * Number of textures must be equal or lower than the number of textures declared in the shader.
	 */
	class NAPAPI UniformSampler2DArray : public UniformSamplerArray
	{
		RTTI_ENABLE(UniformSamplerArray)

	public:

		UniformSampler2DArray() = default;

		/**
		 * Constructor to ensure the array has the correct size to match the shader
		 */
		UniformSampler2DArray(int inSize) :
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

	class NAPAPI UniformSampler2DArrayInstance : public UniformSamplerInstance
	{
		RTTI_ENABLE(UniformSamplerInstance)

	public:
		UniformSampler2DArrayInstance(VkDevice device, const opengl::UniformSamplerDeclaration& declaration, const UniformSampler2DArray* sampler2DArray);

		std::vector<rtti::ObjectPtr<Texture2D>>				mTextures;
	};

}

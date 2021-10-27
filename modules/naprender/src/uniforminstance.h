/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "uniform.h"
#include "storagebuffer.h"

// External Includes
#include <rtti/objectptr.h>
#include <glm/glm.hpp>
#include <utility/dllexport.h>
#include <nap/resource.h>

namespace nap
{
	// Forward Declares
	class Texture2D;
	class UniformInstance;

	using UniformCreatedCallback = std::function<void()>;

	/**
	 * Instantiated version of a nap::Uniform.
	 * Every uniform 'resource' has an associative 'instance', ie: nap::UniformValue -> nap::UniformValueInstance.
	 * An instance can be updated / inspected at run-time and is associated with a declaration.
	 */
	class NAPAPI UniformInstance
	{
		RTTI_ENABLE()

	public:
		// Default Destructor
		virtual ~UniformInstance() = default;

		/**
		 * Required virtual, needs to be implemented in derived classes
		 * @return the declaration associated with this uniform instance
		 */
		virtual const UniformDeclaration& getDeclaration() const = 0;
	};


	//////////////////////////////////////////////////////////////////////////
	// UniformStructInstance
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Contains other uniform instances, including: values, structs and arrays. 
	 * A uniform value, struct or array must be declared as part of a uniform struct.
	 */
	class NAPAPI UniformStructInstance : public UniformInstance
	{
		RTTI_ENABLE(UniformInstance)

	public:

		// Constructor
		UniformStructInstance(const UniformStructDeclaration& declaration, const UniformCreatedCallback& uniformCreatedCallback) :
			mUniformCreatedCallback(uniformCreatedCallback),
			mDeclaration(declaration)
		{ }

		// Delete copy assignment operator and copy constructor
		UniformStructInstance(const UniformStructInstance&) = delete;
		UniformStructInstance& operator=(const UniformStructInstance&) = delete;

		/**
		 * @return all uniform instances contained by this struct.
		 */
		const std::vector<std::unique_ptr<UniformInstance>>& getUniforms() const	{ return mUniforms; }

		/**
		 * Tries to find a uniform with the given name.
		 * @param name the name of the uniform to find.
		 * @return found uniform instance, nullptr if it does not exist.
		 */
		UniformInstance* findUniform(const std::string& name);

		/**
		 * Tries to find a uniform of a specific type with the given name.
		 * @param name the name of the uniform to find.
		 * @return the uniform instance, nullptr if it does not exist
		 */
		template<typename T>
		T* findUniform(const std::string& name);

		/**
		 * Tries to find a uniform of a specific type with the given name, creates it if it does not exist.
		 * @param name the name of the uniform to find or create
		 * @return the uniform instance, nullptr if it can't be found and created.
		 */
		template<typename T>
		T* getOrCreateUniform(const std::string& name);

		/**
		 * @return the uniform declaration, used to create the uniform instance.
		 */
		virtual const UniformDeclaration& getDeclaration() const override		{ return mDeclaration; }

	private:
		friend class BaseMaterial;
		friend class ComputeMaterial;
		friend class Material;
		friend class BaseMaterialInstance;
		friend class MaterialInstance;

		/**
		 * Adds all associated uniforms recursively to this instance, based on the struct declaration and resource.
		 */
		bool addUniformRecursive(const UniformStructDeclaration& structDeclaration, const UniformStruct* structResource, const UniformCreatedCallback& uniformCreatedCallback, bool createDefaults, utility::ErrorState& errorState);

		/**
		 * Creates a uniform instance from a uniform declaration. 
		 * @param declaration the uniform declaration
		 * @param uniformCreatedCallback callback that is triggered when the uniform is created.
		 */
		static std::unique_ptr<UniformInstance> createUniformFromDeclaration(const UniformDeclaration& declaration, const UniformCreatedCallback& uniformCreatedCallback);

	private:
		UniformCreatedCallback							mUniformCreatedCallback;
		const UniformStructDeclaration&					mDeclaration;
		std::vector<std::unique_ptr<UniformInstance>>	mUniforms;
	};


	//////////////////////////////////////////////////////////////////////////
	// UniformStructArrayInstance
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Represents a list of uniform struct instances.
	 */
	class NAPAPI UniformStructArrayInstance : public UniformInstance
	{
		friend class UniformStructInstance;
		RTTI_ENABLE(UniformInstance)
	public:
		// Constructor
		UniformStructArrayInstance(const UniformStructArrayDeclaration& declaration) :
			mDeclaration(declaration)
		{ }

		// Copy construction and copy assignment not allowed
		UniformStructArrayInstance(const UniformStructArrayInstance&) = delete;
		UniformStructArrayInstance& operator=(const UniformStructArrayInstance&) = delete;

		/**
		 * @return all uniform struct instance elements
		 */
		const std::vector<std::unique_ptr<UniformStructInstance>>& getElements() const	{ return mElements; }

		/**
		 * @return the uniform struct at the given index.
		 */
		UniformStructInstance& getElement(int index)									{ assert(index < mElements.size()); return *mElements[index]; }

		/**
		 * @return the uniform struct at the given index, nullptr if not found
		 */
		UniformStructInstance* findElement(int index);

		/**
		 * @return the uniform struct at the given index
		 */
		UniformStructInstance& operator[](size_t index)									{ return getElement(index); }

		/**
		 * @return declaration used to create this instance. 
		 */
		virtual const UniformDeclaration& getDeclaration() const override				{ return mDeclaration; }

	private:
		const UniformStructArrayDeclaration&					mDeclaration;
		std::vector<std::unique_ptr<UniformStructInstance>>		mElements;

		/**
		 * Adds a uniform struct instance.
		 * @param element the element to add
		 */
		void addElement(std::unique_ptr<UniformStructInstance> element);
	};


	//////////////////////////////////////////////////////////////////////////
	// UniformLeafInstance
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Base class of all concrete uniform instances and uniform instance array types, including
	 * value and value array types. Every leaf can push data on to the GPU. 
	 */
	class NAPAPI UniformLeafInstance : public UniformInstance
	{
		RTTI_ENABLE(UniformInstance)
	public:
		/**
		 * Needs to be implemented in derived classes, pushes buffer to the GPU.
		 */
		virtual void push(uint8_t* uniformBuffer) const = 0;
	};


	//////////////////////////////////////////////////////////////////////////
	// Uniform Values
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Base class of a uniform value instance.
	 */
	class NAPAPI UniformValueInstance : public UniformLeafInstance
	{
		RTTI_ENABLE(UniformLeafInstance)
	public:

		// Constructor
		UniformValueInstance(const UniformValueDeclaration& declaration) :
			mDeclaration(&declaration) { }

		/**
		 * @return the uniform value declaration.
		 */
		virtual const UniformDeclaration& getDeclaration() const override		{ return *mDeclaration; }

	protected:
		const UniformValueDeclaration*	mDeclaration = nullptr;
	};


	/**
	 * Specific type of uniform value instance, for example: float -> TypedUniformValueInstance<float>.
	 */
	template<class T>
	class TypedUniformValueInstance : public UniformValueInstance
	{
		RTTI_ENABLE(UniformValueInstance)

	public:
		TypedUniformValueInstance(const UniformValueDeclaration& declaration) :
			UniformValueInstance(declaration) { }

		/**
		 * Updates the uniform value, data is not pushed immediately. 
		 * @param value new uniform value
		 */
		void setValue(T value)								{ mValue = value; }
		
		/**
		 * Update instance from resource, data is not pushed immediately. 
		 * @param resource the resource to copy the value from
		 */
		void set(const TypedUniformValue<T>& resource)		{ mValue = resource.mValue; 	}

		/**
		 * Pushes the data to the 'Shader'.
		 * @param uniformBuffer: the buffer in which to copy the value.
		 */
		virtual void push(uint8_t* uniformBuffer) const override;

	private:
		T mValue = T();
	};


	//////////////////////////////////////////////////////////////////////////
	// Uniform Value Array
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Base class of all uniform value array instances.
	 */
	class NAPAPI UniformValueArrayInstance : public UniformLeafInstance
	{
		RTTI_ENABLE(UniformLeafInstance)

	public:
		UniformValueArrayInstance(const UniformValueArrayDeclaration& declaration) :
			mDeclaration(&declaration) { }

		/**
		 * @return uniform declaration.
		 */
		virtual const UniformDeclaration& getDeclaration() const override	{ return *mDeclaration; }

		/**
		 * Required override, sets up default values.
		 */
		virtual void setDefault() = 0;

	protected:
		const UniformValueArrayDeclaration* mDeclaration;
	};


	/**
	 * Specific type of uniform value array instance, for example: 
	 * std::vector<float> -> TypedUniformValueArrayInstance<float>.
	 * All supported types are defined below for easier readability. 
	 */
	template<typename T>
	class TypedUniformValueArrayInstance : public UniformValueArrayInstance
	{
		RTTI_ENABLE(UniformValueArrayInstance)

	public:
		TypedUniformValueArrayInstance(const UniformValueArrayDeclaration& declaration) :
			UniformValueArrayInstance(declaration)				{ }

		/**
		 * Updates the uniform value from a resource, data is not pushed immediately. 
		 * @param resource resource to copy data from.
		 */
		void set(const TypedUniformValueArray<T>& resource)		{ mValues = resource.mValues; }

		/**
		 * Updates the uniform value, data is not pushed immediately.
		 * Note that the length of the given values must be =< than length declared in shader.
		 * @param values new list of values
		 */
		void setValues(const std::vector<T>& values)			{ assert(values.size() <= mDeclaration->mNumElements); mValues = values; }

		/**
		 * Updates a single uniform value in the array, data is not pushed immediately.
		 * Note that the given index must be =< than length declared in shader. 
		 * @param value the value to set
		 * @param index the index in the array
		 */
		void setValue(T value, int index)						{ assert(index < mValues.size()); mValues[index] = value; }

		/**
		 * Resize based on shader declaration.
		 */
		virtual void setDefault() override						{ mValues.resize(mDeclaration->mNumElements, T()); }

		/**
		 * @return total number of elements in array
		 */
		int getNumElements() const								{ return static_cast<int>(mValues.size()); }

		/**
		 * @return entire array as a reference
		 */
		std::vector<T>& getValues()								{ return mValues; }

		/**
		 * Pushes the data to the 'Shader'
		 * @param uniformBuffer the buffer to copy the array into.
		 */
		virtual void push(uint8_t* uniformBuffer) const override;

		/**
		 * Array subscript operator, returns a specific value in the array as a reference,
		 * making the following possible: `mUniformArray[0] = 12`;
		 * @return a specific value in the array as a reference.
		 */
		T& operator[](size_t index) { assert(index < mValues.size()); return mValues[index]; }


	private:
		std::vector<T> mValues;
	};


	//////////////////////////////////////////////////////////////////////////
	// UniformOpaqueInstance
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Base class of all opaque uniform instances and uniform instance buffer types.
	 * Opaques cannot push data to the GPU.
	 */
	class NAPAPI UniformOpaqueInstance : public UniformInstance
	{
		RTTI_ENABLE(UniformInstance)
	};


	//////////////////////////////////////////////////////////////////////////
	// Uniform Value Buffer
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Base class of all uniform value array instances.
	 */
	class NAPAPI UniformValueBufferInstance : public UniformOpaqueInstance
	{
		RTTI_ENABLE(UniformOpaqueInstance)

	public:
		UniformValueBufferInstance(const UniformValueBufferDeclaration& declaration) :
			mDeclaration(&declaration) { }

		/**
		 * @return uniform declaration.
		 */
		virtual const UniformDeclaration& getDeclaration() const override { return *mDeclaration; }

	protected:
		const UniformValueBufferDeclaration* mDeclaration;
	};


	/**
	 * Specific type of uniform value buffer instance
	 * All supported types are defined below for easier readability.
	 */
	template<typename T>
	class TypedUniformValueBufferInstance : public UniformValueBufferInstance
	{
		RTTI_ENABLE(UniformValueBufferInstance)

	public:
		TypedUniformValueBufferInstance(const UniformValueBufferDeclaration& declaration) :
			UniformValueBufferInstance(declaration) { }

		/**
		 * Updates the uniform value from a resource, data is not pushed immediately.
		 * @param resource resource to copy data from.
		 */
		void set(const TypedUniformValueBuffer<T>& resource) { mBuffer = resource.mBuffer; }

		/**
		 * @return total number of elements in array
		 */
		int getNumElements() const { return mBuffer->mCount; }

	private:
		rtti::ObjectPtr<StorageBuffer<T>> mBuffer;
	};


	//////////////////////////////////////////////////////////////////////////
	// Type definitions for all supported uniform instance value types
	//////////////////////////////////////////////////////////////////////////

	using UniformIntInstance	= TypedUniformValueInstance<int>;
	using UniformFloatInstance	= TypedUniformValueInstance<float>;
	using UniformVec2Instance	= TypedUniformValueInstance<glm::vec2>;
	using UniformVec3Instance	= TypedUniformValueInstance<glm::vec3>;
	using UniformVec4Instance	= TypedUniformValueInstance<glm::vec4>;
	using UniformMat4Instance	= TypedUniformValueInstance<glm::mat4>;


	//////////////////////////////////////////////////////////////////////////
	// Type definitions for all supported uniform instance array value types
	//////////////////////////////////////////////////////////////////////////

	using UniformIntArrayInstance	= TypedUniformValueArrayInstance<int>;
	using UniformFloatArrayInstance = TypedUniformValueArrayInstance<float>;
	using UniformVec2ArrayInstance	= TypedUniformValueArrayInstance<glm::vec2>;
	using UniformVec3ArrayInstance	= TypedUniformValueArrayInstance<glm::vec3>;
	using UniformVec4ArrayInstance	= TypedUniformValueArrayInstance<glm::vec4>;
	using UniformMat4ArrayInstance	= TypedUniformValueArrayInstance<glm::mat4>;


	//////////////////////////////////////////////////////////////////////////
	// Type definitions for all supported uniform instance buffer value types
	//////////////////////////////////////////////////////////////////////////

	using UniformIntBufferInstance = TypedUniformValueBufferInstance<int>;
	using UniformFloatBufferInstance = TypedUniformValueBufferInstance<float>;
	using UniformVec2BufferInstance = TypedUniformValueBufferInstance<glm::vec2>;
	using UniformVec3BufferInstance = TypedUniformValueBufferInstance<glm::vec3>;
	using UniformVec4BufferInstance = TypedUniformValueBufferInstance<glm::vec4>;
	using UniformMat4BufferInstance = TypedUniformValueBufferInstance<glm::mat4>;


	//////////////////////////////////////////////////////////////////////////
	// Template definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename T>
	T* nap::UniformStructInstance::findUniform(const std::string& name)
	{
		UniformInstance* instance = findUniform(name);
		if (instance != nullptr)
			return rtti_cast<T>(instance);
		return nullptr;
	}

	template<typename T>
	T* nap::UniformStructInstance::getOrCreateUniform(const std::string& name)
	{
		// First try to find it, if found cast and return
		UniformInstance* instance = findUniform(name);
		if (instance != nullptr)
		{
			assert(instance->get_type().is_derived_from<T>());
			return rtti_cast<T>(instance);
		}

		// Otherwise fetch the declaration and use it to create the new instance
		const UniformDeclaration* declaration = mDeclaration.findMember(name);
		if (declaration == nullptr)
			return nullptr;

		std::unique_ptr<UniformInstance> new_instance = createUniformFromDeclaration(*declaration, mUniformCreatedCallback);
		T* result = rtti_cast<T>(new_instance.get());
		assert(result != nullptr);
		mUniforms.emplace_back(std::move(new_instance));

		// Notify listeners
		if (mUniformCreatedCallback)
			mUniformCreatedCallback();
		return result;
	}

	template<class T>
	void nap::TypedUniformValueInstance<T>::push(uint8_t* uniformBuffer) const
	{
		assert(sizeof(mValue) == mDeclaration->mSize);
		memcpy(uniformBuffer + mDeclaration->mOffset, &mValue, sizeof(mValue));
	}

	template<typename T>
	void nap::TypedUniformValueArrayInstance<T>::push(uint8_t* uniformBuffer) const
	{
		if (sizeof(T) == mDeclaration->mStride)
		{
			size_t size = mValues.size() * sizeof(T);
			memcpy(uniformBuffer + mDeclaration->mOffset, mValues.data(), size);
		}
		else
		{
			uint8_t* dest = uniformBuffer + mDeclaration->mOffset;
			for (const T& value : mValues)
			{
				memcpy(dest, &value, sizeof(value));
				dest += mDeclaration->mStride;
			}
		}
	}
}

/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "storageuniform.h"
#include "gpuvaluebuffer.h"
#include "gpustructbuffer.h"

// External Includes
#include <rtti/objectptr.h>
#include <glm/glm.hpp>
#include <utility/dllexport.h>
#include <nap/resource.h>

namespace nap
{
	// Forward Declares
	class StorageUniformInstance;
	class StorageUniformValueBufferInstance;
	class StorageUniformStructBufferInstance;

	using StorageUniformChangedCallback = std::function<void()>;

	// Called when the bound buffer resource changes
	using StorageUniformValueBufferChangedCallback = std::function<void(StorageUniformValueBufferInstance&)>;
	using StorageUniformStructBufferChangedCallback = std::function<void(StorageUniformStructBufferInstance&)>;

	/**
	 * Instantiated version of a nap::StorageUniform.
	 * Every uniform 'resource' has an associative 'instance', ie: nap::StorageUniformValueBuffer -> nap::StorageUniformValueBufferInstance.
	 * An instance can be updated / inspected at run-time and is associated with a declaration.
	 */
	class NAPAPI StorageUniformInstance
	{
		RTTI_ENABLE()

	public:
		// Default Destructor
		virtual ~StorageUniformInstance() = default;

		/**
		 * Required virtual, needs to be implemented in derived classes
		 * @return the declaration associated with this uniform instance
		 */
		virtual const UniformDeclaration& getDeclaration() const = 0;
	};


	//////////////////////////////////////////////////////////////////////////
	// StorageUniformStructInstance
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Contains other uniform instances, including: values, structs and arrays. 
	 * A uniform value, struct or array must be declared as part of a uniform struct.
	 */
	class NAPAPI StorageUniformStructInstance : public StorageUniformInstance
	{
		RTTI_ENABLE(StorageUniformInstance)

	public:

		// Constructor
		StorageUniformStructInstance(const UniformStructDeclaration& declaration, const StorageUniformChangedCallback& storageUniformChangedCallback) :
			mStorageUniformChangedCallback(storageUniformChangedCallback),
			mDeclaration(declaration)
		{ }

		// Delete copy assignment operator and copy constructor
		StorageUniformStructInstance(const StorageUniformStructInstance&) = delete;
		StorageUniformStructInstance& operator=(const StorageUniformStructInstance&) = delete;

		/**
		 * @return all uniform instances contained by this struct.
		 */
		const std::vector<std::unique_ptr<StorageUniformInstance>>& getUniforms() const	{ return mUniforms; }

		/**
		 * Tries to find a uniform with the given name.
		 * @param name the name of the uniform to find.
		 * @return found uniform instance, nullptr if it does not exist.
		 */
		StorageUniformInstance* findStorageUniform(const std::string& name);

		/**
		 * Tries to find a uniform of a specific type with the given name.
		 * @param name the name of the uniform to find.
		 * @return the uniform instance, nullptr if it does not exist
		 */
		template<typename T>
		T* findStorageUniform(const std::string& name);

		/**
		 * Tries to find a uniform of a specific type with the given name, creates it if it does not exist.
		 * @param name the name of the uniform to find or create
		 * @return the uniform instance, nullptr if it can't be found and created.
		 */
		template<typename T>
		T* getOrCreateStorageUniform(const std::string& name);

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
		 * Adds all associated uniforms to this instance, based on the struct declaration and resource.
		 */
		bool addStorageUniform(const UniformStructDeclaration& structDeclaration, const StorageUniformStruct* structResource, const StorageUniformChangedCallback& uniformChangedCallback, utility::ErrorState& errorState);

		/**
		 * Creates a uniform instance from a uniform declaration. 
		 * @param declaration the uniform declaration
		 * @param uniformCreatedCallback callback that is triggered when the uniform is created.
		 */
		static std::unique_ptr<StorageUniformInstance> createStorageUniformFromDeclaration(const UniformDeclaration& declaration, const StorageUniformChangedCallback& uniformChangedCallback);

	private:
		StorageUniformChangedCallback							mStorageUniformChangedCallback;
		const UniformStructDeclaration&							mDeclaration;
		std::vector<std::unique_ptr<StorageUniformInstance>>	mUniforms;
	};


	//////////////////////////////////////////////////////////////////////////
	// StorageUniformBufferInstance
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Represents a list of uniform struct instances.
	 */
	class NAPAPI StorageUniformBufferInstance : public StorageUniformInstance
	{
		RTTI_ENABLE(StorageUniformInstance)
	public:
		/**
		 * @return if the value buffer is set
		 */
		virtual bool hasBuffer() const = 0;
	};


	//////////////////////////////////////////////////////////////////////////
	// StorageUniformStructBufferInstance
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Represents a uniform data buffer instance for struct elements
	 */
	class NAPAPI StorageUniformStructBufferInstance : public StorageUniformBufferInstance
	{
		friend class StorageUniformStructInstance;
		RTTI_ENABLE(StorageUniformBufferInstance)
	public:
		// Constructor
		StorageUniformStructBufferInstance(const UniformStructArrayDeclaration& declaration) :
			mDeclaration(&declaration)
		{ }

		// Copy construction and copy assignment not allowed
		StorageUniformStructBufferInstance(const StorageUniformStructBufferInstance&) = delete;
		StorageUniformStructBufferInstance& operator=(const StorageUniformStructBufferInstance&) = delete;

		/**
		 * Updates the uniform value from a resource, data is not pushed immediately.
		 * @param resource resource to copy data from.
		 */
		void set(const GPUStructBuffer& resource)							{ assert(false); /*mBuffer = resource.mBuffer;*/ }

		/**
		 * Binds a new buffer to the uniform instance
		 * @param buffer new buffer to bind
		 */
		void setStructBuffer(GPUStructBuffer& buffer)
		{
			assert(buffer.getSize() == mDeclaration->mSize);
			mBuffer = &buffer;
			raiseChanged();
		}

		/**
		 * @return declaration used to create this instance. 
		 */
		virtual const UniformDeclaration& getDeclaration() const override	{ return *mDeclaration; }

		/**
		 * @return value buffer
		 */
		virtual const GPUStructBuffer& getStructBuffer() const				{ assert(mBuffer != nullptr); return *mBuffer; };

		/**
		 * @return if the value buffer is set
		 */
		virtual bool hasBuffer() const										{ return mBuffer != nullptr; }

	protected:
		/**
		 * Called when the buffer changes
		 */
		void raiseChanged()													{ if (mStructBufferChangedCallback) mStructBufferChangedCallback(*this); }

	private:
		const UniformStructArrayDeclaration* mDeclaration;
		rtti::ObjectPtr<GPUStructBuffer> mBuffer;

		mutable StorageUniformStructBufferChangedCallback mStructBufferChangedCallback;
	};


	//////////////////////////////////////////////////////////////////////////
	// UniformValueBufferInstance
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Base class of all uniform value array instances.
	 */
	class NAPAPI StorageUniformValueBufferInstance : public StorageUniformBufferInstance
	{
		RTTI_ENABLE(StorageUniformBufferInstance)

	public:
		StorageUniformValueBufferInstance(const UniformValueArrayDeclaration& declaration) :
			mDeclaration(&declaration) { }

		/**
		 * @return uniform declaration.
		 */
		virtual const UniformDeclaration& getDeclaration() const override	{ return *mDeclaration; }

		/**
		 * @return value buffer
		 */
		virtual const GPUValueBuffer& getValueBuffer() const = 0;

		/**
		 * @return if the value buffer is set
		 */
		virtual bool hasBuffer() const = 0;

		/**
		 * TODO: Too specific. Handle instances should be decoupled from uniforms. New descriptor type StorageUniform
		 */
		void setValueBufferChangedCallback(const StorageUniformValueBufferChangedCallback& valueBufferChangedCallback) const { mValueBufferChangedCallback = valueBufferChangedCallback; }

	protected:
		/**
		 * Called when the buffer changes
		 */
		void raiseChanged()													{ if (mValueBufferChangedCallback) mValueBufferChangedCallback(*this); }

		const UniformValueArrayDeclaration* mDeclaration;

	private:
		mutable StorageUniformValueBufferChangedCallback mValueBufferChangedCallback;
	};


	/**
	 * Specific type of uniform value buffer instance
	 * All supported types are defined below for easier readability.
	 */
	template<typename T>
	class TypedStorageUniformValueBufferInstance : public StorageUniformValueBufferInstance
	{
		RTTI_ENABLE(StorageUniformValueBufferInstance)

	public:
		TypedStorageUniformValueBufferInstance(const UniformValueArrayDeclaration& declaration) :
			StorageUniformValueBufferInstance(declaration) { }

		/**
		 * Updates the uniform value from a resource, data is not pushed immediately.
		 * @param resource resource to copy data from.
		 */
		void set(const TypedStorageUniformValueBuffer<T>& resource)			{ mBuffer = resource.mBuffer; }

		 /**
		  * Binds a new buffer to the uniform instance
		  * @param buffer new buffer to bind
		  */
		void setValueBuffer(TypedGPUValueBuffer<T>& buffer);

		/**
		 * @return buffer
		 */
		const TypedGPUValueBuffer<T>& getTypedValueBuffer() const			{ assert(mBuffer != nullptr); return *mBuffer; }

		/**
		 * @return value buffer
		 */
		virtual const GPUValueBuffer& getValueBuffer() const override		{ assert(mBuffer != nullptr); return *mBuffer; }

		/**
		 * @return if the value buffer is set
		 */
		virtual bool hasBuffer() const override								{ return mBuffer != nullptr; }

	private:
		rtti::ObjectPtr<TypedGPUValueBuffer<T>> mBuffer;
	};


	//////////////////////////////////////////////////////////////////////////
	// Type definitions for all supported storage uniform value buffer instance types
	//////////////////////////////////////////////////////////////////////////

	using StorageUniformIntBufferInstance	= TypedStorageUniformValueBufferInstance<int>;
	using StorageUniformFloatBufferInstance	= TypedStorageUniformValueBufferInstance<float>;
	using StorageUniformVec2BufferInstance	= TypedStorageUniformValueBufferInstance<glm::vec2>;
	using StorageUniformVec3BufferInstance	= TypedStorageUniformValueBufferInstance<glm::vec3>;
	using StorageUniformVec4BufferInstance	= TypedStorageUniformValueBufferInstance<glm::vec4>;
	using StorageUniformMat4BufferInstance	= TypedStorageUniformValueBufferInstance<glm::mat4>;


	//////////////////////////////////////////////////////////////////////////
	// Template definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename T>
	T* nap::StorageUniformStructInstance::findStorageUniform(const std::string& name)
	{
		StorageUniformInstance* instance = findStorageUniform(name);
		if (instance != nullptr)
			return rtti_cast<T>(instance);
		return nullptr;
	}

	template<typename T>
	T* nap::StorageUniformStructInstance::getOrCreateStorageUniform(const std::string& name)
	{
		// First try to find it, if found cast and return
		StorageUniformInstance* instance = findStorageUniform(name);
		if (instance != nullptr)
		{
			assert(instance->get_type().is_derived_from<T>());
			return rtti_cast<T>(instance);
		}

		// Otherwise fetch the declaration and use it to create the new instance
		const UniformDeclaration* declaration = mDeclaration.findMember(name);
		if (declaration == nullptr)
			return nullptr;

		std::unique_ptr<StorageUniformInstance> new_instance = createStorageUniformFromDeclaration(*declaration, mStorageUniformChangedCallback);
		T* result = rtti_cast<T>(new_instance.get());
		assert(result != nullptr);
		mUniforms.emplace_back(std::move(new_instance));

		// Notify listeners
		//if (mUniformCreatedCallback)
		//	mUniformCreatedCallback();
		return result;
	}

	template<class T>
	void nap::TypedStorageUniformValueBufferInstance<T>::setValueBuffer(TypedGPUValueBuffer<T>& buffer)
	{
		assert(buffer.mSize == mDeclaration->mSize);
		mBuffer = &buffer;
		raiseChanged();
	}
}

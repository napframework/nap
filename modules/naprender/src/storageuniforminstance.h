/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "storageuniform.h"
#include "valuegpubuffer.h"
#include "structgpubuffer.h"

// External Includes
#include <rtti/objectptr.h>
#include <glm/glm.hpp>
#include <utility/dllexport.h>
#include <nap/resource.h>

namespace nap
{
	// Forward Declares
	class StorageUniformInstance;
	class StorageUniformBufferInstance;
	class StorageUniformValueBufferInstance;
	class StorageUniformStructBufferInstance;

	// Called when the bound buffer resource changes
	using StorageUniformBufferChangedCallback = std::function<void(StorageUniformBufferInstance&)>;

	 /**
	  * Instantiated version of a nap::StorageUniform.
	  * Every uniform 'resource' has an associative 'instance', ie: nap::StorageUniformValueBuffer ->
	  * nap::StorageUniformValueBufferInstance.
	  * An instance can be updated / inspected at run-time and is associated with a declaration.
	  * 
	  * Unlike standard uniforms, storage uniforms store a reference to the underlying data as opposed to the data itself.
	  * This allows for any compute shader to read from and write to the same data storage. Storage uniforms currently
	  * always refer to a single nap::GPUBuffer, whether this is simple a `nap::ValueGPUBuffer` or a more complex
	  * `nap::StructGPUBuffer`.
	  *
	  * A single vec4 array can be addressed as a `nap::Vec4GPUValueBuffer`:
	  *~~~~~{.comp}
	  *	layout(std430) buffer PositionSSBO
	  *	{
	  *		vec4 positions[100000];
	  *	} pos_ssbo;
	  *
	  *	layout(std430) buffer NormalSSBO
	  *	{
	  *		vec4 normals[100000];
	  *	} norm_ssbo;
	  *~~~~~
	  *
	  * If you intend to pack some data types together, you can do so with a `nap::StructGPUBuffer`:
	  *~~~~~{.comp}
	  *	struct Item
	  *	{
	  *		vec4 position;
	  *		vec4 normal;
	  *	};
	  *
	  * 	layout(std430) buffer ItemSSBO
	  *	{
	  *		Item items[100000];
	  *	} item_ssbo;
	  *~~~~~
	  *
	  * Declaring multiple shader variables outside of a struct is currently not supported:
	  *~~~~~{.comp}
	  *	// ERROR
	  *	layout(std430) buffer ExampleComputeBuffer
	  *	{
	  *		vec4 positions[100000];
	  *		vec4 normals[100000];
	  *	};
	  *~~~~~
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
		virtual const ShaderVariableDeclaration& getDeclaration() const = 0;
	};


	//////////////////////////////////////////////////////////////////////////
	// StorageUniformStructInstance
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Instance of a Storage Uniform Buffer container.
	 * 
	 * Stores a single StorageUniformBuffer reference as opposed to a UniformStruct, which also supports multiple
	 * and nested shader variables.
	 *
	 * The reason for restricting StorageUniformStruct to a single buffer variable is that we want to associate a
	 * shader resource binding point with single shader storage buffer. This is a typical use case for storage
	 * buffers and simplifies overall resource management.
	 *
	 *~~~~~{.comp}
	 *	layout(std430) buffer PositionSSBO
	 *	{
	 *		vec4 positions[100000];
	 *	} pos_ssbo;
	 *~~~~~
	 */
	class NAPAPI StorageUniformStructInstance : public StorageUniformInstance
	{
		RTTI_ENABLE(StorageUniformInstance)

	public:

		// Constructor
		StorageUniformStructInstance(const ShaderVariableStructDeclaration& declaration, const StorageUniformCreatedCallback& storageUniformCreatedCallback) :
			mStorageUniformCreatedCallback(storageUniformCreatedCallback),
			mDeclaration(declaration)
		{ }

		// Copy construction and copy assignment not allowed
		StorageUniformStructInstance(const StorageUniformStructInstance&) = delete;
		StorageUniformStructInstance& operator=(const StorageUniformStructInstance&) = delete;

		/**
		 * @return all uniform instances contained by this struct.
		 */
		const std::unique_ptr<StorageUniformBufferInstance>& getStorageUniformBuffer() const				{ return mStorageUniformBuffer; }

		/**
		 * Tries to find a uniform with the given name.
		 * @param name the name of the uniform to find.
		 * @return found uniform instance, nullptr if it does not exist.
		 */
		StorageUniformBufferInstance* findStorageUniformBuffer(const std::string& name);

		/**
		 * Tries to find a uniform of a specific type with the given name.
		 * @param name the name of the uniform to find.
		 * @return the uniform instance, nullptr if it does not exist
		 */
		template<typename T>
		T* findStorageUniformBuffer(const std::string& name);

		/**
		 * Tries to find a uniform of a specific type with the given name, creates it if it does not exist.
		 * @param name the name of the uniform to find or create
		 * @return the uniform instance, nullptr if it can't be found and created.
		 */
		template<typename T>
		T* getOrCreateStorageUniformBuffer(const std::string& name);

		/**
		 * @return the uniform declaration, used to create the uniform instance.
		 */
		virtual const ShaderVariableDeclaration& getDeclaration() const override							{ return mDeclaration; }

	private:
		friend class BaseMaterial;
		friend class ComputeMaterial;
		friend class Material;
		friend class BaseMaterialInstance;
		friend class MaterialInstance;

		/**
		 * Sets associated uniform buffer to this instance, based on the struct declaration and resource.
		 */
		bool addStorageUniformBuffer(const ShaderVariableStructDeclaration& structDeclaration, const StorageUniformStruct* structResource, const StorageUniformBufferChangedCallback& bufferChangedCallback, utility::ErrorState& errorState);

		/**
		 * Creates a uniform buffer instance from a uniform declaration. 
		 * @param declaration the uniform declaration
		 */
		static std::unique_ptr<StorageUniformBufferInstance> createStorageUniformBufferFromDeclaration(const ShaderVariableDeclaration& declaration, const StorageUniformCreatedCallback& storageUniformCreatedCallback);

	private:
		StorageUniformCreatedCallback								mStorageUniformCreatedCallback;

		const ShaderVariableStructDeclaration&						mDeclaration;
		std::unique_ptr<StorageUniformBufferInstance>				mStorageUniformBuffer;
	};


	//////////////////////////////////////////////////////////////////////////
	// StorageUniformBufferInstance
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Storage uniform buffer instance base class.
	 *
	 * A StorageUniformBufferInstance must be declared as part of a StorageUniformStructInstance.
	 */
	class NAPAPI StorageUniformBufferInstance : public StorageUniformInstance
	{
		RTTI_ENABLE(StorageUniformInstance)
	public:
		// Constructor
		StorageUniformBufferInstance() { }

		StorageUniformBufferInstance(const StorageUniformBufferChangedCallback& bufferChangedCallback) :
			mBufferChangedCallback(bufferChangedCallback) { }

		/**
		 * @return if the value buffer is set
		 */
		virtual bool hasBuffer() const = 0;

		/**
		 * 
		 */
		void setBufferChangedCallback(const StorageUniformBufferChangedCallback& bufferChangedCallback) const { mBufferChangedCallback = bufferChangedCallback; }

	protected:
		/**
		 * Called when the buffer changes
		 */
		void raiseChanged() { if (mBufferChangedCallback) mBufferChangedCallback(*this); }

		mutable StorageUniformBufferChangedCallback					mBufferChangedCallback;
	};


	//////////////////////////////////////////////////////////////////////////
	// StorageUniformStructBufferInstance
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Represents an instance of a storage uniform struct buffer, for example:
	 * StructGPUBuffer -> StorageUniformStructBuffer.
	 *
	 * A StorageUniformStructBufferInstance must be declared as part of a StorageUniformStructInstance.
	 */
	class NAPAPI StorageUniformStructBufferInstance : public StorageUniformBufferInstance
	{
		friend class StorageUniformStructInstance;
		RTTI_ENABLE(StorageUniformBufferInstance)
	public:
		// Constructor
		StorageUniformStructBufferInstance(const ShaderVariableStructBufferDeclaration& declaration) :
			mDeclaration(&declaration)
		{ }

		StorageUniformStructBufferInstance(const ShaderVariableStructBufferDeclaration& declaration, const StorageUniformBufferChangedCallback& bufferChangedCallback) :
			mDeclaration(&declaration), StorageUniformBufferInstance(bufferChangedCallback)
		{ }

		// Copy construction and copy assignment not allowed
		StorageUniformStructBufferInstance(const StorageUniformStructBufferInstance&) = delete;
		StorageUniformStructBufferInstance& operator=(const StorageUniformStructBufferInstance&) = delete;

		/**
		 * Binds a new buffer to the uniform instance
		 * @param buffer new buffer to bind
		 */
		void setBuffer(StructGPUBuffer& buffer)
		{
			assert(buffer.getSize() == mDeclaration->mSize);
			mBuffer = &buffer;
			raiseChanged();
		}

		/**
		 * Updates the buffer from a resource
		 * @param resource resource to set buffer from.
		 */
		void setBuffer(const StorageUniformStructBuffer& resource)					{ mBuffer = resource.mBuffer; }

		/**
		 * @return declaration used to create this instance. 
		 */
		virtual const ShaderVariableDeclaration& getDeclaration() const override	{ return *mDeclaration; }

		/**
		 * @return if the value buffer is set
		 */
		virtual bool hasBuffer() const override										{ return mBuffer != nullptr; }

		/**
		 * @return value buffer
		 */
		virtual const StructGPUBuffer& getBuffer() const							{ assert(mBuffer != nullptr); return *mBuffer; };

		/**
		 * @return value buffer
		 */
		virtual StructGPUBuffer& getBuffer()										{ assert(mBuffer != nullptr); return *mBuffer; };

	private:
		const ShaderVariableStructBufferDeclaration* mDeclaration;
		rtti::ObjectPtr<StructGPUBuffer> mBuffer;
	};


	//////////////////////////////////////////////////////////////////////////
	// UniformValueBufferInstance
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Base class of all typed storage uniform value buffer instances.
	 * 
	 * A StorageUniformValueBufferInstance must be declared as part of a StorageUniformStructInstance.
	 */
	class NAPAPI StorageUniformValueBufferInstance : public StorageUniformBufferInstance
	{
		RTTI_ENABLE(StorageUniformBufferInstance)
	public:
		// Constructor
		StorageUniformValueBufferInstance(const ShaderVariableValueArrayDeclaration& declaration) :
			mDeclaration(&declaration) { }

		StorageUniformValueBufferInstance(const ShaderVariableValueArrayDeclaration& declaration, const StorageUniformBufferChangedCallback& bufferChangedCallback) :
			mDeclaration(&declaration), StorageUniformBufferInstance(bufferChangedCallback) { }

		/**
		 * @return uniform declaration.
		 */
		virtual const ShaderVariableDeclaration& getDeclaration() const override	{ return *mDeclaration; }

		/**
		 * @return value buffer
		 */
		virtual const ValueGPUBuffer& getBuffer() const = 0;

		/**
		 * @return value buffer
		 */
		virtual ValueGPUBuffer& getBuffer() = 0;

		/**
		 * @return if the value buffer is set
		 */
		virtual bool hasBuffer() const = 0;

	protected:
		const ShaderVariableValueArrayDeclaration* mDeclaration;
	};


	/**
	 * Specific type of storage uniform value buffer instance, for example:
	 * TypedValueGPUBuffer<float> -> TypedStorageUniformValueBufferInstance<float>.
	 * All supported types are defined below for easier readability.
	 *
	 * A StorageUniformValueBufferInstance must be declared as part of a StorageUniformStructInstance.
	 */
	template<typename T>
	class TypedStorageUniformValueBufferInstance : public StorageUniformValueBufferInstance
	{
		RTTI_ENABLE(StorageUniformValueBufferInstance)

	public:
		// Constructor
		TypedStorageUniformValueBufferInstance(const ShaderVariableValueArrayDeclaration& declaration) :
			StorageUniformValueBufferInstance(declaration) { }

		TypedStorageUniformValueBufferInstance(const ShaderVariableValueArrayDeclaration& declaration, const StorageUniformBufferChangedCallback& bufferChangedCallback) :
			StorageUniformValueBufferInstance(declaration, bufferChangedCallback) { }

		 /**
		  * Binds a new buffer to the uniform instance
		  * @param buffer new buffer to bind
		  */
		void setBuffer(TypedValueGPUBuffer<T>& buffer);

		/**
		 * Updates thebuffer from a resource.
		 * @param resource resource to set buffer from.
		 */
		void setBuffer(const TypedStorageUniformValueBuffer<T>& resource)	{ mBuffer = resource.mBuffer; }

		/**
		 * @return buffer
		 */
		const TypedValueGPUBuffer<T>& getTypedBuffer() const						{ assert(mBuffer != nullptr); return *mBuffer; }

		/**
		 * @return buffer
		 */
		TypedValueGPUBuffer<T>& getTypedBuffer()									{ assert(mBuffer != nullptr); return *mBuffer; }

		/**
		 * @return value buffer
		 */
		virtual const ValueGPUBuffer& getBuffer() const override					{ assert(mBuffer != nullptr); return *mBuffer; }

		/**
		 * @return value buffer
		 */
		virtual ValueGPUBuffer& getBuffer() override								{ assert(mBuffer != nullptr); return *mBuffer; }

		/**
		 * @return if the value buffer is set
		 */
		virtual bool hasBuffer() const override										{ return mBuffer != nullptr; }

	private:
		rtti::ObjectPtr<TypedValueGPUBuffer<T>> mBuffer;
	};


	//////////////////////////////////////////////////////////////////////////
	// Type definitions for all supported storage uniform value buffer instance types
	//////////////////////////////////////////////////////////////////////////

	using StorageUniformUIntBufferInstance	= TypedStorageUniformValueBufferInstance<uint>;
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
	T* nap::StorageUniformStructInstance::findStorageUniformBuffer(const std::string& name)
	{
		StorageUniformBufferInstance* instance = findStorageUniformBuffer(name);
		if (instance != nullptr)
			return rtti_cast<T>(instance);
		return nullptr;
	}

	template<typename T>
	T* nap::StorageUniformStructInstance::getOrCreateStorageUniformBuffer(const std::string& name)
	{
		// First try to find it, if found cast and return
		StorageUniformBufferInstance* instance = findStorageUniformBuffer(name);
		if (instance != nullptr)
		{
			assert(instance->get_type().is_derived_from<T>());
			return rtti_cast<T>(instance);
		}

		// Otherwise fetch the declaration and use it to create the new instance
		const ShaderVariableDeclaration* declaration = mDeclaration.findMember(name);
		if (declaration == nullptr)
			return nullptr;

		std::unique_ptr<StorageUniformBufferInstance> new_instance = createStorageUniformBufferFromDeclaration(*declaration, mStorageUniformCreatedCallback);
		T* result = rtti_cast<T>(new_instance.get());
		assert(result != nullptr);
		mStorageUniformBuffer = std::move(new_instance);

		// Notify listeners
		if (mStorageUniformCreatedCallback)
			mStorageUniformCreatedCallback();
		return result;
	}

	template<class T>
	void nap::TypedStorageUniformValueBufferInstance<T>::setBuffer(TypedValueGPUBuffer<T>& buffer)
	{
		assert(buffer.getSize() == mDeclaration->mSize);
		mBuffer = &buffer;
		raiseChanged();
	}
}

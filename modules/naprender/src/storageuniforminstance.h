/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "storageuniform.h"
#include "gpubuffer.h"
#include "structgpubuffer.h"

// External Includes
#include <rtti/objectptr.h>
#include <glm/glm.hpp>
#include <utility/dllexport.h>
#include <nap/resource.h>

namespace nap
{
	// Forward Declares
	class BufferBindingInstance;

	// Called when the bound buffer resource changes
	using BufferBindingChangedCallback = std::function<void(BufferBindingInstance&)>;

	/**
	 * Instantiated version of a nap::BufferBinding.
	 * Every uniform 'resource' has an associative 'instance', ie: nap::BufferBindingNumeric->
	 * nap::BufferBindingNumericInstance.
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
	class NAPAPI BufferBindingInstance
	{
		RTTI_ENABLE()
	public:
		// Constructor
		BufferBindingInstance(const BufferBindingChangedCallback& bindingChangedCallback) :
			mBindingChangedCallback(bindingChangedCallback)
		{ }

		// Default Destructor
		virtual ~BufferBindingInstance() = default;

		/**
		 * Required virtual, needs to be implemented in derived classes
		 * @return the declaration associated with this uniform instance
		 */
		virtual const ShaderVariableDeclaration& getDeclaration() const = 0;

		/**
		 * @return if the buffer is set
		 */
		virtual bool hasBuffer() const = 0;

		/**
		 * @return base buffer
		 */
		virtual const BaseGPUBuffer& getBaseBuffer() const = 0;

	protected:
		BufferBindingChangedCallback mBindingChangedCallback;

	private:
		friend class BaseMaterial;
		friend class BaseMaterialInstance;

		/**
		 * Creates a uniform buffer instance from a uniform declaration.
		 * @param declaration the uniform declaration
		 * @param binding the binding resource to create the instance from. Is allowed to be nullptr.
		 */
		static std::unique_ptr<BufferBindingInstance> createBufferBindingInstanceFromDeclaration(const ShaderVariableDeclaration& declaration, const BufferBinding* binding, BufferBindingChangedCallback bufferChangedCallback, utility::ErrorState& errorState);
	};


	//////////////////////////////////////////////////////////////////////////
	// BufferBindingStructInstance
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Instance of a Storage Uniform Buffer container.
	 * 
	 * Stores a single BufferBinding reference as opposed to a UniformStruct, which also supports multiple
	 * and nested shader variables.
	 *
	 * The reason for restricting BufferBindingStruct to a single buffer variable is that we want to associate a
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
	class NAPAPI BufferBindingStructInstance final : public BufferBindingInstance
	{
		RTTI_ENABLE(BufferBindingInstance)

	public:
		// Constructor
		BufferBindingStructInstance(const ShaderVariableStructDeclaration& declaration, const BufferBindingChangedCallback& bindingChangedCallback) :
			BufferBindingInstance(bindingChangedCallback),
			mDeclaration(&declaration)
		{ }

		// Copy construction and copy assignment not allowed
		BufferBindingStructInstance(const BufferBindingStructInstance&) = delete;
		BufferBindingStructInstance& operator=(const BufferBindingStructInstance&) = delete;

		/**
		 * Required virtual, needs to be implemented in derived classes
		 * @return the declaration associated with this uniform instance
		 */
		virtual const ShaderVariableDeclaration& getDeclaration() const	override	{ return *mDeclaration; }

		/**
		 * @return if the struct buffer is set
		 */
		virtual bool hasBuffer() const override										{ return mBuffer != nullptr; }

		/**
		 * @return base buffer
		 */
		virtual const BaseGPUBuffer& getBaseBuffer() const override					{ assert(hasBuffer()); return *mBuffer; }

		/**
		 * @return the uniform declaration, used to create the uniform instance.
		 */
		const ShaderVariableStructDeclaration& getStructDeclaration() const			{ return *mDeclaration; }

		/**
		 * @return value buffer
		 */
		virtual const StructGPUBuffer& getBuffer() const							{ assert(hasBuffer()); return *mBuffer; };

		/**
		 * @return value buffer
		 */
		virtual StructGPUBuffer& getBuffer()										{ assert(hasBuffer()); return *mBuffer; };

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
		void setBuffer(const BufferBindingStruct& resource)							{ mBuffer = resource.mBuffer; }

	private:
		friend class BaseMaterial;
		friend class BaseMaterialInstance;

		/**
		 * Called when the buffer changes
		 */
		void raiseChanged()															{ if (mBindingChangedCallback) mBindingChangedCallback(*this); }

		/**
		 * Sets associated uniform buffer to this instance, based on the struct declaration and resource.
		 */
		//bool addBufferBinding(const ShaderVariableStructDeclaration& structDeclaration, const BufferBindingStruct* structResource, const BufferBindingChangedCallback& bufferChangedCallback, utility::ErrorState& errorState);

		const ShaderVariableStructDeclaration*				mDeclaration;
		rtti::ObjectPtr<StructGPUBuffer>					mBuffer;
	};


	//////////////////////////////////////////////////////////////////////////
	// UniformValueBufferInstance
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Base class of all typed storage uniform value buffer instances.
	 * 
	 * A BufferBindingNumericInstance must be declared as part of a BufferBindingStructInstance.
	 */
	class NAPAPI BufferBindingNumericInstance : public BufferBindingInstance
	{
		RTTI_ENABLE(BufferBindingInstance)
	public:
		// Constructor
		BufferBindingNumericInstance(const ShaderVariableValueArrayDeclaration& declaration, const BufferBindingChangedCallback& bindingChangedCallback) :
			BufferBindingInstance(bindingChangedCallback),
			mDeclaration(&declaration)
		{ }

		/**
		 * @return shader variable declaration.
		 */
		virtual const ShaderVariableDeclaration& getDeclaration() const override	{ return *mDeclaration; }

		/**
		 * @return shader variable declaration, used to create the buffer binding instance.
		 */
		const ShaderVariableValueArrayDeclaration& getNumericDeclaration() const	{ return *mDeclaration; }

		/**
		 * @return value buffer
		 */
		virtual const GPUBuffer& getBuffer() const = 0;

		/**
		 * @return value buffer
		 */
		virtual GPUBuffer& getBuffer() = 0;

		/**
		 * @return if the value buffer is set
		 */
		virtual bool hasBuffer() const = 0;

	protected:
		const ShaderVariableValueArrayDeclaration* mDeclaration;
		BufferBindingChangedCallback mBindingChangedCallback;
	};


	/**
	 * Specific type of storage uniform value buffer instance, for example:
	 * TypedValueGPUBuffer<float> -> TypedBufferBindingNumericInstance<float>.
	 * All supported types are defined below for easier readability.
	 *
	 * A BufferBindingNumericInstance must be declared as part of a BufferBindingStructInstance.
	 */
	template<typename T>
	class TypedBufferBindingNumericInstance final : public BufferBindingNumericInstance
	{
		RTTI_ENABLE(BufferBindingNumericInstance)
	public:
		// Constructor
		TypedBufferBindingNumericInstance(const ShaderVariableValueArrayDeclaration& declaration, const BufferBindingChangedCallback& bindingChangedCallback) :
			BufferBindingNumericInstance(&declaration, bindingChangedCallback)
		{ }

		/**
		 * @return base buffer
		 */
		virtual const BaseGPUBuffer& getBaseBuffer() const override					{ assert(hasBuffer()); return *mBuffer; }

		/**
		 * @return value buffer
		 */
		virtual const GPUBuffer& getBuffer() const override							{ assert(hasBuffer()); return *mBuffer; }

		/**
		 * @return value buffer
		 */
		virtual GPUBuffer& getBuffer() override										{ assert(hasBuffer()); return *mBuffer; }

		/**
		 * @return if the value buffer is set
		 */
		virtual bool hasBuffer() const override										{ return mBuffer != nullptr; }

		/**
		 * Binds a new buffer to the uniform instance
		 * @param buffer new buffer to bind
		 */
		void setBuffer(GPUBufferNumeric<T>& buffer);

		/**
		 * Updates thebuffer from a resource.
		 * @param resource resource to set buffer from.
		 */
		void setBuffer(const TypedBufferBindingNumeric<T>& resource)				{ mBuffer = resource.mBuffer; }

		/**
		 * @return buffer
		 */
		const GPUBufferNumeric<T>& getTypedBuffer() const							{ assert(mBuffer != nullptr); return *mBuffer; }

		/**
		 * @return buffer
		 */
		GPUBufferNumeric<T>& getTypedBuffer()										{ assert(mBuffer != nullptr); return *mBuffer; }

	private:
		/**
		 * Called when the buffer changes
		 */
		void raiseChanged()															{ if (mBindingChangedCallback) mBindingChangedCallback(*this); }

		rtti::ObjectPtr<GPUBufferNumeric<T>> mBuffer;
	};


	//////////////////////////////////////////////////////////////////////////
	// Type definitions for all supported storage uniform value buffer instance types
	//////////////////////////////////////////////////////////////////////////

	using BufferBindingUIntInstance		= TypedBufferBindingNumericInstance<uint>;
	using BufferBindingIntInstance		= TypedBufferBindingNumericInstance<int>;
	using BufferBindingFloatInstance	= TypedBufferBindingNumericInstance<float>;
	using BufferBindingVec2Instance		= TypedBufferBindingNumericInstance<glm::vec2>;
	using BufferBindingVec3Instance		= TypedBufferBindingNumericInstance<glm::vec3>;
	using BufferBindingVec4Instance		= TypedBufferBindingNumericInstance<glm::vec4>;
	using BufferBindingMat4Instance		= TypedBufferBindingNumericInstance<glm::mat4>;


	//////////////////////////////////////////////////////////////////////////
	// Template definitions
	//////////////////////////////////////////////////////////////////////////

	//template<typename T>
	//T* nap::BufferBindingStructInstance::findBufferBinding(const std::string& name)
	//{
	//	BufferBindingInstance* instance = findBufferBinding(name);
	//	if (instance != nullptr)
	//		return rtti_cast<T>(instance);
	//	return nullptr;
	//}

	//template<typename T>
	//T* nap::BufferBindingStructInstance::getOrCreateBufferBinding(const std::string& name)
	//{
	//	// First try to find it, if found cast and return
	//	BufferBindingInstance* instance = findBufferBinding(name);
	//	if (instance != nullptr)
	//	{
	//		assert(instance->get_type().is_derived_from<T>());
	//		return rtti_cast<T>(instance);
	//	}

	//	// Otherwise fetch the declaration and use it to create the new instance
	//	const ShaderVariableDeclaration* declaration = mDeclaration.findMember(name);
	//	if (declaration == nullptr)
	//		return nullptr;

	//	std::unique_ptr<BufferBindingInstance> new_instance = createBufferBindingFromDeclaration(*declaration, mBufferBindingCreatedCallback);
	//	T* result = rtti_cast<T>(new_instance.get());
	//	assert(result != nullptr);
	//	mBufferBinding = std::move(new_instance);

	//	// Notify listeners
	//	if (mBufferBindingCreatedCallback)
	//		mBufferBindingCreatedCallback();
	//	return result;
	//}

	template<class T>
	void nap::TypedBufferBindingNumericInstance<T>::setBuffer(GPUBufferNumeric<T>& buffer)
	{
		assert(buffer.getSize() == mDeclaration->mSize);
		mBuffer = &buffer;
		raiseChanged();
	}
}

/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "bufferbinding.h"
#include "gpubuffer.h"
#include "structbuffer.h"

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
	 * Instantiated version of nap::BufferBinding.
	 * Every buffer binding 'resource' has an associative 'instance', ie: nap::BufferBindingNumeric ->
	 * nap::BufferBindingNumericInstance.
	 * An instance can be updated / inspected at run-time and is associated with a declaration.
	 *
	 * Buffer bindings, unlike standard uniforms, store a reference to the underlying data as opposed to the data itself.
	 * This allows for any compute shader to read from and write to the same data storage. Buffer bindings always refer to
	 * a single nap::GPUBuffer, whether this is simple a `nap::GPUBufferNumeric` or a more complex `nap::StructGPUBuffer`.
	 *
	 * A single vec4 array can be addressed as a `nap::VertexBufferVec4`:
	 *~~~~~{.comp}
	 *	layout(std430) buffer PositionSSBO		//<- binding name
	 *	{
	 *		vec4 positions[100000];				//<- buffer declaration name
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
		BufferBindingInstance(const std::string& bindingName, const BufferBindingChangedCallback& bindingChangedCallback) :
			mBindingName(bindingName),
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
		bool hasBuffer() const								{ return mBuffer != nullptr; }

		/**
		 * @return base buffer
		 */
		const GPUBuffer& getBuffer() const					{ assert(mBuffer != nullptr); return *mBuffer; }

		/**
		 * @return base buffer
		 */
		GPUBuffer& getBuffer()								{ assert(mBuffer != nullptr); return *mBuffer; }

		/**
		 * @return binging name
		 */
		const std::string& getBindingName() const			{ return mBindingName; }

	protected:
		const std::string									mBindingName;
		BufferBindingChangedCallback						mBindingChangedCallback;
		GPUBuffer*											mBuffer = nullptr;

	private:
		friend class BaseMaterial;
		friend class BaseMaterialInstance;

		/**
		 * Creates a buffer binding instance from a uniform declaration and returns a unique ptr to if successful. Returns nullptr otherwise.
		 * @param declaration the shader variable declaration.
		 * @param binding the binding resource to create the instance from. Is allowed to be nullptr, in which case the instance will have no buffer.
		 * @param bindingChangedCallback callback function that is fired each time the binding instance is updated.
		 * @param errorState contains the error if the buffer binding creation fails.
		 * @return a unique ptr to the new buffer binding instance. nullptr if the operation has failed.
		 */
		static std::unique_ptr<BufferBindingInstance> createBufferBindingInstanceFromDeclaration(const BufferObjectDeclaration& declaration, const BufferBinding* binding, BufferBindingChangedCallback bindingChangedCallback, utility::ErrorState& errorState);
	};


	//////////////////////////////////////////////////////////////////////////
	// BufferBindingStructInstance
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Instantiated version of nap::BufferBindingStruct.
	 * 
	 * Every buffer binding 'resource' has an associative 'instance', ie: nap::BufferBindingNumeric ->
	 * nap::BufferBindingNumericInstance.
	 * An instance can be updated / inspected at run-time and is associated with a declaration.
	 *
	 * Buffer bindings, unlike standard uniforms, store a reference to the underlying data as opposed to the data itself.
	 * This allows for any compute shader to read from and write to the same data storage. Buffer bindings always refer to
	 * a single nap::GPUBuffer, whether this is simple a `nap::GPUBufferNumeric` or a more complex `nap::StructGPUBuffer`.
	 *
	 * A struct array can be addressed as a `nap::GPUStructBuffer`:
	 *~~~~~{.comp}
	 *	struct Particle
	 * 	{
	 *		vec4 position;
	 *		vec4 velocity;
	 *	}
	 * 
	 *	layout(std430) buffer ParticleSSBO
	 *	{
	 *		Particle particles[100000];
	 *	} pos_ssbo;
	 *~~~~~
	 */
	class NAPAPI BufferBindingStructInstance final : public BufferBindingInstance
	{
		RTTI_ENABLE(BufferBindingInstance)
	public:
		// Constructor
		BufferBindingStructInstance(const std::string& bindingName, const ShaderVariableStructBufferDeclaration& declaration, const BufferBindingChangedCallback& bindingChangedCallback) :
			BufferBindingInstance(bindingName, bindingChangedCallback),
			mDeclaration(&declaration)
		{ }

		// Copy construction and copy assignment not allowed
		BufferBindingStructInstance(const BufferBindingStructInstance&) = delete;
		BufferBindingStructInstance& operator=(const BufferBindingStructInstance&) = delete;

		/**
		 * Required virtual, needs to be implemented in derived classes
		 * @return the declaration associated with this uniform instance
		 */
		virtual const ShaderVariableDeclaration& getDeclaration() const	override		{ return *mDeclaration; }

		/**
		 * @return the uniform declaration, used to create the uniform instance.
		 */
		const ShaderVariableStructBufferDeclaration& getStructDeclaration() const		{ return *mDeclaration; }

		/**
		 * @return struct buffer
		 */
		const StructBuffer& getBuffer() const											{ assert(hasBuffer()); return static_cast<const StructBuffer&>(*mBuffer); };

		/**
		 * @return struct buffer
		 */
		StructBuffer& getBuffer()														{ assert(hasBuffer()); return static_cast<StructBuffer&>(*mBuffer); };

		/**
		 * Binds a new buffer to the uniform instance
		 * @param buffer new buffer to bind
		 */
		void setBuffer(StructBuffer& buffer)
		{
			assert(buffer.getSize() == mDeclaration->mSize);
			BufferBindingInstance::mBuffer = &buffer;
			raiseChanged();
		}

		/**
		 * Updates the buffer from a resource
		 * @param resource resource to set buffer from.
		 */
		void setBuffer(const BufferBindingStruct& resource)
		{
			assert(resource.mBuffer != nullptr);
			assert(resource.mBuffer->getSize() == mDeclaration->mSize);
			BufferBindingInstance::mBuffer = resource.mBuffer.get();
			raiseChanged();
		}

	private:
		friend class BaseMaterial;
		friend class BaseMaterialInstance;

		/**
		 * Called when the buffer changes
		 */
		void raiseChanged()									{ if (mBindingChangedCallback) mBindingChangedCallback(*this); }

		const ShaderVariableStructBufferDeclaration*		mDeclaration;
	};


	//////////////////////////////////////////////////////////////////////////
	// BufferBindingNumericInstance
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Base class of all numeric value typed buffer binding instances.
	 */
	class NAPAPI BufferBindingNumericInstance : public BufferBindingInstance
	{
		RTTI_ENABLE(BufferBindingInstance)
	public:
		// Constructor
		BufferBindingNumericInstance(const std::string& bindingName, const ShaderVariableValueArrayDeclaration& declaration, const BufferBindingChangedCallback& bindingChangedCallback) :
			BufferBindingInstance(bindingName, bindingChangedCallback),
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

	protected:
		const ShaderVariableValueArrayDeclaration*			mDeclaration;
		BufferBindingChangedCallback						mBindingChangedCallback;
	};


	/**
	 * Specific numeric value type of buffer binding instance, for example:
	 * `VertexBufferVec4` binds to `BufferBindingVec4Instance`.
	 */
	template<typename T>
	class TypedBufferBindingNumericInstance final : public BufferBindingNumericInstance
	{
		RTTI_ENABLE(BufferBindingNumericInstance)
	public:
		// Constructor
		TypedBufferBindingNumericInstance(const std::string& bindingName, const ShaderVariableValueArrayDeclaration& declaration, const BufferBindingChangedCallback& bindingChangedCallback) :
			BufferBindingNumericInstance(bindingName, declaration, bindingChangedCallback)
		{ }

		/**
		 * @return numeric buffer
		 */
		const GPUBufferNumeric& getBuffer() const								{ assert(hasBuffer()); return static_cast<const GPUBufferNumeric&>(*mBuffer); }

		/**
		 * @return numeric buffer
		 */
		GPUBufferNumeric& getBuffer()											{ assert(hasBuffer()); return static_cast<GPUBufferNumeric&>(*mBuffer); }

		/**
		 * Binds a new buffer to the uniform instance
		 * @param buffer new buffer to bind
		 */
		void setBuffer(TypedGPUBufferNumeric<T>& buffer);

		/**
		 * Updates thebuffer from a resource.
		 * @param resource resource to set buffer from.
		 */
		void setBuffer(const TypedBufferBindingNumeric<T>& resource)			{ BufferBindingInstance::mBuffer = resource.mBuffer.get(); }

		/**
		 * @return buffer
		 */
		const TypedGPUBufferNumeric<T>& getTypedBuffer() const						{ assert(mBuffer != nullptr); return static_cast<const TypedGPUBufferNumeric<T>&>(*mBuffer); }

		/**
		 * @return buffer
		 */
		TypedGPUBufferNumeric<T>& getTypedBuffer()									{ assert(mBuffer != nullptr); return static_cast<TypedGPUBufferNumeric<T>&>(*mBuffer); }

	private:
		/**
		 * Called when the buffer changes
		 */
		void raiseChanged()														{ if (mBindingChangedCallback) mBindingChangedCallback(*this); }
	};


	//////////////////////////////////////////////////////////////////////////
	// TypedBufferBindingNumericInstance type definitions
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

	template<class T>
	void nap::TypedBufferBindingNumericInstance<T>::setBuffer(TypedGPUBufferNumeric<T>& buffer)
	{
		assert(buffer.getSize() == mDeclaration->mSize);
		BufferBindingInstance::mBuffer = &buffer;
		raiseChanged();
	}
}

/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
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
	class BufferBinding;

	/**
	 * Buffer Binding resource base class.
	 * 
	 * Buffer bindings, unlike uniforms, store a reference to the underlying data as opposed to the data itself.
	 * This allows for any compute shader to read from and write to the same data storage. Buffer bindings always refer
	 * to a single nap::GPUBuffer, whether this is simple a `nap::VertexBufferVec4` or a more complex
	 * `nap::StructGPUBuffer`.
	 *
	 * A single vec4 array can be addressed as a `nap::VertexBufferVec4`:
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
	class NAPAPI BufferBinding : public Resource
	{
		RTTI_ENABLE(Resource)
	public:

		/**
		 * Ensures the buffer is set.
		 * @param errorState contains the error if the binding can't be initialized
		 * @return if the binding initialized
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * @return The number of elements in this array
		 */
		virtual int getCount() const					{ assert(mBuffer != nullptr);  return mBuffer->getCount(); }

		/**
		 * @return The size in bytes
		 */
		virtual size_t getSize() const					{ assert(mBuffer != nullptr); return mBuffer->getSize(); }

		/**
		 * @return The element size in bytes
		 */
		virtual size_t getElementSize() const			{ assert(mBuffer != nullptr); return mBuffer->getElementSize(); }

		/**
		 * @return the base GPU buffer, nullptr if not set
		 */
		const GPUBuffer* getBuffer() const				{ return mBuffer; }

		std::string mName;								///< Property: 'Name' name of buffer binding uniform in shader

	protected:
		// Buffer, set by derived classes
		const GPUBuffer* mBuffer = nullptr;
	};


	/**
	 * Base class of all numeric value typed buffer bindings.
	 */
	class NAPAPI BufferBindingNumeric : public BufferBinding
	{
		RTTI_ENABLE(BufferBinding)
	public:

		/**
		 * Ensures the buffer is set
		 * @param errorState contains the error if the binding can't be initialized
		 * @return if the binding initialized
		 */
		virtual bool init(utility::ErrorState& errorState) override		{ return BufferBinding::init(errorState); }

		/**
		 * @return a pointer to the buffer, nullptr if not set
		 */
		const GPUBufferNumeric* getBuffer() const						{ return static_cast<const GPUBufferNumeric*>(BufferBinding::mBuffer); }
	};


	/**
	 * Specific numeric value type of buffer binding, for example:
	 * `VertexBufferFloat` binds to `BufferBindingFloat`.
	 */
	template <typename T>
	class TypedBufferBindingNumeric : public BufferBindingNumeric
	{
		RTTI_ENABLE(BufferBindingNumeric)
	public:
		/**
		 * Ensures the buffer is set
		 * @param errorState contains the error if the binding can't be initialized
		 * @return if the binding initialized
		 */
		virtual bool init(utility::ErrorState& errorState);

		rtti::ObjectPtr<TypedGPUBufferNumeric<T>> mBuffer = nullptr;	/// Property: 'Buffer' the GPU NumericBuffer to bind
	};


	/**
	 * Represents a struct buffer binding, for example:
	 * `StructGPUBuffer` binds to `BufferBindingStruct`.
	 */
	class NAPAPI BufferBindingStruct : public BufferBinding
	{
		RTTI_ENABLE(BufferBinding)
	public:

		/**
		 * Ensures the buffer is set
		 * @param errorState contains the error if the binding can't be initialized
		 * @return if the binding initialized
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * @return a pointer to the buffer, nullptr if not set
		 */
		virtual const StructBuffer* getBuffer() const					{ return mBuffer.get(); };

		rtti::ObjectPtr<StructBuffer> mBuffer = nullptr;				/// Property 'Buffer' the GPU StructBuffer to bind
	};


	//////////////////////////////////////////////////////////////////////////
	// TypedBufferBindingNumeric type definitions
	//////////////////////////////////////////////////////////////////////////

	using BufferBindingUInt		= TypedBufferBindingNumeric<uint>;
	using BufferBindingInt		= TypedBufferBindingNumeric<int>;
	using BufferBindingFloat	= TypedBufferBindingNumeric<float>;
	using BufferBindingVec2		= TypedBufferBindingNumeric<glm::vec2>;
	using BufferBindingVec3		= TypedBufferBindingNumeric<glm::vec3>;
	using BufferBindingVec4		= TypedBufferBindingNumeric<glm::vec4>;
	using BufferBindingIVec4	= TypedBufferBindingNumeric<glm::ivec4>;
	using BufferBindingUVec4	= TypedBufferBindingNumeric<glm::uvec4>;
	using BufferBindingMat4		= TypedBufferBindingNumeric<glm::mat4>;


	//////////////////////////////////////////////////////////////////////////
	// Template definitions
	//////////////////////////////////////////////////////////////////////////

	template <typename T>
	bool nap::TypedBufferBindingNumeric<T>::init(utility::ErrorState& errorState)
	{
		BufferBinding::mBuffer = TypedBufferBindingNumeric::mBuffer.get();
		return BufferBindingNumeric::init(errorState);
	}

}

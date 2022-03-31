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
		std::string mName;		///< Name of uniform in shader

		/**
		 * @return The number of elements in this array
		 */
		virtual int getCount() const = 0;

		/**
		 * @return The size in bytes
		 */
		virtual size_t getSize() const = 0;

		/**
		 * @return Whether a buffer is set
		 */
		virtual bool hasBuffer() const = 0;

		/**
		 * @return the base GPU buffer, nullptr if not set
		 */
		virtual const BaseGPUBuffer* getBaseBuffer() const = 0;
	};


	/**
	 * Base class of all numeric value typed buffer bindings.
	 */
	class NAPAPI BufferBindingNumeric : public BufferBinding
	{
		RTTI_ENABLE(BufferBinding)
	public:
		/**
		 * @return a pointer to the buffer, nullptr if not set
		 */
		virtual const GPUBuffer* getBuffer() const = 0;
	};


	/**
	 * Specific numeric value type of buffer binding, for example:
	 * `VertexBufferFloat` binds to `BufferBindingFloat`.
	 */
	template <typename T>
	class NAPAPI TypedBufferBindingNumeric : public BufferBindingNumeric
	{
		RTTI_ENABLE(BufferBindingNumeric)
	public:
		/**
		 * @return total number of elements
		 */
		virtual int getCount() const override							{ return hasBuffer() ? mBuffer->mCount : 0; }

		/**
		 * @return The size in bytes
		 */
		virtual size_t getSize() const override							{ return mBuffer->getSize(); }

		/**
		 * @return Whether a buffer is set
		 */
		virtual bool hasBuffer() const override							{ return mBuffer != nullptr; }

		/**
		 * @return the base GPU buffer, nullptr if not set
		 */
		virtual const BaseGPUBuffer* getBaseBuffer() const override		{ return mBuffer.get(); }

		/**
		 * @return a pointer to the buffer, nullptr if not set
		 */
		virtual const GPUBuffer* getBuffer() const override				{ return mBuffer.get(); }

		rtti::ObjectPtr<GPUBufferNumeric<T>> mBuffer = nullptr;	/// Property 'Buffer'
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
		 * @return total number of elements.
		 */
		virtual int getCount() const override							{ return mBuffer->getCount(); }

		/**
		 * @return The size in bytes
		 */
		virtual size_t getSize() const override							{ return mBuffer->getSize(); }

		/**
		 * @return if the buffer is set
		 */
		virtual bool hasBuffer() const override							{ return mBuffer != nullptr; }

		/**
		 * @return the base GPU buffer, nullptr if not set
		 */
		virtual const BaseGPUBuffer* getBaseBuffer() const override		{ return mBuffer.get(); }

		/**
		 * @return a pointer to the buffer, nullptr if not set
		 */
		virtual const StructBuffer* getBuffer() const					{ return mBuffer.get(); };

		rtti::ObjectPtr<StructBuffer> mBuffer = nullptr;
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
	using BufferBindingMat4		= TypedBufferBindingNumeric<glm::mat4>;
}

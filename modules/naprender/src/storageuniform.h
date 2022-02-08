/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "shadervariabledeclarations.h"
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
	class StorageUniformBuffer;

	using StorageUniformCreatedCallback = std::function<void()>;

	/**
	 * Shader storage uniform resource base class.
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
	class NAPAPI StorageUniform : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		std::string mName;		///< Name of uniform in shader
	};


	/**
	 * Storage Uniform Buffer container.
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
	class NAPAPI StorageUniformStruct : public StorageUniform
	{
		RTTI_ENABLE(StorageUniform)
	public:
		/**
		 * @param name the name of the storage uniform buffer to find.
		 * @return a storage uniform buffer with the given name, nullptr if not found
		 */
		StorageUniformBuffer* findStorageUniformBuffer(const std::string& name);

		/**
		 * @param name the name of the storage uniform buffer to find.
		 * @return a storage uniform buffer with the given name, nullptr if not found
		 */
		const StorageUniformBuffer* findStorageUniformBuffer(const std::string& name) const;

		rtti::ObjectPtr<StorageUniformBuffer> mStorageUniformBuffer;
	};


	/**
	 * Storage uniform buffer base class.
	 * 
	 * A StorageUniformBuffer must be declared as part of a StorageUniformStruct.
	 */
	class NAPAPI StorageUniformBuffer : public StorageUniform
	{
		RTTI_ENABLE(StorageUniform)
	public:
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
	};


	/**
	 * Base class of all typed storage uniform value buffers.
	 * 
	 * A StorageUniformValueBuffer must be declared as part of a StorageUniformStruct.
	 */
	class NAPAPI StorageUniformValueBuffer : public StorageUniformBuffer
	{
		RTTI_ENABLE(StorageUniformBuffer)
	public:
		/**
		 * @return a pointer to the buffer, nullptr if not set
		 */
		virtual const ValueGPUBuffer* getBuffer() const = 0;
	};


	/**
	 * Specific type of storage uniform value buffer, for example:
	 * TypedValueGPUBuffer<float> -> TypedStorageUniformValueBuffer<float>.
	 * All supported types are defined below for easier readability.
	 *
	 * A StorageUniformValueBuffer must be declared as part of a StorageUniformStruct.
	 */
	template <typename T>
	class NAPAPI TypedStorageUniformValueBuffer : public StorageUniformValueBuffer
	{
		RTTI_ENABLE(StorageUniformValueBuffer)
	public:
		/**
		 * @return total number of elements
		 */
		virtual int getCount() const override { return hasBuffer() ? mBuffer->mCount : 0; }

		/**
		 * @return The size in bytes
		 */
		virtual size_t getSize() const override { return mBuffer->getSize(); }

		/**
		 * @return Whether a buffer is set
		 */
		virtual bool hasBuffer() const override { return mBuffer != nullptr; }

		/**
		 * @return a pointer to the buffer, nullptr if not set
		 */
		virtual const ValueGPUBuffer* getBuffer() const override { return mBuffer.get(); }

		rtti::ObjectPtr<TypedValueGPUBuffer<T>> mBuffer = nullptr;	/// Property 'Buffer'
	};


	/**
	 * Represents a storage uniform struct buffer, for example:
	 * StructGPUBuffer -> StorageUniformStructBuffer.
	 *
	 * A StorageUniformStructBuffer must be declared as part of a StorageUniformStruct.
	 */
	class NAPAPI StorageUniformStructBuffer : public StorageUniformBuffer
	{
		RTTI_ENABLE(StorageUniformBuffer)
	public:
		/**
		 * @return total number of elements.
		 */
		virtual int getCount() const override { return mBuffer->getCount(); }

		/**
		 * @return The size in bytes
		 */
		virtual size_t getSize() const override { return mBuffer->getSize(); }

		/**
		 * @return if the buffer is set
		 */
		virtual bool hasBuffer() const override { return mBuffer != nullptr; }

		/**
		 * @return a pointer to the buffer, nullptr if not set
		 */
		virtual const StructGPUBuffer* getBuffer() const { return mBuffer.get(); };

		rtti::ObjectPtr<StructGPUBuffer> mBuffer = nullptr;
	};


	/**
	 * Find a shader storage uniform based on the given shader variable declaration.
	 * @param members uniforms of type nap::StorageUniform to search through.
	 * @param declaration uniform declaration to match
	 * @return uniform that matches with the given shader declaration, nullptr if not found.
	 */
	template<class T>
	const StorageUniform* findStorageUniformStructMember(const std::vector<T>& members, const ShaderVariableDeclaration& declaration)
	{
		for (auto& member : members)
			if (member->mName == declaration.mName)
				return member.get();
		return nullptr;
	}


	//////////////////////////////////////////////////////////////////////////
	// Storage uniform value buffer type definitions
	//////////////////////////////////////////////////////////////////////////

	using StorageUniformUIntBuffer	= TypedStorageUniformValueBuffer<uint>;
	using StorageUniformIntBuffer	= TypedStorageUniformValueBuffer<int>;
	using StorageUniformFloatBuffer = TypedStorageUniformValueBuffer<float>;
	using StorageUniformVec2Buffer	= TypedStorageUniformValueBuffer<glm::vec2>;
	using StorageUniformVec3Buffer	= TypedStorageUniformValueBuffer<glm::vec3>;
	using StorageUniformVec4Buffer	= TypedStorageUniformValueBuffer<glm::vec4>;
	using StorageUniformMat4Buffer	= TypedStorageUniformValueBuffer<glm::mat4>;
}

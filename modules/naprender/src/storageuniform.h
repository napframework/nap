#pragma once

// Local Includes
#include "uniformdeclarations.h"
#include "gpuvaluebuffer.h"

// External Includes
#include <rtti/objectptr.h>
#include <glm/glm.hpp>
#include <utility/dllexport.h>
#include <nap/resource.h>

namespace nap
{
	// Forward Declares
	class StorageUniformInstance;

	using StorageUniformCreatedCallback = std::function<void()>;

	/**
	 * Shader uniform resource base class.
	 */
	class NAPAPI StorageUniform : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		std::string mName;		///< Name of uniform in shader
	};


	/**
	 * Uniform block container
	 */
	class NAPAPI StorageUniformStruct : public StorageUniform
	{
		RTTI_ENABLE(StorageUniform)
	public:
		rtti::ObjectPtr<StorageUniformBlock> mUniformBlock;
	};


	/**
	 * Uniform block base class
	 */
	class NAPAPI StorageUniformBlock : public StorageUniform
	{
		RTTI_ENABLE(StorageUniform)
	};


	/**
	 * Structured data
	 */
	class NAPAPI StorageUniformValueBuffer : public StorageUniformBlock
	{
		RTTI_ENABLE(StorageUniformBlock)
	public:
		/**
		 * @return The number of elements in this array
		 */
		virtual int getCount() const = 0;

		/**
		 * @return Whether a buffer is set
		 */
		virtual bool hasBuffer() const = 0;
	};


	/**
	 * Structured data
	 */
	template <typename T>
	class NAPAPI TypedStorageUniformValueBuffer : public StorageUniformValueBuffer
	{
		RTTI_ENABLE(StorageUniformValueBuffer)
	public:
		/**
		 * @return total number of elements.
		 */
		virtual int getCount() const override { return hasBuffer() ? mBuffer->mCount : 0; }

		virtual bool hasBuffer() const override { return mBuffer != nullptr; }

		rtti::ObjectPtr<TypedGPUValueBuffer<T>> mBuffer;	/// Property 'Buffer'
	};


	/**
	 * Block of raw data
	 */
	class NAPAPI StorageUniformBlockBuffer : public StorageUniformBlock
	{
		RTTI_ENABLE(StorageUniform)
	public:
		//rtti::ObjectPtr<GPUDataBuffer> mBuffer;
	};
}

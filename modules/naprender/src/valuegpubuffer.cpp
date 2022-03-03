/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "valuegpubuffer.h"
#include "renderservice.h"
#include "mathutils.h"

// External Includes
#include <nap/core.h>
#include <nap/logger.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::GPUBuffer)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////
// TypedValueGPUBuffer
//////////////////////////////////////////////////////////////////////////

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::UIntGPUBuffer)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::IntGPUBuffer)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::FloatGPUBuffer)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Vec2GPUBuffer)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Vec3GPUBuffer)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Vec4GPUBuffer)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Mat4GPUBuffer)
RTTI_END_CLASS


//////////////////////////////////////////////////////////////////////////
// GPUBuffer
//////////////////////////////////////////////////////////////////////////

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::UIntGPUBuffer)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Count", &nap::UIntGPUBuffer::mCount, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Usage", &nap::UIntGPUBuffer::mUsage, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DescriptorType", &nap::UIntGPUBuffer::mDescriptorType, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FillPolicy", &nap::UIntGPUBuffer::mBufferFillPolicy, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Clear", &nap::UIntGPUBuffer::mClear, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::IntGPUBuffer)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Count", &nap::IntGPUBuffer::mCount, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Usage", &nap::IntGPUBuffer::mUsage, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DescriptorType", &nap::IntGPUBuffer::mDescriptorType, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FillPolicy", &nap::IntGPUBuffer::mBufferFillPolicy, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Clear", &nap::IntGPUBuffer::mClear, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::FloatGPUBuffer)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Count", &nap::FloatGPUBuffer::mCount, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Usage", &nap::FloatGPUBuffer::mUsage, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DescriptorType", &nap::FloatGPUBuffer::mDescriptorType, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FillPolicy", &nap::FloatGPUBuffer::mBufferFillPolicy, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Clear", &nap::FloatGPUBuffer::mClear, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Vec2GPUBuffer)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Count", &nap::Vec2GPUBuffer::mCount, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Usage", &nap::Vec2GPUBuffer::mUsage, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DescriptorType", &nap::Vec2GPUBuffer::mDescriptorType, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FillPolicy", &nap::Vec2GPUBuffer::mBufferFillPolicy, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Clear", &nap::Vec2GPUBuffer::mClear, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Vec3GPUBuffer)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Count", &nap::Vec3GPUBuffer::mCount, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Usage", &nap::Vec3GPUBuffer::mUsage, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DescriptorType", &nap::Vec3GPUBuffer::mDescriptorType, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FillPolicy", &nap::Vec3GPUBuffer::mBufferFillPolicy, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Clear", &nap::Vec3GPUBuffer::mClear, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Vec4GPUBuffer)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Count", &nap::Vec4GPUBuffer::mCount, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Usage", &nap::Vec4GPUBuffer::mUsage, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DescriptorType", &nap::Vec4GPUBuffer::mDescriptorType, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FillPolicy", &nap::Vec4GPUBuffer::mBufferFillPolicy, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Clear", &nap::Vec4GPUBuffer::mClear, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Mat4GPUBuffer)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Count", &nap::Mat4GPUBuffer::mCount, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Usage", &nap::Mat4GPUBuffer::mUsage, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DescriptorType", &nap::Mat4GPUBuffer::mDescriptorType, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FillPolicy", &nap::Mat4GPUBuffer::mBufferFillPolicy, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Clear", &nap::Mat4GPUBuffer::mClear, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS


//////////////////////////////////////////////////////////////////////////
// VertexBuffer
//////////////////////////////////////////////////////////////////////////

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::UIntVertexBuffer)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::IntVertexBuffer)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::FloatVertexBuffer)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Vec2VertexBuffer)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Vec3VertexBuffer)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Vec4VertexBuffer)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS


//////////////////////////////////////////////////////////////////////////
// IndexBuffer
//////////////////////////////////////////////////////////////////////////

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::IndexBuffer)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS


namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// GPUBufferNumeric
	//////////////////////////////////////////////////////////////////////////

	template<typename T>
	bool GPUBufferNumeric<T>::init(utility::ErrorState& errorState)
	{
		if (!GPUBuffer::init(errorState))
			return false;

		if (!errorState.check(mUsage != EMemoryUsage::DynamicWrite || mCount >= 0, "Cannot allocate a non-DynamicWrite buffer with zero elements."))
			return false;

		// Compose usage flags from buffer configuration
		mUsageFlags |= getBufferUsage(mDescriptorType);

		// Calculate buffer size
		uint32 buffer_size = mCount * sizeof(T);

		// Allocate buffer memory
		if (!allocateInternal(buffer_size, mUsageFlags, errorState))
			return false;

		// Upload data when a buffer fill policy is available
		if (mBufferFillPolicy != nullptr)
		{
			if (mUsage != EMemoryUsage::DynamicRead)
			{
				// Create a staging buffer to upload
				auto staging_buffer = std::make_unique<T[]>(mCount);
				mBufferFillPolicy->fill(mCount, staging_buffer.get());

				// Prepare staging buffer upload
				if (!setDataInternal(staging_buffer.get(), buffer_size, buffer_size, mUsageFlags, errorState))
					return false;
			}
			else
			{
				// Warn user that buffers cannot be filled when their usage is set to DynamicRead
				nap::Logger::warn(utility::stringFormat("%s: The configured fill policy was ignored as the buffer usage is DynamicRead", mID.c_str()).c_str());
			}
		}

		// Optionally clear - does not count as an upload
		else if (mClear)
			BaseGPUBuffer::requestClear();

		mInitialized = true;
		return true;
	}


	//////////////////////////////////////////////////////////////////////////
	// VertexBuffer
	//////////////////////////////////////////////////////////////////////////

	template<typename T>
	bool VertexBuffer<T>::init(utility::ErrorState& errorState)
	{
		GPUBufferNumeric<T>::mUsageFlags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		return GPUBufferNumeric<T>::init(errorState);
	}


	//////////////////////////////////////////////////////////////////////////
	// Index Buffer
	//////////////////////////////////////////////////////////////////////////

	bool IndexBuffer::init(utility::ErrorState& errorState)
	{
		GPUBufferNumeric<uint>::mUsageFlags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		return GPUBufferNumeric<uint>::init(errorState);
	}


	// Explicit template instantiations
	template bool UIntGPUBuffer::init(utility::ErrorState& errorState);
	template bool IntGPUBuffer::init(utility::ErrorState& errorState);
	template bool FloatGPUBuffer::init(utility::ErrorState& errorState);
	template bool Vec2GPUBuffer::init(utility::ErrorState& errorState);
	template bool Vec3GPUBuffer::init(utility::ErrorState& errorState);
	template bool Vec4GPUBuffer::init(utility::ErrorState& errorState);
	template bool Mat4GPUBuffer::init(utility::ErrorState& errorState);

	template bool UIntVertexBuffer::init(utility::ErrorState& errorState);
	template bool IntVertexBuffer::init(utility::ErrorState& errorState);
	template bool FloatVertexBuffer::init(utility::ErrorState& errorState);
	template bool Vec2VertexBuffer::init(utility::ErrorState& errorState);
	template bool Vec3VertexBuffer::init(utility::ErrorState& errorState);
	template bool Vec4VertexBuffer::init(utility::ErrorState& errorState);
}

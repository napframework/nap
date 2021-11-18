/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <string>
#include <vector>
#include "vulkan/vulkan_core.h"
#include "rtti/typeinfo.h"

namespace nap
{
	/**
	 * All available shader uniform types
	 */
	enum class EUniformValueType : uint8_t
	{
		Unknown = 0,		///< unknown or invalid shader uniform
		Float = 1,			///< float
		Int = 2,			///< int
		UInt = 3,			///< unsigned int
		Vec2 = 4,			///< 2 float vector
		Vec3 = 5,			///< 3 float vector
		Vec4 = 6,			///< 4 float vector
		Mat2 = 7,			///< 2x2 float matrix
		Mat3 = 8,			///< 3x3 float matrix
		Mat4 = 9			///< 4x4 float matrix
	};


	/**
	 * Supported buffer types
	 */
	enum class EBufferObjectType : uint8_t
	{
		Uniform = 0,		///< device readonly
		Storage = 1			///< device read/write
	};


	/**
	 * Returns the vulkan buffer usage flags for a given buffer type
	 */
	static VkBufferUsageFlags getBufferUsage(EBufferObjectType type)
	{
		if (type == EBufferObjectType::Uniform)
			return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

		// Storage buffers may be used as vertex attribute buffers on device memory
		else if (type == EBufferObjectType::Storage)
			return VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

		assert(false);
		return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	}


	/**
	 * Returns the vulkan descriptor type for a given buffer object type
	 */
	static VkDescriptorType getDescriptorType(nap::EBufferObjectType type)
	{
		if (type == nap::EBufferObjectType::Uniform)
			return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

		else if (type == nap::EBufferObjectType::Storage)
			return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

		assert(false);
		return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	}


	/**
	 * Uniform shader declaration base class.
	 */
	class UniformDeclaration
	{
		RTTI_ENABLE()
	public:
		UniformDeclaration(const std::string& name, int offset, int size);
		virtual ~UniformDeclaration() {}

		std::string		mName;												///< Name of the declaration
		int				mOffset;											///< Memory offset
		int				mSize;												///< Total size (in bytes) of declaration
	};


	/**
	 * Uniform value shader declaration (float, int etc.)
	 */
	class UniformValueDeclaration : public UniformDeclaration
	{
		RTTI_ENABLE(UniformDeclaration)

	public:
		UniformValueDeclaration(const std::string& name, int offset, int size, EUniformValueType type);
		EUniformValueType mType;											///< Uniform type
	};


	/**
	 * Uniform struct shader declaration.
	 */
	class NAPAPI UniformStructDeclaration : public UniformDeclaration
	{
		RTTI_ENABLE(UniformDeclaration)
	public:
		UniformStructDeclaration(const std::string& name, EBufferObjectType type, int offset, int size);
		virtual ~UniformStructDeclaration() override;

		UniformStructDeclaration(UniformStructDeclaration&& inRHS);
		UniformStructDeclaration& operator=(UniformStructDeclaration&& inRHS);
		UniformStructDeclaration(const UniformStructDeclaration&) = delete;
		UniformStructDeclaration& operator=(const UniformStructDeclaration&) = delete;

		/**
		 * @return a uniform shader declaration with the given name.
		 * @param name name of the declaration to find
		 * @return found declaration, nullptr if not found.
		 */
		const UniformDeclaration* findMember(const std::string& name) const;
		std::vector<std::unique_ptr<UniformDeclaration>> mMembers;				///< All shader declarations associated with struct
		EBufferObjectType mBufferObjectType;									///< Usage: uniform, storage
	};


	/**
	 * List of uniform struct shader declarations.
	 */
	class UniformStructArrayDeclaration : public UniformDeclaration
	{
		RTTI_ENABLE(UniformDeclaration)
	public:
		UniformStructArrayDeclaration(const std::string& name, int offset, int size);

		std::vector<std::unique_ptr<UniformStructDeclaration>> mElements;		///< All struct declarations.
	};


	/**
	 * List of uniform value shader declarations.
	 */
	class UniformValueArrayDeclaration : public UniformDeclaration
	{
		RTTI_ENABLE(UniformDeclaration)
	public:
		UniformValueArrayDeclaration(const std::string& name, int offset, int size, int stride, EUniformValueType elementType, int numElements);

		EUniformValueType	mElementType;										///< Uniform type
		int					mNumElements;										///< Total number of elements in list
		int					mStride;											///< Stride of element in array
	};


	class UniformBufferObjectDeclaration : public UniformStructDeclaration
	{
		RTTI_ENABLE(UniformStructDeclaration)
	public:
		UniformBufferObjectDeclaration(const std::string& name, int binding, VkShaderStageFlagBits inStage, EBufferObjectType type, int size);

		UniformBufferObjectDeclaration(UniformBufferObjectDeclaration&& inRHS);
		UniformBufferObjectDeclaration& operator=(UniformBufferObjectDeclaration&& inRHS);
		UniformBufferObjectDeclaration(const UniformBufferObjectDeclaration&) = delete;
		UniformBufferObjectDeclaration& operator=(const UniformBufferObjectDeclaration&) = delete;

		int														mBinding;	///< Shader binding identifier
		VkShaderStageFlagBits									mStage;		///< Shader stage: vertex, fragment, compute etc.
	};
	using UBODeclarationList = std::vector<nap::UniformBufferObjectDeclaration>;

	// Shader Variable Declaration typedefs
	using ShaderVariableDeclaration = UniformDeclaration;
	using ShaderVariableValueDeclaration = UniformValueDeclaration;
	using ShaderVariableStructDeclaration = UniformStructDeclaration;
	using ShaderVariableStructArrayDeclaration = UniformStructArrayDeclaration;
	using ShaderVariableValueArrayDeclaration = UniformValueArrayDeclaration;

	using EShaderVariableValueType = EUniformValueType;
}

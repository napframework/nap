/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <nap/numeric.h>
#include <string>
#include <vector>
#include "vulkan/vulkan_core.h"
#include "rtti/typeinfo.h"
#include "gpubuffer.h"

namespace nap
{
	/**
	 * All available shader variable value types
	 */
	enum class EShaderVariableValueType : uint8
	{
		Unknown = 0,		///< unknown or invalid shader uniform
		Float = 2,			///< float
		Int = 3,			///< int
		UInt = 4,			///< unsigned int
		Vec2 = 5,			///< 2 float vector
		Vec3 = 6,			///< 3 float vector
		Vec4 = 7,			///< 4 float vector
		Mat2 = 8,			///< 2x2 float matrix
		Mat3 = 9,			///< 3x3 float matrix
		Mat4 = 10			///< 4x4 float matrix
	};


	/**
	 * Shader variable shader declaration base class.
	 */
	class ShaderVariableDeclaration
	{
		RTTI_ENABLE()
	public:
		ShaderVariableDeclaration(const std::string& name, int offset, int size);
		virtual ~ShaderVariableDeclaration() {}

		std::string		mName;													///< Name of the declaration
		int				mOffset;												///< Memory offset
		int				mSize;													///< Total size (in bytes) of declaration
	};


	/**
	 * Shadrer variable value shader declaration (float, int etc.)
	 */
	class ShaderVariableValueDeclaration : public ShaderVariableDeclaration
	{
		RTTI_ENABLE(ShaderVariableDeclaration)

	public:
		ShaderVariableValueDeclaration(const std::string& name, int offset, int size, EShaderVariableValueType type);
		EShaderVariableValueType mType;											///< ShaderVariable type
	};


	/**
	 * ShaderVariable struct shader declaration.
	 */
	class NAPAPI ShaderVariableStructDeclaration : public ShaderVariableDeclaration
	{
		RTTI_ENABLE(ShaderVariableDeclaration)
	public:
		ShaderVariableStructDeclaration(const std::string& name, EDescriptorType descriptorType, int offset, int size);
		virtual ~ShaderVariableStructDeclaration() override;

		ShaderVariableStructDeclaration(ShaderVariableStructDeclaration&& inRHS);
		ShaderVariableStructDeclaration& operator=(ShaderVariableStructDeclaration&& inRHS);
		ShaderVariableStructDeclaration(const ShaderVariableStructDeclaration&) = delete;
		ShaderVariableStructDeclaration& operator=(const ShaderVariableStructDeclaration&) = delete;

		/**
		 * @return a shader variable shader declaration with the given name.
		 * @param name name of the declaration to find
		 * @return found declaration, nullptr if not found.
		 */
		const ShaderVariableDeclaration* findMember(const std::string& name) const;
		std::vector<std::unique_ptr<ShaderVariableDeclaration>> mMembers;		///< All shader declarations associated with struct
		EDescriptorType mDescriptorType;										///< e.g. ShaderVariable or Storage
	};


	/**
	 * List of shader variable struct shader declarations.
	 */
	class ShaderVariableStructArrayDeclaration : public ShaderVariableDeclaration
	{
		RTTI_ENABLE(ShaderVariableDeclaration)
	public:
		ShaderVariableStructArrayDeclaration(const std::string& name, int offset, int size);

		std::vector<std::unique_ptr<ShaderVariableStructDeclaration>> mElements; ///< Struct declaration
	};


	/**
	 * Buffer representation of shader variable struct shader declarations. Used for storage buffers.
	 */
	class ShaderVariableStructBufferDeclaration : public ShaderVariableDeclaration
	{
		RTTI_ENABLE(ShaderVariableDeclaration)
	public:
		ShaderVariableStructBufferDeclaration(const std::string& name, int offset, int size, int stride, int numElements);

		std::unique_ptr<ShaderVariableStructDeclaration> mElement;				///< Struct declaration
		int					mNumElements;										///< Total number of struct elements in list
		int					mStride;											///< Stride of struct element in array
	};


	/**
	 * List of ShaderVariable value shader declarations.
	 */
	class ShaderVariableValueArrayDeclaration : public ShaderVariableDeclaration
	{
		RTTI_ENABLE(ShaderVariableDeclaration)
	public:
		ShaderVariableValueArrayDeclaration(const std::string& name, int offset, int size, int stride, EShaderVariableValueType elementType, int numElements);

		EShaderVariableValueType	mElementType;								///< Shader variable type
		int					mNumElements;										///< Total number of elements in list
		int					mStride;											///< Stride of element in array
	};


	/**
	 * Buffer Object Declaration struct
	 * Stores shader variable declarations and is used as a descriptor object for UBOs and SSBOs
	 */
	class BufferObjectDeclaration : public ShaderVariableStructDeclaration
	{
		RTTI_ENABLE(ShaderVariableStructDeclaration)
	public:
		BufferObjectDeclaration(const std::string& name, int binding, VkShaderStageFlagBits inStage, EDescriptorType descriptorType, int size);

		BufferObjectDeclaration(BufferObjectDeclaration&& inRHS);
		BufferObjectDeclaration& operator=(BufferObjectDeclaration&& inRHS);
		BufferObjectDeclaration(const BufferObjectDeclaration&) = delete;
		BufferObjectDeclaration& operator=(const BufferObjectDeclaration&) = delete;

		int														mBinding;	///< Shader binding identifier
		VkShaderStageFlagBits									mStage;		///< Shader stage: vertex, fragment, compute etc.
	};

	// Type alias
	using BufferObjectDeclarationList = std::vector<nap::BufferObjectDeclaration>;
}

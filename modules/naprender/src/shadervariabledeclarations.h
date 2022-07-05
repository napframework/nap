/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local includes
#include "vulkan/vulkan_core.h"
#include "rtti/typeinfo.h"

// External Includes
#include <nap/numeric.h>
#include <string>
#include <vector>

namespace nap
{
	/**
	 * Flag that determines the descriptor type of a shader resource. Regards the type of data access on the device (GPU) 
	 * inside a shader program.
	 * 
	 * Uniform buffers are typically small blocks of data that are updated very frequently from CPU to GPU (each frame),
	 * but immutable in a shader program.
	 * Storage buffers are typically large blocks of data that are bound/unbound to an SSBO and frequently read and written
	 * in a compute shader program.
	 * The Default option is used for buffers that are not bound to descriptorsets, e.g. vertex and index buffers.
	 */
	enum class EDescriptorType : uint
	{
		Uniform,			///< Specifies a uniform buffer descriptor. device readonly
		Storage				///< Specifies a storage buffer descriptor. device read/write
	};


	/**
	 * All available shader variable value types
	 */
	enum class EShaderVariableValueType : uint8
	{
		Unknown = 0,		///< unknown or invalid shader uniform
		Float,				///< float
		Int,				///< int
		UInt,				///< unsigned int
		Vec2,				///< 2 float vector
		Vec3,				///< 3 float vector
		Vec4,				///< 4 float vector
		IVec4,				///< 4 int vector
		UVec4,				///< 4 uint vector
		Mat2,				///< 2x2 float matrix
		Mat3,				///< 3x3 float matrix
		Mat4				///< 4x4 float matrix
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

		std::string					mName;										///< Name of the declaration
		int							mOffset;									///< Memory offset
		int							mSize;										///< Total size (in bytes) of declaration
	};
	

	/**
	 * Shader variable value shader declaration (float, int etc.)
	 */
	class ShaderVariableValueDeclaration : public ShaderVariableDeclaration
	{
		RTTI_ENABLE(ShaderVariableDeclaration)

	public:
		ShaderVariableValueDeclaration(const std::string& name, int offset, int size, EShaderVariableValueType type);
		EShaderVariableValueType	mType;										///< ShaderVariable type
	};


	/**
	 * Shader variable struct shader declaration.
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
		EDescriptorType				mDescriptorType;							///< The type of descriptor for this resource
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
	 *
	 * ShaderVariableStructBufferDeclaration is a special shader variable declaration type exclusive to storage buffer descriptor types.
	 * They are not built recursively as none of the values have to be assigned or accessed individually, but rather the buffer as a whole.
	 * Therefore, we only store the declaration of a single struct item as a ShaderVariableStructDeclaration, and set it to the element member
	 * of the ShaderVariableStructBufferDeclaration along with the element stride and count.
	 */
	class ShaderVariableStructBufferDeclaration : public ShaderVariableDeclaration
	{
		RTTI_ENABLE(ShaderVariableDeclaration)
	public:
		ShaderVariableStructBufferDeclaration(const std::string& name, int offset, int size, int stride, int numElements);

		std::unique_ptr<ShaderVariableStructDeclaration> mElement;				///< Struct declaration
		int							mNumElements;								///< Total number of struct elements in list
		int							mStride;									///< Stride of struct element in array
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
		int							mNumElements;								///< Total number of elements in list
		int							mStride;									///< Stride of element in array
	};


	/**
	 * Buffer Object Declaration struct.
	 * Stores shader variable declarations and is used as a descriptor object for UBOs and SSBOs.
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

		/**
		 * Returns the first buffer declaration. Asserts if not present.
		 * Handy accessor for buffer bindings.
		 */
		const ShaderVariableDeclaration& getBufferDeclaration() const;

		int							mBinding;									///< Shader binding identifier
		VkShaderStageFlagBits		mStage;										///< Shader stage: vertex, fragment, compute etc.
	};

	// Type alias
	using BufferObjectDeclarationList = std::vector<nap::BufferObjectDeclaration>;
}

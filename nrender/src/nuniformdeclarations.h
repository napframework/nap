#pragma once

// External Includes
#include <string>
#include <vector>
#include "vulkan/vulkan_core.h"
#include "rtti/typeinfo.h"

namespace opengl
{
	/**
	* All available OpenGL shader uniform types
	* All uniform types need to have a set factory
	* function associated with it
	*/
	enum class EGLSLType : uint8_t
	{
		Unknown = 0,			///< unknown or invalid shader uniform
		Float = 1,			///< float
		Int = 2,			///< int
		UInt = 3,			///< unsigned int
		Vec2 = 4,			///< 2 float vector
		Vec3 = 5,			///< 3 float vector
		Vec4 = 6,			///< 4 float vector
		Mat2 = 7,			///< 2x2 float matrix
		Mat3 = 8,			///< 3x3 float matrix
		Mat4 = 9				///< 4x4 float matrix
	};

	class UniformDeclaration
	{
		RTTI_ENABLE()

	public:
		UniformDeclaration(const std::string& name, int offset, int size) :
			mName(name),
			mOffset(offset),
			mSize(size)
		{
		}

		std::string		mName;
		int				mOffset;
		int				mSize;
	};

	class UniformValueDeclaration : public UniformDeclaration
	{
		RTTI_ENABLE(UniformDeclaration)

	public:
		UniformValueDeclaration(const std::string& name, int offset, int size, EGLSLType type) :
			UniformDeclaration(name, offset, size),
			mType(type)
		{
		}

		EGLSLType	mType;
	};

	class UniformStructDeclaration : public UniformDeclaration
	{
		RTTI_ENABLE(UniformDeclaration)

	public:
		UniformStructDeclaration(const std::string& name, int offset, int size) :
			UniformDeclaration(name, offset, size)
		{
		}

		const UniformDeclaration* findMember(const std::string& name) const
		{
			for (auto& member : mMembers)
				if (member->mName == name)
					return member.get();

			return nullptr;
		}

		std::vector<std::unique_ptr<UniformDeclaration>> mMembers;
	};

	class UniformStructArrayDeclaration : public UniformDeclaration
	{
		RTTI_ENABLE(UniformDeclaration)

	public:
		UniformStructArrayDeclaration(const std::string& name, int offset, int size) :
			UniformDeclaration(name, offset, size)
		{
		}

		std::vector<std::unique_ptr<UniformStructDeclaration>> mElements;
	};

	class UniformValueArrayDeclaration : public UniformDeclaration
	{
		RTTI_ENABLE(UniformDeclaration)

	public:
		UniformValueArrayDeclaration(const std::string& name, int offset, int size, EGLSLType elementType, int numElements) :
			UniformDeclaration(name, offset, size),
			mElementType(elementType),
			mNumElements(numElements)
		{
		}

		EGLSLType	mElementType;
		int			mNumElements;
	};

	class UniformBufferObjectDeclaration : public UniformStructDeclaration
	{
		RTTI_ENABLE(UniformStructDeclaration)

	public:
		UniformBufferObjectDeclaration(const std::string& name, int binding, VkShaderStageFlagBits inStage, int size) :
			UniformStructDeclaration(name, 0, size),
			mBinding(binding),
			mStage(inStage)
		{
		}

		int														mBinding;
		VkShaderStageFlagBits									mStage;
	};
}

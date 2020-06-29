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

	class UniformDeclaration
	{
		RTTI_ENABLE()

	public:
		UniformDeclaration(const std::string& name, int offset, int size);
		virtual ~UniformDeclaration() {}

		std::string		mName;
		int				mOffset;
		int				mSize;
	};

	class UniformValueDeclaration : public UniformDeclaration
	{
		RTTI_ENABLE(UniformDeclaration)

	public:
		UniformValueDeclaration(const std::string& name, int offset, int size, EUniformValueType type);

		EUniformValueType	mType;
	};

	class NAPAPI UniformStructDeclaration : public UniformDeclaration
	{
		RTTI_ENABLE(UniformDeclaration)

	public:
		UniformStructDeclaration(const std::string& name, int offset, int size);

		virtual ~UniformStructDeclaration() override;

		UniformStructDeclaration(const UniformStructDeclaration&) = delete;
		UniformStructDeclaration& operator=(const UniformStructDeclaration&) = delete;

		UniformStructDeclaration(UniformStructDeclaration&& inRHS);
		UniformStructDeclaration& operator=(UniformStructDeclaration&& inRHS);

		const UniformDeclaration* findMember(const std::string& name) const;

		std::vector<std::unique_ptr<UniformDeclaration>> mMembers;
	};

	class UniformStructArrayDeclaration : public UniformDeclaration
	{
		RTTI_ENABLE(UniformDeclaration)

	public:
		UniformStructArrayDeclaration(const std::string& name, int offset, int size);

		std::vector<std::unique_ptr<UniformStructDeclaration>> mElements;
	};

	class UniformValueArrayDeclaration : public UniformDeclaration
	{
		RTTI_ENABLE(UniformDeclaration)

	public:
		UniformValueArrayDeclaration(const std::string& name, int offset, int size, int stride, EUniformValueType elementType, int numElements);

		EUniformValueType	mElementType;
		int					mNumElements;
		int					mStride;
	};

	class UniformBufferObjectDeclaration : public UniformStructDeclaration
	{
		RTTI_ENABLE(UniformStructDeclaration)

	public:
		UniformBufferObjectDeclaration(const std::string& name, int binding, VkShaderStageFlagBits inStage, int size);

		UniformBufferObjectDeclaration(const UniformBufferObjectDeclaration&) = delete;
		UniformBufferObjectDeclaration& operator=(const UniformBufferObjectDeclaration&) = delete;
		
		UniformBufferObjectDeclaration(UniformBufferObjectDeclaration&& inRHS);
		UniformBufferObjectDeclaration& operator=(UniformBufferObjectDeclaration&& inRHS);

		int														mBinding;
		VkShaderStageFlagBits									mStage;
	};
}

#pragma once

// External Includes
#include <string>
#include <GL/glew.h>
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>
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
		Float   = 1,			///< float
		Int     = 2,			///< int
		UInt    = 3,			///< unsigned int
		Vec2	= 4,			///< 2 float vector
		Vec3	= 5,			///< 3 float vector
		Vec4	= 6,			///< 4 float vector
		Mat2    = 7,			///< 2x2 float matrix
		Mat3    = 8,			///< 3x3 float matrix
		Mat4    = 9				///< 4x4 float matrix
	};

	/**
	 * Result of OpenGL shader validation
	 */
	enum class EShaderValidationResult : uint8_t
	{
		Success,				///< Shader validation succeeded
		Warning,				///< Shader validation succeeded with a warning
		Error					///< Shader validation failed
	};

	/**
	 * @return the uniform type based on the opengl uniform type
	 * @param glType: opengl internal type that describes a uniform
	 */
	EGLSLType getGLSLType(GLenum glType);

	/**
	* Represents an opengl shader attribute
	*/
	class ShaderInput
	{
	public:
		// Constructor
		ShaderInput(const std::string& name, int location, VkFormat format);
		ShaderInput() = delete;

		std::string		mName;							///< Name of the shader attribute
		int				mLocation;
		VkFormat		mFormat;
	};

	class UniformSamplerDeclaration
	{
		RTTI_ENABLE()

	public:
		enum class EType : uint8_t
		{
			Type_1D,
			Type_2D,
			Type_3D
		};

		UniformSamplerDeclaration(const std::string& name, int binding, VkShaderStageFlagBits stage, EType type, int numArrayElements) :
			mName(name),
			mBinding(binding),
			mStage(stage),
			mType(type),
			mNumArrayElements(numArrayElements)
		{
		}

		std::string				mName;
		int						mBinding = -1;
		VkShaderStageFlagBits	mStage;
		EType					mType = EType::Type_2D;
		int						mNumArrayElements = 1;
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

	// Typedefs
	using UniformSamplerDeclarations = std::vector<UniformSamplerDeclaration>;

	using ShaderVertexAttribute = ShaderInput;
	using ShaderVertexAttributes = std::unordered_map<std::string, std::unique_ptr<ShaderVertexAttribute>>;

	/**
	 * Given part of a shader (say vertex shader), validateShader will get information from OpenGl 
	 * on whether or not the shader was compiled successfully
	 * Note that the shader part must be loaded
	 * @return if the shader part is loaded correctly
	 */
	EShaderValidationResult validateShader(GLuint shader, std::string& validationMessage);


	/**
	 * Given a shader program, validateProgram will request from OpenGL, any information
	 * related to the validation or linking of the program with it's attached shaders. It will
	 * then output any issues that have occurred.
	 * Note that the shader must be loaded
	 * @return if the shader program is loaded correctly without errors
	 */
	EShaderValidationResult validateShaderProgram(GLuint program, std::string& validationMessage);
}

//////////////////////////////////////////////////////////////////////////
// Hash
//////////////////////////////////////////////////////////////////////////
namespace std
{
	template<>
	struct hash<opengl::ShaderInput> 
	{
		size_t operator()(const opengl::ShaderInput &k) const 
		{
			return hash<std::string>()(k.mName);
		}
	};

	template <>
	struct hash<opengl::EGLSLType>
	{
		size_t operator()(const opengl::EGLSLType& v) const
		{
			return hash<uint8_t>()(static_cast<uint8_t>(v));
		}
	};
}

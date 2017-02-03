// local includes
#include "shaderutils.h"
#include "shaderresource.h"

// External Includes
#include <unordered_map>
#include <shaderresource.h>

namespace nap
{
	/**
	 * All convert functions
	 */
	void setUniformFloat(const opengl::UniformVariable& var, const AttributeBase& attr)			{ }
	void setUniformInt(const opengl::UniformVariable& var, const AttributeBase& attr)				{ }
	void setUniformUInt(const opengl::UniformVariable& var, const AttributeBase& attr)			{ }
	void setUniformVec2(const opengl::UniformVariable& var, const AttributeBase& attr)			{ }
	void setUniformVec3(const opengl::UniformVariable& var, const AttributeBase& attr)			{ }
	void setUniformVec4(const opengl::UniformVariable& var, const AttributeBase& attr)			{ }
	void setUniformMat2(const opengl::UniformVariable& var, const AttributeBase& attr)			{ }
	void setUniformMat3(const opengl::UniformVariable& var, const AttributeBase& attr)			{ }
	void setUniformMat4(const opengl::UniformVariable& var, const AttributeBase& attr)			{ }

	/**
	 * All attribute create functions
	 */
	AttributeBase& createGLSLFloatAttribute(const opengl::UniformVariable& uvar, CompoundAttribute& compound)
	{
		return compound.addAttribute<float>(uvar.mName.c_str(), 1.0f);
	}

	// Create int attribute
	AttributeBase& createGLSLIntAttribute(const opengl::UniformVariable& uvar, CompoundAttribute& compound)
	{
		return compound.addAttribute<int>(uvar.mName.c_str(), 0);
	}

	// Create uint attribute
	AttributeBase& createGLSLUIntAttribute(const opengl::UniformVariable& uvar, CompoundAttribute& compound)
	{
		return compound.addAttribute<nap::uint>(uvar.mName.c_str(), 0);
	}

	// Create vec2 attribute
	AttributeBase& createGLSLVec2Attribute(const opengl::UniformVariable& uvar, CompoundAttribute& compound)
	{
		return compound.addAttribute<glm::vec2>(uvar.mName.c_str(), glm::vec2());
	}

	// Create vec3 attribute
	AttributeBase& createGLSLVec3Attribute(const opengl::UniformVariable& uvar, CompoundAttribute& compound)
	{
		return compound.addAttribute<glm::vec3>(uvar.mName.c_str(), glm::vec3());
	}

	// Create vec4 attribute
	AttributeBase& createGLSLVec4Attribute(const opengl::UniformVariable& uvar, CompoundAttribute& compound)
	{
		return compound.addAttribute<glm::vec4>(uvar.mName.c_str(), glm::vec4());
	}

	// Create matrix 2x2 attribute
	AttributeBase& createGLSLMat2Attribute(const opengl::UniformVariable& uvar, CompoundAttribute& compound)
	{
		return compound.addAttribute<glm::mat2x2>(uvar.mName.c_str(), glm::mat2x2());
	}

	// Create matrix 3x3 attribute
	AttributeBase& createGLSLMat3Attribute(const opengl::UniformVariable& uvar, CompoundAttribute& compound)
	{
		return compound.addAttribute<glm::mat3x3>(uvar.mName.c_str(), glm::mat3x3());
	}

	// Create matrix 4x4 attribute
	AttributeBase& createGLSLMat4Attribute(const opengl::UniformVariable& uvar, CompoundAttribute& compound)
	{
		return compound.addAttribute<glm::mat4x4>(uvar.mName.c_str(), glm::mat4x4());
	}


	// Maps GLSL shader types to nap attribute types
	using GLSLAttributeMap = std::unordered_map<opengl::GLSLType, RTTI::TypeInfo>;
	
	/**
	* @return a map that holds a type for every GLSL shader input variable
	* TODO: MAKE THREAD SAFE: LOCK
	*/
	const GLSLAttributeMap& getGLSLAttributeMap()
	{
		static GLSLAttributeMap map;
		if (map.empty())
		{
			map.emplace(std::make_pair(opengl::GLSLType::Float, RTTI_OF(float)));
			map.emplace(std::make_pair(opengl::GLSLType::Int,	RTTI_OF(int)));
			map.emplace(std::make_pair(opengl::GLSLType::UInt,	RTTI_OF(nap::uint)));
			map.emplace(std::make_pair(opengl::GLSLType::Vec2,	RTTI_OF(glm::vec2)));
			map.emplace(std::make_pair(opengl::GLSLType::Vec3,	RTTI_OF(glm::vec3)));
			map.emplace(std::make_pair(opengl::GLSLType::Vec4,	RTTI_OF(glm::vec4)));
			map.emplace(std::make_pair(opengl::GLSLType::Mat2,	RTTI_OF(glm::mat2x2)));
			map.emplace(std::make_pair(opengl::GLSLType::Mat3,	RTTI_OF(glm::mat3x3)));
			map.emplace(std::make_pair(opengl::GLSLType::Mat4,	RTTI_OF(glm::mat4x4)));
			map.emplace(std::make_pair(opengl::GLSLType::Tex2D, RTTI_OF(int)));		// TODO: THIS NEEDS TO BECOME A LINK!
		}
		return map;
	}


	// Maps a nap attribute type to a glsl setter function
	using GLSLSetterMap = std::unordered_map<RTTI::TypeInfo, GLSLSetterFunction>;

	/**
	 * @return a map that links an attribute type to a GLSL set function
	 */
	const GLSLSetterMap& getGLSLSetterMap()
	{
		static GLSLSetterMap map;
		if (map.empty())
		{
			map.emplace(RTTI_OF(float),			setUniformFloat);
			map.emplace(RTTI_OF(int),			setUniformInt);
			map.emplace(RTTI_OF(nap::uint),		setUniformUInt);
			map.emplace(RTTI_OF(glm::vec2),		setUniformVec2);
			map.emplace(RTTI_OF(glm::vec3),		setUniformVec3);
			map.emplace(RTTI_OF(glm::vec4),		setUniformVec4);
			map.emplace(RTTI_OF(glm::mat2x2),	setUniformMat2);
			map.emplace(RTTI_OF(glm::mat3x3),	setUniformMat3);
			map.emplace(RTTI_OF(glm::mat4x4),	setUniformMat4);
		}
		return map;
	}


	// Maps an Attribute type to an attribute create function
	using GLSLAttributeCreateMap = std::unordered_map<opengl::GLSLType, GLSLAttributeCreateFunction>;

	/**
	 * @return a map that contains the attribute create function for the specified GLSL type
	 */
	GLSLAttributeCreateMap& getGLSLAttributeCreateMap()
	{
		static GLSLAttributeCreateMap map;
		if (map.empty())
		{
			map.emplace(std::make_pair(opengl::GLSLType::Float, createGLSLFloatAttribute));
			map.emplace(std::make_pair(opengl::GLSLType::Int,	createGLSLIntAttribute));
			map.emplace(std::make_pair(opengl::GLSLType::UInt,	createGLSLUIntAttribute));
			map.emplace(std::make_pair(opengl::GLSLType::Vec2,	createGLSLVec2Attribute));
			map.emplace(std::make_pair(opengl::GLSLType::Vec3,	createGLSLVec3Attribute));
			map.emplace(std::make_pair(opengl::GLSLType::Vec4,	createGLSLVec4Attribute));
			map.emplace(std::make_pair(opengl::GLSLType::Mat2,	createGLSLMat2Attribute));
			map.emplace(std::make_pair(opengl::GLSLType::Mat3,	createGLSLMat3Attribute));
			map.emplace(std::make_pair(opengl::GLSLType::Mat4,	createGLSLMat4Attribute));
			map.emplace(std::make_pair(opengl::GLSLType::Tex2D, createGLSLIntAttribute));		// TODO: THIS NEEDS TO AN ATTRIBUTE OBJECT LINK!
		}
		return map;
	}


	// The attribute type associated with a certain GLSL shader input type
	RTTI::TypeInfo getAttributeType(opengl::GLSLType type)
	{
		auto it = getGLSLAttributeMap().find(type);
		if (it == getGLSLAttributeMap().end())
		{
			nap::Logger::warn("unable to find attribute associated with GLSL type: %d", type);
			return RTTI::TypeInfo::empty();
		}
		return it->second;
	}


	// Return the attribute create function
	GLSLAttributeCreateFunction* getAttributeCreateFunction(opengl::GLSLType type)
	{
		auto it = getGLSLAttributeCreateMap().find(type);
		if (it == getGLSLAttributeCreateMap().end())
		{
			nap::Logger::warn("unable to find associated GLSL attribute create function for GLSL type: %d", type);
			return nullptr;
		}
		return &(it->second);
	}

} // End Namespace NAP

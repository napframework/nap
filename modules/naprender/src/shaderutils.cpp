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
	void setUniformFloat(const ShaderResource&, const AttributeBase&)			{ }
	void setUniformInt(const ShaderResource&,	const AttributeBase&)			{ }
	void setUniformUInt(const ShaderResource&,	const AttributeBase&)			{ }
	void setUniformVec2(const ShaderResource&,	const AttributeBase&)			{ }
	void setUniformVec3(const ShaderResource&,	const AttributeBase&)			{ }
	void setUniformVec4(const ShaderResource&,	const AttributeBase&)			{ }
	void setUniformMat2(const ShaderResource&,	const AttributeBase&)			{ }
	void setUniformMat3(const ShaderResource&,	const AttributeBase&)			{ }
	void setUniformMat4(const ShaderResource&,	const AttributeBase&)			{ }


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

} // End Namespace NAP

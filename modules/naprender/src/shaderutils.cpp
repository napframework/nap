// local includes
#include "shaderutils.h"
#include "shaderresource.h"

// External Includes
#include <unordered_map>
#include <shaderresource.h>
#include <imageresource.h>
#include <glm/gtc/type_ptr.hpp>
#include <nap/arrayattribute.h>

namespace nap
{
	// Set uniform float based attr
	void setUniformFloat(const opengl::UniformVariable& var, const AttributeBase& attr)			
	{
		assert(attr.getValueType() == RTTI_OF(float));
		if (var.isArray())
		{
			assert(false);
			return;
		}
		const Attribute<float>& attribute = static_cast<const Attribute<float>&>(attr);
		var.set((&attribute.getValue()));
	}

	// Set uniform int based on attr
	void setUniformInt(const opengl::UniformVariable& var, const AttributeBase& attr)				
	{
		assert(attr.getValueType() == RTTI_OF(int));
		if (var.isArray())
		{
			assert(false);
			return;
		}
		const Attribute<int>& attribute = static_cast<const Attribute<int>&>(attr);
		var.set((&attribute.getValue()));
	}

	// Set uniform unsigned int based on var
	void setUniformUInt(const opengl::UniformVariable& var, const AttributeBase& attr)			
	{
		assert(attr.getValueType() == RTTI_OF(nap::uint));
		if (var.isArray())
		{
			assert(false);
			return;
		}
		const Attribute<nap::uint>& attribute = static_cast<const Attribute<nap::uint>&>(attr);
		var.set((&attribute.getValue()));
	}
	
	// Set vec2 based on var
	void setUniformVec2(const opengl::UniformVariable& var, const AttributeBase& attr)			
	{
		assert(attr.getValueType() == RTTI_OF(glm::vec2));
		if (var.isArray())
		{
			assert(false);
			return;
		}

		const Attribute<glm::vec2>& attribute = static_cast<const Attribute<glm::vec2>&>(attr);
		var.set(glm::value_ptr(attribute.getValue()));
	}
	
	// set vec3 based on var
	void setUniformVec3(const opengl::UniformVariable& var, const AttributeBase& attr)			
	{
		assert(attr.getValueType() == RTTI_OF(glm::vec3));
		if (var.isArray())
		{
			assert(false);
			return;
		}

		const Attribute<glm::vec3>& attribute = static_cast<const Attribute<glm::vec3>&>(attr);
		var.set(glm::value_ptr(attribute.getValue()));
	}
	
	// Set vec4 based on var
	void setUniformVec4(const opengl::UniformVariable& var, const AttributeBase& attr)			
	{
		assert(attr.getValueType() == RTTI_OF(glm::vec4));
		if (var.isArray())
		{
			assert(false);
			return;
		}

		const Attribute<glm::vec4>& attribute = static_cast<const Attribute<glm::vec4>&>(attr);
		var.set(glm::value_ptr(attribute.getValue()));
	}

	// Set mat2 based on var
	void setUniformMat2(const opengl::UniformVariable& var, const AttributeBase& attr)			
	{
		assert(attr.getValueType() == RTTI_OF(glm::mat2x2));
		if (var.isArray())
		{
			assert(false);
			return;
		}

		const Attribute<glm::mat2x2>& attribute = static_cast<const Attribute<glm::mat2x2>&>(attr);
		var.set(glm::value_ptr(attribute.getValue()));
	}

	// Set uniform 3 based on var
	void setUniformMat3(const opengl::UniformVariable& var, const AttributeBase& attr)			
	{
		assert(attr.getValueType() == RTTI_OF(glm::mat3x3));
		if (var.isArray())
		{
			assert(false);
			return;
		}

		const Attribute<glm::mat3x3>& attribute = static_cast<const Attribute<glm::mat3x3>&>(attr);
		var.set(glm::value_ptr(attribute.getValue()));
	}

	// set mat4 based on var
	void setUniformMat4(const opengl::UniformVariable& var, const AttributeBase& attr)			
	{
		assert(attr.getValueType() == RTTI_OF(glm::mat4x4));
		if (var.isArray())
		{
			assert(false);
			return;
		}

		const Attribute<glm::mat4x4>& attribute = static_cast<const Attribute<glm::mat4x4>&>(attr);
		var.set(glm::value_ptr(attribute.getValue()));
	}

	/**
	 * Static iterative uniform attribute create function
	 */
	template<typename T>
	static AttributeBase& createGLSLAttribute(const opengl::UniformVariable& uvar, CompoundAttribute& compound, const T& defaultValue)
	{
		if (!uvar.isArray())
		{
			return compound.addAttribute<T>(uvar.mName.c_str(), defaultValue);
		}

		ArrayAttribute<T>& array_attr = compound.addArrayAttribute<T>(uvar.mName.c_str());
		for (int i = 0; i < uvar.mSize; i++)
		{
			std::string attr_name = stringFormat("%s_%d", uvar.mName.c_str(), i);
			array_attr.addAttribute(attr_name, defaultValue);
		}
		return array_attr;
	}


	/**
	 * All attribute create functions
	 */
	AttributeBase& createGLSLFloatAttribute(const opengl::UniformVariable& uvar, CompoundAttribute& compound)
	{
		return createGLSLAttribute<float>(uvar, compound, 1.0f);
	}

	// Create int attribute
	AttributeBase& createGLSLIntAttribute(const opengl::UniformVariable& uvar, CompoundAttribute& compound)
	{
		return createGLSLAttribute<int>(uvar, compound, 0);
	}

	// Create uint attribute
	AttributeBase& createGLSLUIntAttribute(const opengl::UniformVariable& uvar, CompoundAttribute& compound)
	{
		return createGLSLAttribute<nap::uint>(uvar, compound, 0);
	}

	// Create vec2 attribute
	AttributeBase& createGLSLVec2Attribute(const opengl::UniformVariable& uvar, CompoundAttribute& compound)
	{
		return createGLSLAttribute<glm::vec2>(uvar, compound, glm::vec2());
	}

	// Create vec3 attribute
	AttributeBase& createGLSLVec3Attribute(const opengl::UniformVariable& uvar, CompoundAttribute& compound)
	{
		return createGLSLAttribute<glm::vec3>(uvar, compound, glm::vec3());
	}

	// Create vec4 attribute
	AttributeBase& createGLSLVec4Attribute(const opengl::UniformVariable& uvar, CompoundAttribute& compound)
	{
		return createGLSLAttribute<glm::vec4>(uvar, compound, glm::vec4());
	}

	// Create matrix 2x2 attribute
	AttributeBase& createGLSLMat2Attribute(const opengl::UniformVariable& uvar, CompoundAttribute& compound)
	{
		return createGLSLAttribute<glm::mat2x2>(uvar, compound, glm::mat2x2());
	}

	// Create matrix 3x3 attribute
	AttributeBase& createGLSLMat3Attribute(const opengl::UniformVariable& uvar, CompoundAttribute& compound)
	{
		return createGLSLAttribute<glm::mat3x3>(uvar, compound, glm::mat3x3());
	}

	// Create matrix 4x4 attribute
	AttributeBase& createGLSLMat4Attribute(const opengl::UniformVariable& uvar, CompoundAttribute& compound)
	{
		return createGLSLAttribute<glm::mat4x4>(uvar, compound, glm::mat4x4());
	}

	// Create texture 2d attribute
	AttributeBase& createTexture2DAttribute(const opengl::UniformVariable& uvar, CompoundAttribute& compound)
	{
		if (uvar.isArray())
		{
			nap::Logger::warn("GLSL texture arrays are not supported");
		}
		return compound.addResourceLinkAttribute(uvar.mName, RTTI_OF(TextureResource));
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
			map.emplace(std::make_pair(opengl::GLSLType::Tex2D, RTTI_OF(TextureResource)));		// TODO: THIS NEEDS TO BECOME A LINK!
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
	const GLSLAttributeCreateMap& getGLSLAttributeCreateMap()
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
			map.emplace(std::make_pair(opengl::GLSLType::Tex2D, createTexture2DAttribute));		// TODO: THIS NEEDS TO AN ATTRIBUTE OBJECT LINK!
		}
		return map;
	}


	// The attribute type associated with a certain GLSL shader input type
	RTTI::TypeInfo getAttributeType(opengl::GLSLType type)
	{
		const nap::GLSLAttributeMap& map = getGLSLAttributeMap();
		auto it = map.find(type);
		if (it == map.end())
		{
			nap::Logger::warn("unable to find attribute associated with GLSL type: %d", type);
			return RTTI::TypeInfo::empty();
		}
		return it->second;
	}


	// Return the attribute create function
	const GLSLAttributeCreateFunction* getAttributeCreateFunction(opengl::GLSLType type)
	{
		const GLSLAttributeCreateMap& map = getGLSLAttributeCreateMap();
		auto it = map.find(type);
		if (it == map.end())
		{
			nap::Logger::warn("unable to find associated GLSL attribute create function for GLSL type: %d", type);
			return nullptr;
		}
		return &(it->second);
	}


	// Get glsl set function based on rtti attribute type
	const GLSLSetterFunction* getGLSLSetFunction(const RTTI::TypeInfo& type)
	{
		const nap::GLSLSetterMap& map = getGLSLSetterMap();
		auto it = map.find(type);
		if (it == map.end())
		{
			nap::Logger::warn("unable to find associated GLSL uniform set function for type: %s", type.getName().c_str());
			return nullptr;
		}
		return &(it->second);
	}

} // End Namespace NAP

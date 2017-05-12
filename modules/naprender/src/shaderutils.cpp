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
	/**
	 * Represents a uniform action that is associated with a specific opengl GLSL type
	 * This is a simple container used to bind a create and set function
	 */
	class GLSLUniformAction
	{
	public:
		GLSLUniformAction(const opengl::GLSLType type, GLSLAttributeCreateFunction createFunc, GLSLSetterFunction setterFunc) : mType(type),
			mCreateFunc(createFunc), mSetFunc(setterFunc) { }
		virtual ~GLSLUniformAction() = default;

		const GLSLAttributeCreateFunction* const	getAttributeCreateFunction()	{ return &mCreateFunc; }
		const GLSLSetterFunction* const				getSetFunction()				{ return &mSetFunc; }

	private:
		opengl::GLSLType							mType = opengl::GLSLType::Unknown;
		GLSLAttributeCreateFunction					mCreateFunc = nullptr;
		GLSLSetterFunction							mSetFunc = nullptr;
	};


	// Set uniform float based attr
	void setUniformFloat(const opengl::UniformVariable& var, const AttributeBase& attr, int& currentTexture)
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
	void setUniformInt(const opengl::UniformVariable& var, const AttributeBase& attr, int& currentTexture)
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
	void setUniformUInt(const opengl::UniformVariable& var, const AttributeBase& attr, int& currentTexture)
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
	void setUniformVec2(const opengl::UniformVariable& var, const AttributeBase& attr, int& currentTexture)
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
	void setUniformVec3(const opengl::UniformVariable& var, const AttributeBase& attr, int& currentTexture)
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
	void setUniformVec4(const opengl::UniformVariable& var, const AttributeBase& attr, int& currentTexture)
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
	void setUniformMat2(const opengl::UniformVariable& var, const AttributeBase& attr, int& currentTexture)
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
	void setUniformMat3(const opengl::UniformVariable& var, const AttributeBase& attr, int& currentTexture)
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
	void setUniformMat4(const opengl::UniformVariable& var, const AttributeBase& attr, int& currentTexture)
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

	// set texture 2D
	void setTexture2D(const opengl::UniformVariable& var, const AttributeBase& attr, int& currentTexture)
	{
		// Don't support arrays
		assert(attr.getValueType() == RTTI_OF(std::string));
		if (var.isArray())
		{
			assert(false);
			return;
		}

		// Make sure it's a resource link we're resolving
		if (!attr.get_type().is_derived_from(RTTI_OF(ResourceLinkAttribute)))
		{
			nap::Logger::warn("attribute is not of type: ResourceLinkAttribute");
			return;
		}

		// Check if the texture resource is linked
		const ResourceLinkAttribute& texture_resource_link = static_cast<const ResourceLinkAttribute&>(attr);
		nap::TextureResource* texture_resource = texture_resource_link.getResource<TextureResource>();
		if (texture_resource == nullptr)
		{
			nap::Logger::debug("no texture resource linked to uniform: %s", var.mName.c_str());
			return;
		}

		// Make sure we're setting a texture 2D
		if (texture_resource->getTexture().getTargetType() != GL_TEXTURE_2D)
		{
			nap::Logger::warn("linked texture: %s to uniform: %s is not of type texture 2D", texture_resource->getDisplayName().c_str(), var.mName.c_str());
			return;
		}

		// Set current active texture unit
		glActiveTexture(GL_TEXTURE0 + currentTexture);
		
		// Bind texture resource to texture unit
		texture_resource->bind();

		// Set shader binding to reflect current texture unit
		var.set(&currentTexture);

		// Increment texture for subsequent texture allocations
		currentTexture++;
	}

	/**
	 * Static iterative uniform attribute create function
	 */
	template<typename T>
	static AttributeBase& createGLSLAttribute(const opengl::UniformVariable& uvar, CompoundAttribute& compound, const T& defaultValue)
	{
		if (!uvar.isArray())
		{
			return compound.addAttribute<T>(defaultValue);
		}

		ArrayAttribute<T>& array_attr = compound.addArrayAttribute<T>(uvar.mName.c_str());
		for (int i = 0; i < uvar.mSize; i++)
		{
			array_attr.add(defaultValue);
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


	using GLSLUniformActionMap = std::unordered_map<opengl::GLSLType, std::unique_ptr<GLSLUniformAction>>;
	/**
	 * @return a map that contains all uniform actions for all supported glsl types
	 */
	const GLSLUniformActionMap& getGLSLUniformActionMap()
	{
		static GLSLUniformActionMap map;
		if (map.empty())
		{
			map.emplace(std::make_pair(opengl::GLSLType::Float, std::make_unique<GLSLUniformAction>(opengl::GLSLType::Float, createGLSLFloatAttribute, setUniformFloat)));
			map.emplace(std::make_pair(opengl::GLSLType::Int,	std::make_unique<GLSLUniformAction>(opengl::GLSLType::Int,	createGLSLIntAttribute, setUniformInt)));
			map.emplace(std::make_pair(opengl::GLSLType::UInt,	std::make_unique<GLSLUniformAction>(opengl::GLSLType::UInt, createGLSLUIntAttribute, setUniformUInt)));
			map.emplace(std::make_pair(opengl::GLSLType::Vec2,	std::make_unique<GLSLUniformAction>(opengl::GLSLType::Vec2, createGLSLVec2Attribute, setUniformVec2)));
			map.emplace(std::make_pair(opengl::GLSLType::Vec3,	std::make_unique<GLSLUniformAction>(opengl::GLSLType::Vec3, createGLSLVec3Attribute, setUniformVec3)));
			map.emplace(std::make_pair(opengl::GLSLType::Vec4,	std::make_unique<GLSLUniformAction>(opengl::GLSLType::Vec4, createGLSLVec4Attribute, setUniformVec4)));
			map.emplace(std::make_pair(opengl::GLSLType::Mat2,	std::make_unique<GLSLUniformAction>(opengl::GLSLType::Mat2, createGLSLMat2Attribute, setUniformMat2)));
			map.emplace(std::make_pair(opengl::GLSLType::Mat3,	std::make_unique<GLSLUniformAction>(opengl::GLSLType::Mat3, createGLSLMat3Attribute, setUniformMat3)));
			map.emplace(std::make_pair(opengl::GLSLType::Mat4,	std::make_unique<GLSLUniformAction>(opengl::GLSLType::Mat4, createGLSLMat4Attribute, setUniformMat4)));
			map.emplace(std::make_pair(opengl::GLSLType::Tex2D, std::make_unique<GLSLUniformAction>(opengl::GLSLType::Tex2D, createTexture2DAttribute, setTexture2D)));
		}
		return map;
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
		const GLSLUniformActionMap& map = getGLSLUniformActionMap();
		auto it = map.find(type);
		if (it == map.end())
		{
			nap::Logger::warn("unable to find associated GLSL attribute create function for GLSL type: %d", type);
			return nullptr;
		}
		return it->second->getAttributeCreateFunction();
	}


	// Get glsl set function based on rtti attribute type
	const GLSLSetterFunction* getGLSLSetFunction(const opengl::GLSLType& type)
	{
		const nap::GLSLUniformActionMap& map = getGLSLUniformActionMap();
		auto it = map.find(type);
		if (it == map.end())
		{
			nap::Logger::warn("unable to find associated GLSL uniform set function for type: %d", type);
			return nullptr;
		}
		return it->second->getSetFunction();
	}

	// Reset active texture unit
	void resetActiveTexture()
	{
		glActiveTexture(GL_TEXTURE0);
	}

} // End Namespace NAP

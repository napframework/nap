#pragma once

// Local Includes
#include "shadervariabledeclarations.h"
#include "valuegpubuffer.h"

// External Includes
#include <rtti/objectptr.h>
#include <glm/glm.hpp>
#include <utility/dllexport.h>
#include <nap/resource.h>

namespace nap
{
	// Forward Declares
	class UniformInstance;

	using UniformCreatedCallback = std::function<void()>;

	/**
	 * Shader uniform resource base class.
	 */
	class NAPAPI Uniform : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		std::string mName;		///< Name of uniform in shader
	};


	/**
	 * Contains other uniforms, including: values, structs and arrays.
	 * myshader.frag example:
	 * 
	 * ~~~~~{.cpp}
	 *	struct PointLight
	 *	{
	 *		vec3		mPosition;
	 *		vec3 		mIntensity;
	 *	};
	 * ~~~~~
	 */
	class NAPAPI UniformStruct : public Uniform
	{
		RTTI_ENABLE(Uniform)
	public:

		/**
		 * Adds a uniform.
		 * @param uniform the uniform to add
		 */
		void addUniform(Uniform& uniform);

		/**
		 * @param name the name of the uniform to find.
		 * @return a uniform with the given name, nullptr if not found
		 */
		Uniform* findUniform(const std::string& name);

		/**
		 * @param name the name of the uniform to find.
		 * @return a uniform with the given name, nullptr if not found
		 */
		const Uniform* findUniform(const std::string& name) const;

	public:
		std::vector<rtti::ObjectPtr<Uniform>> mUniforms;
	};
	

	/**
	 * A list of uniform structs. 
	 * The list must be declared inside a uniform buffer object (block).
	 * myshader.frag example:
	 *
	 * ~~~~~{.cpp}
	 *	struct PointLight						///< Point light structure
	 *	{
	 *		vec3		mPosition;
	 *		vec3 		mIntensity;
	 *	};
	 * 
	 *	uniform UBO								///< Uniform buffer object
	 *	{
	 *		uniform PointLight lights[10];		///< 10 lights
	 *	} ubo;
	 * ~~~~~
	 */
	class NAPAPI UniformStructArray : public Uniform
	{
		RTTI_ENABLE(Uniform)
	public:
		/**
		 * Inserts a structure at the given index.
		 * @param index index to insert struct.
		 * @param uniformStruct the struct to insert
		 */
		void insertStruct(int index, UniformStruct& uniformStruct);

	public:
		std::vector<rtti::ObjectPtr<UniformStruct>> mStructs;
	};


	/**
	 * Uniform value base class.
	 */
	class NAPAPI UniformValue : public Uniform
	{
		RTTI_ENABLE(Uniform)
	};


	/**
	 * Specific type of uniform value, for example: float, vec2, vec3, int etc.
	 * All values must be declared inside a uniform buffer object (block).
	 * myshader.frag example:
	 *
	 * ~~~~~{.cpp}
	 *	uniform UBO								///< Uniform buffer object
	 *	{
	 *		vec3 color;							///< object color uniform
	 *		float length;						///< hair length uniform
	 *	} ubo;
	 * ~~~~~
	 */
	template<typename T>
	class TypedUniformValue : public UniformValue
	{
		RTTI_ENABLE(UniformValue)
	public:
		T mValue = T();
	};


	/**
	 * Base class for list of uniform values.
	 * The list must be declared inside a uniform buffer object (block).
	 * myshader.frag example:
	 *
	 * ~~~~~{.cpp}
	 *	uniform UBO								///< Uniform buffer object
	 *	{
	 *		uniform vec3 positions[10];			///< List of object positions
	 *	} ubo;
	 * ~~~~~
	 */
	class NAPAPI UniformValueArray : public UniformValue
	{
		RTTI_ENABLE(UniformValue)

	public:
		/**
		 * @return The number of elements in this array
		 */
		virtual int getCount() const = 0;
	};


	/**
	 * List of uniform values, for example: float[3], vec3[10] etc.
	 * The list must be declared inside a uniform buffer object (block).
	 * myshader.frag example:
	 * 
	 * ~~~~~{.cpp}
	 *	uniform UBO								///< Uniform buffer object
	 *	{
	 *		uniform vec3 positions[10];			///< List of object position
	 *	} ubo;
	 * ~~~~~
	 */
	template<typename T>
	class TypedUniformValueArray : public UniformValueArray
	{
		RTTI_ENABLE(UniformValueArray)
	public:
		/**
		 * @return total number of elements.
		 */
		virtual int getCount() const override { return mValues.size(); }
		std::vector<T> mValues;
	};


	/**
	 * Find a shader uniform based on the given shader variable declaration.
	 * @param members uniforms of type nap::Uniform to search through.
	 * @param declaration uniform declaration to match
	 * @return uniform that matches with the given shader declaration, nullptr if not found.
	 */
	template<class T>
	const Uniform* findUniformStructMember(const std::vector<T>& members, const ShaderVariableDeclaration& declaration)
	{
		for (auto& member : members)
			if (member->mName == declaration.mName)
				return member.get();
		return nullptr;
	}


	//////////////////////////////////////////////////////////////////////////
	// Uniform value type definitions
	//////////////////////////////////////////////////////////////////////////
	
	using UniformUInt						= TypedUniformValue<uint>;
	using UniformInt						= TypedUniformValue<int>;
	using UniformFloat						= TypedUniformValue<float>;
	using UniformVec2						= TypedUniformValue<glm::vec2>;
	using UniformVec3						= TypedUniformValue<glm::vec3>;
	using UniformVec4						= TypedUniformValue<glm::vec4>;
	using UniformMat4						= TypedUniformValue<glm::mat4>;


	//////////////////////////////////////////////////////////////////////////
	// Uniform value array type definitions
	//////////////////////////////////////////////////////////////////////////

	using UniformUIntArray					= TypedUniformValueArray<uint>;
	using UniformIntArray					= TypedUniformValueArray<int>;
	using UniformFloatArray					= TypedUniformValueArray<float>;
	using UniformVec2Array					= TypedUniformValueArray<glm::vec2>;
	using UniformVec3Array					= TypedUniformValueArray<glm::vec3>;
	using UniformVec4Array					= TypedUniformValueArray<glm::vec4>;
	using UniformMat4Array					= TypedUniformValueArray<glm::mat4>;
}

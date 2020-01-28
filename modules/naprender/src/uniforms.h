#pragma once

// Local Includes
#include "nuniformdeclarations.h"

// External Includes
#include <rtti/objectptr.h>
#include <glm/glm.hpp>
#include <utility/dllexport.h>
#include <nap/resource.h>

namespace nap
{
	class Texture2D;
	class UniformInstance;

	using UniformCreatedCallback = std::function<void()>;

	/**
	 * Base class for all types of uniforms, whether texture or value.
	 */
	class NAPAPI Uniform : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		std::string mName;		///< Name of uniform as in shader
	};

	/**
	 * Represents a 'struct' uniform. A struct can contain other uniforms, but does not directly map onto a uniform in the shader
	 */
	class NAPAPI UniformStruct : public Uniform
	{
		RTTI_ENABLE(Uniform)
	public:

		void addUniform(Uniform& uniform);
		Uniform* findUniform(const std::string& name);

	public:
		std::vector<rtti::ObjectPtr<Uniform>> mUniforms;
	};
	
	/**
	 * Represents an 'array of structures' uniform. A UniformStructArray can contain UniformStructs, but does not directly map onto a uniform in the shader
	 */
	class NAPAPI UniformStructArray : public Uniform
	{
		RTTI_ENABLE(Uniform)
	public:
		void insertStruct(int index, UniformStruct& uniformStruct);

	public:
		std::vector<rtti::ObjectPtr<UniformStruct>> mStructs;
	};

	/**
	* Represents a 'value' uniform, or: something that is not a texture.
	* Derived classes should store value data and implement push() to update the
	* value in the shader.
	*/
	class NAPAPI UniformValue : public Uniform
	{
		RTTI_ENABLE(Uniform)
	};

	template<typename T>
	class TypedUniformValue : public UniformValue
	{
		RTTI_ENABLE(UniformValue)
	
	public:
		T mValue = T();			///< Data storage
	};

	/**
	* Represents an array of 'value' uniforms.
	* Derived classes should return the correct amount of elements that are in the array.
	*/
	class NAPAPI UniformValueArray : public UniformValue
	{
		RTTI_ENABLE(UniformValue)

	public:
		/**
		 * Retrieve the number of elements in this array
		 *
		 * @return The number of elements in this array
		 */
		virtual int getCount() const = 0;
	};

	template<typename T>
	class TypedUniformValueArray : public UniformValueArray
	{
		RTTI_ENABLE(UniformValueArray)

		virtual int getCount() const override { return mValues.size(); }

	public:
		std::vector<T> mValues;
	};

		template<class T>
	const Uniform* findUniformStructMember(const std::vector<T>& members, const opengl::UniformDeclaration& declaration)
	{
		for (auto& member : members)
			if (member->mName == declaration.mName)
				return member.get();

		return nullptr;
	}

	using UniformInt = TypedUniformValue<int>;
	using UniformFloat = TypedUniformValue<float>;
	using UniformVec2 = TypedUniformValue<glm::vec2>;
	using UniformVec3 = TypedUniformValue<glm::vec3>;
	using UniformVec4 = TypedUniformValue<glm::vec4>;
	using UniformMat4 = TypedUniformValue<glm::mat4>;

	using UniformIntArray = TypedUniformValueArray<int>;
	using UniformFloatArray = TypedUniformValueArray<float>;
	using UniformVec2Array = TypedUniformValueArray<glm::vec2>;
	using UniformVec3Array = TypedUniformValueArray<glm::vec3>;
	using UniformVec4Array = TypedUniformValueArray<glm::vec4>;
	using UniformMat4Array = TypedUniformValueArray<glm::mat4>;
}

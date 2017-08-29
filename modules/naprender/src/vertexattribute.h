#pragma once

#include <utility/dllexport.h>
#include "rtti/rttiobject.h"
#include "glm/glm.hpp"
#include "gl/glew.h"

namespace nap
{
	class NAPAPI VertexAttribute : public rtti::RTTIObject
	{
		RTTI_ENABLE(rtti::RTTIObject)
	public:		
		VertexAttribute();

		virtual void* getData() = 0;
		virtual GLenum getType() const = 0;
		virtual int getNumComponents() const = 0;

		std::string			mAttributeID;
	};

	template<typename ELEMENTTYPE>
	class TypedVertexAttribute : public VertexAttribute
	{
		RTTI_ENABLE(VertexAttribute)
	public:
		const std::vector<ELEMENTTYPE>& getData() const { return mData;  }

		void reserve(size_t numElements)
		{
			mData.reserve(numElements);
		}

		void add(const ELEMENTTYPE& element)
		{
			mData.push_back(element);
		}

		void setData(const ELEMENTTYPE* elements, int numElements)
		{
			mData.resize(numElements);
			memcpy(mData.data(), elements, numElements * sizeof(ELEMENTTYPE));
		}

		virtual void* getData() override
		{
			return static_cast<void*>(mData.data());
		}

		virtual GLenum getType() const override { assert(false); return GL_FLOAT; } // Not implemented for this type

		virtual int getNumComponents() const override { assert(false); return -1; } // Not implemented for this type

		std::vector<ELEMENTTYPE>	mData;
	};

	using FloatMeshAttribute	= TypedVertexAttribute<float>;
	using IntMeshAttribute		= TypedVertexAttribute<int>;
	using ByteMeshAttribute		= TypedVertexAttribute<int8_t>;
	using DoubleMeshAttribute	= TypedVertexAttribute<double>;
	using Vec2MeshAttribute		= TypedVertexAttribute<glm::vec2>;
	using Vec3MeshAttribute		= TypedVertexAttribute<glm::vec3>;
	using Vec4MeshAttribute		= TypedVertexAttribute<glm::vec4>;

	template<>
	GLenum FloatMeshAttribute::getType() const { return GL_FLOAT; }

	template<>
	int FloatMeshAttribute::getNumComponents() const { return 1; }

	template<>
	GLenum IntMeshAttribute::getType() const { return GL_INT; }

	template<>
	int IntMeshAttribute::getNumComponents() const { return 1; }

	template<>
	GLenum ByteMeshAttribute::getType() const { return GL_BYTE; }

	template<>
	int ByteMeshAttribute::getNumComponents() const { return 1; }

	template<>
	GLenum DoubleMeshAttribute::getType() const { return GL_DOUBLE; }

	template<>
	int DoubleMeshAttribute::getNumComponents() const { return 1; }

	template<>
	GLenum Vec2MeshAttribute::getType() const { return GL_FLOAT; }

	template<>
	int Vec2MeshAttribute::getNumComponents() const { return 2; }

	template<>
	GLenum Vec3MeshAttribute::getType() const { return GL_FLOAT; }

	template<>
	int Vec3MeshAttribute::getNumComponents() const { return 3; }

	template<>
	GLenum Vec4MeshAttribute::getType() const { return GL_FLOAT; }

	template<>
	int Vec4MeshAttribute::getNumComponents() const { return 4; }

} // nap


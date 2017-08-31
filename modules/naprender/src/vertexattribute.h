#pragma once

#include <utility/dllexport.h>
#include "rtti/rttiobject.h"
#include "glm/glm.hpp"
#include "GL/glew.h"

namespace nap
{
	/**
	 * Base class for vertex attribute. Describes what kind of data will be present in the attribute.
	 * This base class is necessary to have a type independent way to update the GPU meshes.
	 */
	class NAPAPI VertexAttribute : public rtti::RTTIObject
	{
		RTTI_ENABLE(rtti::RTTIObject)
	public:		
		VertexAttribute();
		
		/**
		 * @return Should return type-less data for the vertex attribute buffer. This is a type independent way to update
		 * meshes to the GPU.
		 */
		virtual void* getData() = 0;

		/**
		 * @return Should return GL type of the attribute. This is used during construction of the vertex attributes on the GPU.
		 */
		virtual GLenum getType() const = 0;
		
		/**
		 * @return Should return number of components per value. For instance, float has one attribute, a vec3 has three components. 
		 * This is used during construction of the vertex attributes on the GPU.
		 */
		virtual int getNumComponents() const = 0;

		/**
		 * @return Should return amount of element in the buffer.
		 */
		virtual int getNumElements() const = 0;

		std::string			mAttributeID;		///< Name/ID of the attribute
	};


	/**
	 * Typed interface for vertex attributes. 
	 * CPU data can be read by using getValues().
	 * CPU data can be updated by using the various other functions. Make sure to call MeshInstance::update() after edits are performed.
	 * 
	 * Notice that all known vertex attribute types have specializations for GetType and GetNumComponents.
	 */
	template<typename ELEMENTTYPE>
	class TypedVertexAttribute : public VertexAttribute
	{
		RTTI_ENABLE(VertexAttribute)
	public:

		/**
		 * @return Types interface toward the internal values. Use this function to read CPU data.
		 */
		const std::vector<ELEMENTTYPE>& getValues() const { return mData;  }

		/**
		 * @return Types interface toward the internal values. Use this function to read CPU data.
		 */
		std::vector<ELEMENTTYPE>& getValues()	{ return mData; }

		/**
		 * Reserves an amount of memory.
		 */
		void reserve(size_t numElements)		{ mData.reserve(numElements); }

		/**
		 * Adds a single element to the buffer.
		 */
		void add(const ELEMENTTYPE& element)	{ mData.push_back(element); }

		/**
		 * Sets the entire vertex attribute buffer.
		 * @param elements Pointer to the elements to copy.
		 * @param numElements Amount of elements in @elements to copy.
		 */
		void setData(const ELEMENTTYPE* elements, int numElements)
		{
			mData.resize(numElements);
			memcpy(mData.data(), elements, numElements * sizeof(ELEMENTTYPE));
		}

		/**
		 * Default implementation. If this is called, the proper specialization is not properly implemented.
		 */
		virtual GLenum getType() const override;

		/**
		 * Default implementation. If this is called, the proper specialization is not properly implemented.
		 */
		virtual int getNumComponents() const override;

		/**
		 * @return Should return amount of element in the buffer.
		 */
		virtual int getNumElements() const override { return mData.size(); }

		std::vector<ELEMENTTYPE>	mData;		///< Actual typed data of the attribute

	protected:

		/**
		 * Implementation for all typed buffer, returns data.
		 */
		virtual void* getData() override
		{
			return static_cast<void*>(mData.data());
		}
	};

	// Type definitions for all supported vertex attribute types
	using FloatVertexAttribute	= TypedVertexAttribute<float>;
	using IntVertexAttribute	= TypedVertexAttribute<int>;
	using ByteVertexAttribute	= TypedVertexAttribute<int8_t>;
	using DoubleVertexAttribute	= TypedVertexAttribute<double>;
	using Vec2VertexAttribute	= TypedVertexAttribute<glm::vec2>;
	using Vec3VertexAttribute	= TypedVertexAttribute<glm::vec3>;
	using Vec4VertexAttribute	= TypedVertexAttribute<glm::vec4>;
} // nap


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
	class NAPAPI BaseVertexAttribute : public rtti::RTTIObject
	{
		RTTI_ENABLE(rtti::RTTIObject)
	public:		
		BaseVertexAttribute();
		
		/**
		 * @return Should return type-less data for the vertex attribute buffer. This is a type independent way to update
		 * meshes to the GPU.
		 */
		virtual void* getRawData() = 0;

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
		virtual int getCount() const = 0;

		/**
		 * @return The internally allocated memory size.
		 */
		virtual size_t getCapacity() const = 0;

		/**
		 * Reserves CPU memory for the buffer. 
		 */
		virtual void reserve(size_t numElements) = 0;

		std::string			mAttributeID;		///< Name/ID of the attribute
	};


	//////////////////////////////////////////////////////////////////////////

	/**
	 * Typed interface for vertex attributes. 
	 * CPU data can be read by using getValues().
	 * CPU data can be updated by using the various other functions. Make sure to call MeshInstance::update() after edits are performed.
	 * 
	 * Notice that all known vertex attribute types have specializations for GetType and GetNumComponents.
	 */
	template<typename ELEMENTTYPE>
	class VertexAttribute : public BaseVertexAttribute
	{
		RTTI_ENABLE(BaseVertexAttribute)
	public:

		/**
		 * Reserves an amount of memory to hold @number amount of elements
		 */
		virtual void reserve(size_t numElements) override		{ mData.reserve(numElements); }

		/**
		 * @return The internally allocated memory size.
		 */
		virtual size_t getCapacity() const override				{ return mData.capacity(); }

		/**
		 *	Resizes the data container to hold @number amount of elements
		 */
		void resize(size_t numElements)							{ mData.resize(numElements); }

		/**
		 * Clears all data associated with this attribute
		 */
		void clear()											{ mData.clear(); }

		/**
		* @return Types interface toward the internal values. Use this function to read CPU data.
		*/
		const std::vector<ELEMENTTYPE>& getData() const			{ return mData; }

		/**
		* @return Types interface toward the internal values. Use this function to read CPU data.
		*/
		std::vector<ELEMENTTYPE>& getData()						{ return mData; }

		/**
		 * Adds a single element to the buffer.
		 */
		void addData(const ELEMENTTYPE& element)				{ mData.emplace_back(element); }

		/**
		 * Adds data to the existing data in the buffer.
		 * @param elements Pointer to the elements to copy.
		 * @param numElements Amount of elements in @elements to copy.
		 */
		void addData(const ELEMENTTYPE* elements, int numElements);

		/**
		 * Sets the internal values based on the contained type
		 * @param values: the values that will be copied over
		 */
		void setData(std::vector<ELEMENTTYPE>& values)			{ setData(&(values.front()), values.size()); }

		/**
		 * Sets the entire vertex attribute buffer.
		 * @param elements Pointer to the elements to copy.
		 * @param numElements Amount of elements in @elements to copy.
		 */
		void setData(const ELEMENTTYPE* elements, int numElements);

		/**
		 * @return the opengl type associated with this vertex attribute
		 * Note that this only works for specialized types such as float, int etc.
		 * Refer to the type definitions for supported vertex attribute types
		 */
		virtual GLenum getType() const override;

		/**
		 * @return the number of components associated with this vertex attribute, 1 for float, 3 for vec3 etc.
		 * Note that this only works for supported specialized types such as float, int etc.
		 * Refer to the type definitions for supported vertex attribute types
		 */
		virtual int getNumComponents() const override;

		/**
		 * @return the number vertices in the buffer
		 */
		virtual int getCount() const override					{ return static_cast<int>(mData.size()); }

		std::vector<ELEMENTTYPE>	mData;						///< Actual typed data of the attribute

	protected:
		/**
		 * Implementation for all typed buffer, returns data.
		 */
		virtual void* getRawData() override;
	};


	//////////////////////////////////////////////////////////////////////////
	// Type definitions for all supported vertex attribute types
	//////////////////////////////////////////////////////////////////////////

	using FloatVertexAttribute	= VertexAttribute<float>;
	using IntVertexAttribute	= VertexAttribute<int>;
	using ByteVertexAttribute	= VertexAttribute<int8_t>;
	using DoubleVertexAttribute	= VertexAttribute<double>;
	using Vec2VertexAttribute	= VertexAttribute<glm::vec2>;
	using Vec3VertexAttribute	= VertexAttribute<glm::vec3>;
	using Vec4VertexAttribute	= VertexAttribute<glm::vec4>;


	//////////////////////////////////////////////////////////////////////////
	// Template Definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename ELEMENTTYPE>
	void nap::VertexAttribute<ELEMENTTYPE>::setData(const ELEMENTTYPE* elements, int numElements)
	{
		mData.resize(numElements);
		memcpy(mData.data(), elements, numElements * sizeof(ELEMENTTYPE));
	}

	template<typename ELEMENTTYPE>
	void nap::VertexAttribute<ELEMENTTYPE>::addData(const ELEMENTTYPE* elements, int numElements)
	{
		int cur_num_elements = mData.size();
		mData.resize(cur_num_elements + numElements);
		memcpy((void*)&mData[cur_num_elements], elements, numElements * sizeof(ELEMENTTYPE));
	}

	template<typename ELEMENTTYPE>
	void* nap::VertexAttribute<ELEMENTTYPE>::getRawData()
	{
		return static_cast<void*>(mData.data());
	}


	//////////////////////////////////////////////////////////////////////////
	// Forward declarations of templated functions
	//////////////////////////////////////////////////////////////////////////

	template<>
	NAPAPI GLenum FloatVertexAttribute::getType() const;

	template<>
	NAPAPI int FloatVertexAttribute::getNumComponents() const;

	template<>
	NAPAPI GLenum IntVertexAttribute::getType() const;

	template<>
	NAPAPI int IntVertexAttribute::getNumComponents() const;

	template<>
	NAPAPI GLenum ByteVertexAttribute::getType() const;

	template<>
	NAPAPI int ByteVertexAttribute::getNumComponents() const;

	template<>
	NAPAPI GLenum DoubleVertexAttribute::getType() const;

	template<>
	NAPAPI int DoubleVertexAttribute::getNumComponents() const;

	template<>
	NAPAPI GLenum Vec2VertexAttribute::getType() const;

	template<>
	NAPAPI int Vec2VertexAttribute::getNumComponents() const;

	template<>
	NAPAPI GLenum Vec3VertexAttribute::getType() const;

	template<>
	NAPAPI int Vec3VertexAttribute::getNumComponents() const;

	template<>
	NAPAPI GLenum Vec4VertexAttribute::getType() const;

	template<>
	NAPAPI int Vec4VertexAttribute::getNumComponents() const;

} // nap


/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <utility/dllexport.h>
#include <nap/resource.h>
#include <glm/glm.hpp>
#include <nap/numeric.h>

#include "vulkan/vulkan_core.h"

namespace nap
{
	/**
	 * Base class for vertex attribute. Describes what kind of data will be present in the attribute.
	 * This base class is necessary to have a type independent way to update the GPU meshes.
	 */
	class NAPAPI BaseVertexAttribute : public Resource
	{
		RTTI_ENABLE(Resource)
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
		virtual VkFormat getFormat() const = 0;
		
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
	 * Notice that all known vertex attribute types have specializations for GetType and GetNumComponents.
	 */
	template<typename ELEMENTTYPE>
	class VertexAttribute : public BaseVertexAttribute
	{
		RTTI_ENABLE(BaseVertexAttribute)
	public:

		/**
		 * Reserves a certain amount of CPU memory to hold the given number of elements
		 * @param numElements the number of elements to reserve memory for.
		 */
		virtual void reserve(size_t numElements) override		{ mData.reserve(numElements); }

		/**
		 * @return The internally allocated memory size.
		 */
		virtual size_t getCapacity() const override				{ return mData.capacity(); }

		/**
		 * Resizes the data container to hold the given number of elements.
		 * @param numElements the new number of elements.
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
		 * Adds a single element to the end of the buffer. Data is copied.
		 * @param element to add.
		 */
		void addData(const ELEMENTTYPE& element)				{ mData.emplace_back(element); }

		/**
		 * Adds data to the existing data in the buffer. Data is copied.
		 * @param elements pointer to the elements to add.
		 * @param numElements number of elements in buffer to add.
		 */
		void addData(const ELEMENTTYPE* elements, int numElements);

		/**
		 * Sets the internal values based on the contained type
		 * @param values: the values that will be copied over
		 */
		void setData(const std::vector<ELEMENTTYPE>& values)			{ setData(&(values.front()), values.size()); }

		/**
		 * Sets the entire vertex attribute buffer.
		 * @param elements pointer to the elements to copy.
		 * @param numElements amount of elements to copy.
		 */
		void setData(const ELEMENTTYPE* elements, int numElements);

		virtual VkFormat getFormat() const override;

		/**
		 * @return the number vertices in the buffer
		 */
		virtual int getCount() const override					{ return static_cast<int>(mData.size()); }

		/**
		 * Array subscript overload
		 * @param index the index of the attribute value
		 * @return the vertex attribute value at index
		 */
		ELEMENTTYPE& operator[](std::size_t index)				{ return mData[index]; }
		
		/**
		 * Const array subscript overload
		 * @param index the index of the attribute value
		 * @return the vertex attribute value at index
		 */
		const ELEMENTTYPE& operator[](std::size_t index) const	{ return mData[index]; }


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

	using UIntVertexAttribute	= VertexAttribute<uint>;
	using IntVertexAttribute	= VertexAttribute<int>;
	using FloatVertexAttribute	= VertexAttribute<float>;
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
	NAPAPI VkFormat UIntVertexAttribute::getFormat() const;

	template<>
	NAPAPI VkFormat IntVertexAttribute::getFormat() const;

	template<>
	NAPAPI VkFormat FloatVertexAttribute::getFormat() const;

	template<>
	NAPAPI VkFormat Vec2VertexAttribute::getFormat() const;

	template<>
	NAPAPI VkFormat Vec3VertexAttribute::getFormat() const;

	template<>
	NAPAPI VkFormat Vec4VertexAttribute::getFormat() const;

} // nap


	#pragma once

#include <utility/dllexport.h>
#include <vertexattribute.h>
#include <mesh.h>
#include <glm/glm.hpp>
#include <array>

namespace nap
{
	class MeshInstance;
	class MeshShape;

	/**
	 * Data associated with a single triangle.
	 * The data is extracted from a VertexAttribute using the Triangle interface or can be constructed manually.
	 */
	template<class T>
	class TriangleData
	{
	public:
		TriangleData(const T& first, const T& second, const T& third) : 
        mData({{first, second, third }} )
		{ }

		TriangleData() = default;

		/**
		 * @return The data belonging to the first vertex of the triangle
		 */
		inline const T& first() const						{ return mData[0]; }
		
		/**
		 * Set the data belonging to the first vertex of the triangle
		 * @param data data associated with the first vertex
		 */
		inline void setFirst(const T& data)					{ mData[0] = data; }

		/**
		 * @return The data belonging to the second vertex of the triangle
		 */
		inline const T& second() const						{ return mData[1]; }
		
		/**
		 * Set the data belonging to the second vertex of the triangle
		 * @param data data associated with the second vertex
		 */
		inline void setSecond(const T& data)				{ mData[1] = data; }

		/**
		 * @return The data belonging to the third vertex of the triangle
		 */
		inline const T& third() const						{ return mData[2]; }

		/**
		 * Set the data belonging to the third vertex of the triangle
		 * @param data data associated with the third vertex
		 */
		inline void setThird(const T& data) const			{ mData[2] = data; }

		/**
		* Array subscript overload
		* @param index the index of the attribute value
		* @return the vertex attribute value at index
		*/
		T& operator[](std::size_t index)					{ return mData[index]; }

		/**
		* Const array subscript overload
		* @param index the index of the attribute value
		* @return the vertex attribute value at index
		*/
		const T& operator[](std::size_t index) const		{ return mData[index]; }

	private:
		std::array<T, 3> mData;
	};


	/**
	 * Contains the indices of a triangle associated with a specific nap::MeshShape inside a nap::MeshInstance
	 */
	class ShapeTriangle
	{
	public:
		using IndexArray = std::array<int, 3>;

		ShapeTriangle(int triangleIndex, int index0, int index1, int index2);

		/**
		 * @return The index of the triangle in the shape
		 */
		int getTriangleIndex() const						{ return mTriangleIndex; }

		/**
		 * @return The indices of this triangle
		 */
		IndexArray indices() const							{ return mIndices; }

		/**
		 * @return The index of the first vertex of this triangle
		 */
		int firstIndex() const								{ return mIndices[0]; }
		
		/**
		 * @return The index of the second vertex of this triangle
		 */
		int secondIndex() const								{ return mIndices[1]; }

		/**
		 * @return The index of the third vertex of this triangle
		 */
		int thirdIndex() const								{ return mIndices[2]; }	

		/**
		 * @return The index of the vertex at the specified index
		 */
		int operator[](std::size_t index) const				{ return mIndices[index]; }

		IndexArray::const_iterator begin()					{ return mIndices.begin(); }
		IndexArray::const_iterator end()					{ return mIndices.end(); }

		/**
		 * Returns the triangle vertex data of a specific attribute, for example the triangle position, color or normal.
		 * @param attribute The vertex attribute to retrieve vertex data for.
		 * @return the vertex data data of the given attribute, corresponding to the indices in this triangle.
		 */
		template<class T>
		TriangleData<T> getVertexData(const VertexAttribute<T>& attribute) const
		{
			return TriangleData<T>(attribute[mIndices[0]], attribute[mIndices[1]], attribute[mIndices[2]]);
		}

		/**
		 * Sets the triangle vertex data of a specific attribute, for example the triangle position, color or normal.
		 * @param attribute The VertexAttribute to set data to
		 * @param firstValue The value to set to the first vertex of the triangle
		 * @param secondValue The value to set to the second vertex of the triangle
		 * @param thirdValue The value to set to the third vertex of the triangle
		 */
		template<class T>
		void setVertexData(VertexAttribute<T>& attribute, const T& firstValue, const T& secondValue, const T& thirdValue)
		{
			attribute[mIndices[0]] = firstValue;
			attribute[mIndices[1]] = secondValue;
			attribute[mIndices[2]] = thirdValue;
		}

		/**
		 * @param attribute The VertexAttribute to set data to
		 * @param value The value to set to all vertices of the triangle
		 */
		template<class T>
		void setVertexData(VertexAttribute<T>& attribute, const T& value)
		{
			setVertexData(attribute, value, value, value);
		}

		/**
		 * @param attribute The VertexAttribute to set data to
		 * @param value the triangle data to set
		 */
		template<class T>
		void setVertexData(VertexAttribute<T>& attribute, const TriangleData<T>& value)
		{
			setVertexData(value[0], value[1], value[2]);
		}

	private:
		IndexArray mIndices;
		int mTriangleIndex;
	};


	/**
	 * Contains the indices of a triangle associated with a MeshInstance.
	 * The indices are always bound to a specific shape.
	 */
	class Triangle : public ShapeTriangle
	{
	public:
		/**
		 * @return The index of the shape within the MeshInstance this triangle belongs to
		 */
		int getShapeIndex() const { return mShapeIndex; }

	private:
		friend class TriangleIterator;

		Triangle(int shapeIndex, const ShapeTriangle& shapeTriangle);

	private:
		int mShapeIndex;
	};


	/**
	 * A ShapeTriangleIterator can be used to iterate through all triangles in a MeshShape.
	 * The ShapeTriangleIterator itself only serves as a base class; derived classes are provided that deal with differing index layouts depending on the type of the mesh.
	 */
	class NAPAPI ShapeTriangleIterator
	{
	public:
		ShapeTriangleIterator(const MeshShape& shape, int startIndex);
		virtual ~ShapeTriangleIterator()	{}

		/**
		 * Checks whether this iterator is done iterating. next() should only be called while this function returns false.
		 * @return true if done iterating, false if not
		 */
		bool isDone() const	{ return mCurrentIndex >= mIndexEnd; }

		/**
		 * Retrieves the indices of the next triangle in the MeshShape. Should only be called while isDone() returns false.
		 * @return the 3 indices of the next triangle in a glm::ivec3
		 */
		virtual const ShapeTriangle next() = 0;

	protected:
		const uint32*	mCurrentIndex;		///< The current position in the index buffer of the shape that we're iterating through
		int mCurrentTriangle = 0;
	
	private:
		const uint32*	mIndexEnd;			///< End of the index buffer of the shape that we're iterating through
	};


	/**
	 * Implementation of ShapeTriangleIterator that can iterate through the triangles of a MeshShape of type EDrawMode::TRIANGLES
	 */
	class NAPAPI ShapeTriangleListIterator final : public ShapeTriangleIterator
	{
	public:
		ShapeTriangleListIterator(const MeshShape& shape);

		/**
		 * Retrieves the indices of the next triangle in the MeshShape. Should only be called while isDone() returns false.
		 * @return the 3 indices of the next triangle in a glm::ivec3
		 */
		virtual const ShapeTriangle next() override;
	};


	/**
	 * Implementation of ShapeTriangleIterator that can iterate through the triangles of a MeshShape of type EDrawMode::TRIANGLE_FAN
	 */
	class NAPAPI ShapeTriangleFanIterator final : public ShapeTriangleIterator
	{
	public:
		ShapeTriangleFanIterator(const MeshShape& shape);

		/**
		 * Retrieves the indices of the next triangle in the MeshShape. Should only be called while isDone() returns false.
		 * @return the 3 indices of the next triangle in a glm::ivec3
		 */
		virtual const ShapeTriangle next() override;

	private:
		uint32	mFanStartIndex;			///< First index of the fan (all triangles share this index)
	};


	/**
	 * Implementation of ShapeTriangleIterator that can iterate through the triangles of a MeshShape of type EDrawMode::TRIANGLE_STRIP
	 */
	class NAPAPI ShapeTriangleStripIterator final : public ShapeTriangleIterator
	{
	public:
		ShapeTriangleStripIterator(const MeshShape& shape);

		/**
		 * Retrieves the indices of the next triangle in the MeshShape. Should only be called while isDone() returns false.
		 * @return the 3 indices of the next triangle in a glm::ivec3
		 */
		virtual const ShapeTriangle next() override;
	};

	/**
	 * While the various ShapeTriangleIterators can be used to iterate through the triangles of a specific MeshShape, quite often you want to
	 * iterate through all the triangles in all MeshShapes in a particular MeshInstance. TriangleIterator provides this functionality.
	 *
	 * This iterator will internally use the right type of ShapeTriangleIterator for a specific MeshShape and retrieve triangles through that.
	 * Note that only triangles are returned; MeshShapes of a different type will be skipped.
	 */
	class NAPAPI TriangleIterator final
	{
	public:
		TriangleIterator(const MeshInstance& meshInstance);
		~TriangleIterator();

		/**
		 * Checks whether this iterator is done iterating. next() should only be called while this function returns false.
		 * @return true if done iterating, false if not
		 */
		bool isDone() const							{ return mCurIterator == nullptr; }

		/**
		 * Retrieves the indices of the next triangle. Should only be called while isDone() returns false.
		 * @return the 3 indices of the next triangle in a glm::ivec3
		 */
		const Triangle next();

	private:
		/**
		 * Advance the iterator to the next shape (if any)
		 */
		void advanceToNextShape();

	private:
		const MeshInstance*		mMeshInstance;			///< The MeshInstance that we're iterating through
		ShapeTriangleIterator*	mCurIterator;			///< The current triangle iterator for the current shape. Note that we're not using unique_ptr to avoid overhead when dereffing pointer during iteration
		int						mCurShapeIndex;			///< Current index in the MeshInstances' shape list 
	};
}

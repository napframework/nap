#pragma once

#include <utility/dllexport.h>
#include <glm/glm.hpp>

namespace nap
{
	class MeshInstance;
	class MeshShape;

	/**
	 * A TriangleShapeIterator can be used to iterate through all triangles in a MeshShape.
	 * The TriangleShapeIterator itself only serves as a base class; derived classes are provided that deal with differing index layouts depending on the type of the mesh.
	 */
	class NAPAPI TriangleShapeIterator
	{
	public:
		TriangleShapeIterator(const MeshShape& shape, int startIndex);
		virtual ~TriangleShapeIterator()	{}

		/**
		 * Checks whether this iterator is done iterating. next() should only be called while this function returns false.
		 * @return true if done iterating, false if not
		 */
		bool isDone() const	{ return mCurrentIndex >= mIndexEnd; }

		/**
		 * Retrieves the indices of the next triangle in the MeshShape. Should only be called while isDone() returns false.
		 * @return the 3 indices of the next triangle in a glm::ivec3
		 */
		virtual const glm::ivec3 next() = 0;

	protected:
		const unsigned int*	mCurrentIndex;	///< The current position in the index buffer of the shape that we're iterating through
	
	private:
		const unsigned int*	mIndexEnd;		///< End of the index buffer of the shape that we're iterating through
	};


	/**
	 * Implementation of TriangleShapeIterator that can iterate through the triangles of a MeshShape of type opengl::EDrawMode::TRIANGLES
	 */
	class NAPAPI TriangleShapeListIterator final : public TriangleShapeIterator
	{
	public:
		TriangleShapeListIterator(const MeshShape& shape);

		/**
		 * Retrieves the indices of the next triangle in the MeshShape. Should only be called while isDone() returns false.
		 * @return the 3 indices of the next triangle in a glm::ivec3
		 */
		virtual const glm::ivec3 next() override;
	};


	/**
	 * Implementation of TriangleShapeIterator that can iterate through the triangles of a MeshShape of type opengl::EDrawMode::TRIANGLE_FAN
	 */
	class NAPAPI TriangleShapeFanIterator final : public TriangleShapeIterator
	{
	public:
		TriangleShapeFanIterator(const MeshShape& shape);

		/**
		 * Retrieves the indices of the next triangle in the MeshShape. Should only be called while isDone() returns false.
		 * @return the 3 indices of the next triangle in a glm::ivec3
		 */
		virtual const glm::ivec3 next() override;

	private:
		unsigned int mFanStartIndex;	///< First index of the fan (all triangles share this index)
	};


	/**
	 * Implementation of TriangleShapeIterator that can iterate through the triangles of a MeshShape of type opengl::EDrawMode::TRIANGLE_STRIP
	 */
	class NAPAPI TriangleShapeStripIterator final : public TriangleShapeIterator
	{
	public:
		TriangleShapeStripIterator(const MeshShape& shape);

		/**
		 * Retrieves the indices of the next triangle in the MeshShape. Should only be called while isDone() returns false.
		 * @return the 3 indices of the next triangle in a glm::ivec3
		 */
		virtual const glm::ivec3 next() override;
	};


	/**
	 * While the various TriangleShapeIterators can be used to iterate through the triangles of a specific MeshShape, quite often you want to
	 * iterate through all the triangles in all MeshShapes in a particular MeshInstance. TriangleIterator provides this functionality.
	 *
	 * This iterator will internally use the right type of TriangleShapeIterator for a specific MeshShape and retrieve triangles through that.
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
		const glm::ivec3 next();

		/**
		 * @return The index of the current shape (in the MeshInstances' shape list) that we're iterating through.
		 */
		int getCurrentShapeIndex() const			{ return mCurShapeIndex; }

		/**
		 * @return The index of the current triangle in the current shape that we're iterating through
		 */
		int getCurrentTriangleIndex() const			{ return mCurrentTriangleIndex; }

	private:
		/**
		 * Advance the iterator to the next shape (if any)
		 */
		void advanceToNextShape();

	private:
		const MeshInstance*		mMeshInstance;			///< The MeshInstance that we're iterating through
		TriangleShapeIterator*	mCurIterator;			///< The current triangle iterator for the current shape. Note that we're not using unique_ptr to avoid overhead when dereffing pointer during iteration
		int						mCurShapeIndex;			///< Current index in the MeshInstances' shape list 
		int						mCurrentTriangleIndex;	///< Index of current triangle
	};
}

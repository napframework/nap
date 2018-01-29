#include <triangleiterator.h>
#include "mesh.h"

namespace nap
{
	TriangleShapeIterator::TriangleShapeIterator(const MeshShape& shape, int startIndex) :
		mCurrentIndex(shape.getIndices().data()),
		mIndexEnd(shape.getIndices().data() + shape.getIndices().size()) 
	{
	}

	//////////////////////////////////////////////////////////////////////////

	TriangleShapeListIterator::TriangleShapeListIterator(const MeshShape& shape) :
		TriangleShapeIterator(shape, 0)
	{
		assert(shape.getDrawMode() == opengl::EDrawMode::TRIANGLES);
		assert(shape.getNumIndices() != 0 && shape.getNumIndices() % 3 == 0);
	}


	const glm::ivec3 TriangleShapeListIterator::next() 
	{
		// Note: we deref current index without advancing current index. This results in the most efficient asm code:
		// the offset can be used in the mov instruction directly,  we don't need to increment mCurrentIndex for each element.
		glm::ivec3 result(*mCurrentIndex, *(mCurrentIndex + 1), *(mCurrentIndex + 2));
		mCurrentIndex += 3;

		return result;
	}

	//////////////////////////////////////////////////////////////////////////

	TriangleShapeFanIterator::TriangleShapeFanIterator(const MeshShape& shape) :
		TriangleShapeIterator(shape, 2)
	{
		assert(shape.getDrawMode() == opengl::EDrawMode::TRIANGLE_FAN);
		assert(shape.getNumIndices() >= 3);
		
		// Triangle fans always share the first vertex, so we can just cache it here
		mFanStartIndex = shape.getIndices().front();
	}


	const glm::ivec3 TriangleShapeFanIterator::next()
	{
		// Note: we deref current index without modifying current index. This results in the most efficient asm code:
		// the offset can be used in the mov instruction directly, we don't need to increment mCurrentIndex for each element.
		glm::ivec3 result(mFanStartIndex, *(mCurrentIndex - 1), *mCurrentIndex);
		++mCurrentIndex;

		return result;
	}

	//////////////////////////////////////////////////////////////////////////

	TriangleShapeStripIterator::TriangleShapeStripIterator(const MeshShape& shape) :
		TriangleShapeIterator(shape, 2)
	{
		assert(shape.getDrawMode() == opengl::EDrawMode::TRIANGLE_STRIP);
		assert(shape.getNumIndices() >= 3);
	}


	const glm::ivec3 TriangleShapeStripIterator::next()
	{
		// Note: we deref current index without modifying current index. This results in the most efficient asm code:
		// the offset can be used in the mov instruction directly, we don't need to increment mCurrentIndex for each element.
		glm::ivec3 result(*(mCurrentIndex-2), *(mCurrentIndex - 1), *mCurrentIndex);
		++mCurrentIndex;

		return result;
	}

	//////////////////////////////////////////////////////////////////////////

	TriangleIterator::TriangleIterator(const MeshInstance& meshInstance) :
		mMeshInstance(&meshInstance),
		mCurIterator(nullptr),
		mCurShapeIndex(0),
		mCurrentTriangleIndex(0)
	{
		// When constructing, advance to next shape immediately. This deals with empty mesh cases.
		advanceToNextShape();
	}


	TriangleIterator::~TriangleIterator()
	{
		delete mCurIterator;
	}


	const glm::ivec3 TriangleIterator::next()
	{
		// Retrieve next value from the current iterator. This cannot fail, because next() should only be called while isDone() returns false
		glm::ivec3 result = mCurIterator->next();
		++mCurrentTriangleIndex;

		// If retrieving the current value finished the current iterator, advance to the next one
		if (mCurIterator->isDone())
			advanceToNextShape();

		return result;
	}


	void TriangleIterator::advanceToNextShape()
	{
		// Reset state when advancing to next shape
		// Note that we use new/delete explicitly, to avoid overhead of unique_ptr in debug builds
		delete mCurIterator;
		mCurIterator = nullptr;
		mCurrentTriangleIndex = 0;

		// Try to advance to the next shape of triangle type. If no triangle shapes are found, mCurIterator will remain null and we're done with iteration
		for (; mCurShapeIndex < mMeshInstance->getNumShapes() && mCurIterator == nullptr; ++mCurShapeIndex)
		{
			const MeshShape& shape = mMeshInstance->getShape(mCurShapeIndex);

			switch (shape.getDrawMode())
			{
			case opengl::EDrawMode::TRIANGLES:
				mCurIterator = new TriangleShapeListIterator(shape);
				break;
			case opengl::EDrawMode::TRIANGLE_STRIP:
				mCurIterator = new TriangleShapeStripIterator(shape);
				break;
			case opengl::EDrawMode::TRIANGLE_FAN:
				mCurIterator = new TriangleShapeFanIterator(shape);
				break;
			default:
				break;
			}
		}
	}
}
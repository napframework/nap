/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <triangleiterator.h>
#include "mesh.h"

namespace nap
{
	ShapeTriangle::ShapeTriangle(int triangleIndex, int index0, int index1, int index2) :
		mTriangleIndex(triangleIndex)
	{
		mIndices[0] = index0;
		mIndices[1] = index1;
		mIndices[2] = index2;
	}

	//////////////////////////////////////////////////////////////////////////

	ShapeTriangleIterator::ShapeTriangleIterator(const MeshShape& shape, int startIndex) :
		mCurrentIndex(shape.getIndices().data()),
		mIndexEnd(shape.getIndices().data() + shape.getIndices().size()) 
	{
	}

	//////////////////////////////////////////////////////////////////////////

	ShapeTriangleListIterator::ShapeTriangleListIterator(const MeshShape& shape) :
		ShapeTriangleIterator(shape, 0)
	{
		assert(shape.getNumIndices() != 0 && shape.getNumIndices() % 3 == 0);
	}


	const ShapeTriangle ShapeTriangleListIterator::next() 
	{
		// Note: we deref current index without advancing current index. This results in the most efficient asm code:
		// the offset can be used in the mov instruction directly,  we don't need to increment mCurrentIndex for each element.
		ShapeTriangle result(mCurrentTriangle++, *mCurrentIndex, *(mCurrentIndex + 1), *(mCurrentIndex + 2));
		mCurrentIndex += 3;

		return result;
	}

	//////////////////////////////////////////////////////////////////////////

	ShapeTriangleFanIterator::ShapeTriangleFanIterator(const MeshShape& shape) :
		ShapeTriangleIterator(shape, 2)
	{
		assert(shape.getNumIndices() >= 3);
		
		// Triangle fans always share the first vertex, so we can just cache it here
		mFanStartIndex = shape.getIndices().front();
	}


	const ShapeTriangle ShapeTriangleFanIterator::next()
	{
		// Note: we deref current index without modifying current index. This results in the most efficient asm code:
		// the offset can be used in the mov instruction directly, we don't need to increment mCurrentIndex for each element.
		ShapeTriangle result(mCurrentTriangle++, mFanStartIndex, *(mCurrentIndex - 1), *mCurrentIndex);
		++mCurrentIndex;

		return result;
	}

	//////////////////////////////////////////////////////////////////////////

	ShapeTriangleStripIterator::ShapeTriangleStripIterator(const MeshShape& shape) :
		ShapeTriangleIterator(shape, 2)
	{
		assert(shape.getNumIndices() >= 3);
	}


	const ShapeTriangle ShapeTriangleStripIterator::next()
	{
		// Note: we deref current index without modifying current index. This results in the most efficient asm code:
		// the offset can be used in the mov instruction directly, we don't need to increment mCurrentIndex for each element.
		ShapeTriangle result(mCurrentTriangle++, *(mCurrentIndex-2), *(mCurrentIndex - 1), *mCurrentIndex);
		++mCurrentIndex;

		return result;
	}

	//////////////////////////////////////////////////////////////////////////

	TriangleIterator::TriangleIterator(const MeshInstance& meshInstance) :
		mMeshInstance(&meshInstance),
		mCurIterator(nullptr),
		mCurShapeIndex(0)
	{
		// When constructing, advance to next shape immediately. This deals with empty mesh cases.
		advanceToNextShape();
	}


	TriangleIterator::~TriangleIterator()
	{
		delete mCurIterator;
	}


	const Triangle TriangleIterator::next()
	{
		// Retrieve next value from the current iterator. This cannot fail, because next() should only be called while isDone() returns false
		Triangle result(mCurShapeIndex, mCurIterator->next());

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

		// Try to advance to the next shape of triangle type. If no triangle shapes are found, mCurIterator will remain null and we're done with iteration
		for (; mCurShapeIndex < mMeshInstance->getNumShapes() && mCurIterator == nullptr; ++mCurShapeIndex)
		{
			const MeshShape& shape = mMeshInstance->getShape(mCurShapeIndex);

			switch (mMeshInstance->getDrawMode())
			{
			case EDrawMode::Triangles:
				mCurIterator = new ShapeTriangleListIterator(shape);
				break;
			case EDrawMode::TriangleStrip:
				mCurIterator = new ShapeTriangleStripIterator(shape);
				break;
			case EDrawMode::TriangleFan:
				mCurIterator = new ShapeTriangleFanIterator(shape);
				break;
			default:
				break;
			}
		}
	}


	Triangle::Triangle(int shapeIndex, const ShapeTriangle& shapeTriangle) :
		ShapeTriangle(shapeTriangle),
		mShapeIndex(shapeIndex)
	{

	}

}
// Local Includes
#include <Spline/nofSpline.h>

// std Includes
#include <assert.h>
#include <limits.h>
#include <nofUtils.h>

/**
@brief Constructor
**/
NSpline::NSpline(const ofPolyline& inLine)
{
	SetPolyLine(inLine);
}



/**
@brief Sets the polyline, initializes all the buffers
**/
void NSpline::SetPolyLine(const ofPolyline& inLine)
{
	mPolyLine = inLine;
	InitBuffers();
}


/**
@brief Clears all the buffers
**/
void NSpline::ClearBuffers()
{
	// Clear vertex buffer object
	mVertexBuffer.clearColors();
	mVertexBuffer.clearNormals();
	mVertexBuffer.clearVertices();

	// Clear all locally stored data
	mNormals.clear();
	mColors.clear();

	// Reset bounds
	mMinLocalBounds = ofVec3f(0.0f, 0.0f, 0.0f);
	mMaxLocalBounds = ofVec3f(0.0f, 0.0f, 0.0f);
	mCentroid = ofVec3f(0.0f, 0.0f, 0.0f);
}



/**
@brief Initializes all the buffers
**/
void NSpline::InitBuffers()
{
	// Clear Existing buffers
	ClearBuffers();

	// Ensure there's actual poly line data
	if (GetPointCount() < 2)
	{
		assert(false);
		return;
	}

	// Allocate color and normal buffers
	mColors.resize(mPolyLine.size());
	mNormals.resize(mPolyLine.size());

	// Calculate normals
	CalculateNormals();

	// Calculate local bounds
	CalculateBounds();

	// Store original verts
	mOriginalVerts = mPolyLine.getVertices();

	// Setup VBO
	mVertexBuffer.setVertexData(&mPolyLine.getVertices()[0], GetPointCount(), GL_STREAM_DRAW);
	mVertexBuffer.setColorData(&mColors[0], GetPointCount(), GL_STREAM_DRAW);
	mVertexBuffer.setNormalData(&mNormals[0], GetPointCount(), GL_STATIC_DRAW);
}



/**
@brief Calculates the curve normals
**/
void NSpline::CalculateNormals()
{
	for (unsigned int i = 0; i < GetPointCount(); i++)
		mNormals[i] = mPolyLine.getNormalAtIndex(i);

	if (mNormals.size() < 2)
		return;

	// Give first and last normals the neighbour values
	mNormals[0] = mNormals[1];
	mNormals[GetPointCount() - 1] = mNormals[GetPointCount() - 2];
}



/**
@brief Updates the VBO on the GPU -> pushes set of vertex changes
**/
void NSpline::UpdateVBO(DataType inType)
{
	switch (inType)
	{
	case NSpline::DataType::COLOR:
		mVertexBuffer.updateColorData(&mColors[0], GetPointCount());
		break;
	case DataType::NORMAL:
		mVertexBuffer.updateNormalData(&mNormals[0], GetPointCount());
		break;
	case DataType::VERTEX:
		mVertexBuffer.updateVertexData(&mPolyLine.getVertices()[0], GetPointCount());
		break;
	case DataType::ALL:
		mVertexBuffer.updateColorData(&mColors[0], GetPointCount());
		mVertexBuffer.updateVertexData(&mPolyLine.getVertices()[0], GetPointCount());
		mVertexBuffer.updateNormalData(&mNormals[0], GetPointCount());
		break;
	}
}



/**
@brief Calculates the bounds based on the vertices in the current polyline
**/
void NSpline::CalculateBounds()
{
	// Get min max float bounds
	float max_f = std::numeric_limits<float>::max();
	float min_f = std::numeric_limits<float>::min();

	// Clear min / max bounds
	mMinLocalBounds = ofVec3f(max_f, max_f, max_f);
	mMaxLocalBounds = ofVec3f(min_f, min_f, min_f);

	// Calculate local vertex bounds
	for (const auto& v : mPolyLine.getVertices())
	{
		mMinLocalBounds.x = v.x < mMinLocalBounds.x ? v.x : mMinLocalBounds.x;
		mMinLocalBounds.y = v.y < mMinLocalBounds.y ? v.y : mMinLocalBounds.y;
		mMinLocalBounds.z = v.z < mMinLocalBounds.z ? v.z : mMinLocalBounds.z;

		mMaxLocalBounds.x = v.x > mMaxLocalBounds.x ? v.x : mMaxLocalBounds.x;
		mMaxLocalBounds.y = v.y > mMaxLocalBounds.y ? v.y : mMaxLocalBounds.y;
		mMaxLocalBounds.z = v.z > mMaxLocalBounds.x ? v.z : mMaxLocalBounds.z;
	}

	// Calculate centroid based on bounds
	mCentroid = ((mMaxLocalBounds - mMinLocalBounds) / 2.0f) + mMinLocalBounds;
}



/**
@brief Draw
**/
void NSpline::Draw()
{
	if (mPolyLine.isClosed())
	{
		mVertexBuffer.draw(GL_LINE_LOOP, 0, GetPointCount());
		return;
	}
	mVertexBuffer.draw(GL_LINE_STRIP, 0, GetPointCount());
}
#pragma once

// OF Includes
#include <ofPolyline.h>
#include <ofVbo.h>

// STD Includes
#include <vector>

//////////////////////////////////////////////////////////////////////////

using SplineColorData  = std::vector<ofFloatColor>;
using SplineNormalData = std::vector<ofVec3f>;
using SplineVertexData = std::vector<ofVec3f>;

/**
@brief NSpline acts as a wrapper around a PolyLine object

Next to the Polyline it includes functions for adding color and normals
The ofPolyLine can be drawn using a Vertex Buffer Object
**/

class NSpline
{
public:
	enum class DataType
	{
		VERTEX,
		COLOR,
		NORMAL,
		ALL
	};

	/**
	@brief Constructor
	**/
	NSpline(const ofPolyline& inLine);
	NSpline()																		{ }

	//@name Initialization
	void							SetPolyLine(const ofPolyline& inLine);
	
	//@name Spline data 
	SplineVertexData&				GetVertexDataRef()									{ return mPolyLine.getVertices();  }
	SplineColorData&				GetColorDataRef()									{ return mColors; }
	SplineNormalData&				GetNormalDataRef()									{ return mNormals; }
	ofVec3f							GetPointAtPercent(float inValue)					{ return mPolyLine.getPointAtPercent(inValue); }
	float							GetIndexAtPercent(float inValue)					{ return mPolyLine.getIndexAtPercent(inValue);  }

	//@name Setters / Getters
	inline void						SetColor(unsigned int inIndex, const ofFloatColor& inColor)	{ assert(inIndex < GetPointCount()); mColors[inIndex] = inColor; }
	inline ofFloatColor&			GetColor(unsigned int inIndex)								{ assert(inIndex < GetPointCount()); return mColors[inIndex]; }
	inline ofVec3f&					GetVertex(unsigned int inIndex)								{ assert(inIndex < GetPointCount()); return mPolyLine.getVertices()[inIndex]; }
	inline ofVec3f&					GetNormal(unsigned int inIndex)								{ assert(inIndex < GetPointCount()); return mNormals[inIndex]; }
	inline ofVec3f					GetSourceVertex(unsigned int inIndex) const					{ assert(inIndex < GetPointCount()); return mOriginalVerts[inIndex]; }
	bool							IsClosed() const											{ return mPolyLine.isClosed(); }

	//@name Vertex buffer
	const ofVbo&					GetVertexBuffer() const								{ return mVertexBuffer;  }

	//@name Utility
	inline unsigned int				GetPointCount() const								{ return mPolyLine.size(); }
	const ofVec3f&					GetCentroid() const									{ return mCentroid; }
	void							GetBounds(ofVec3f& outMin, ofVec3f& outMax) const	{ outMin = mMinLocalBounds; outMax = mMaxLocalBounds; }

	//@name Update
	void							UpdateVBO(DataType inType);

	//@name Draw
	void							Draw();

private:
	//@name Spline Data
	ofPolyline						mPolyLine;
	SplineColorData					mColors;
	SplineNormalData				mNormals;

	//@name Vertex Buffer Object
	ofVbo							mVertexBuffer;

	//@name Bound Information
	ofVec3f							mMinLocalBounds;
	ofVec3f							mMaxLocalBounds;
	ofVec3f							mCentroid;

	//@name Original Verts
	SplineVertexData				mOriginalVerts;

	//@name Utility
	void							ClearBuffers();
	void							InitBuffers();
	void							CalculateNormals();
	void							CalculateBounds();
};

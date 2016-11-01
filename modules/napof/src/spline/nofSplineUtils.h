#pragma once

#include <spline/nofSpline.h>

// All the currently supported splines
enum class SplineType : int
{
	Circle,
	Triangle,
	Square,
	Line,
	Hexagon,
	Max = Hexagon,
	Invalid = -1
};

// Returns the spline type name for the given type
const std::string&	gGetSplineTypeName(SplineType inType);

// Returns the spline type based on it's name
const SplineType	gGetSplineType(const std::string& inType);

// Up sample a polyline equally over all edges (every edge has the same number of points)
void				gUpsamplePolyLine(const ofPolyline& inPolyline, int inCount, ofPolyline& outPolyline);

// Creates a circle with radius @inRadius
void				gCreateCircle(float inDiameter, int inPointCount, const ofPoint& inCenter, NSpline& outSpline);

// Creates a triangle with equal sides
void				gCreateEqualTriangle(float inSize, int inPointCount, const ofPoint& inCenter, NSpline& outSpline);

// Creates a rectangle width the given width and height
void				gCreateRectangle(float inWidth, float inHeight, int inPointCount, const ofPoint& inCenter, NSpline& outSpline);

// Creates a square with the given width
void				gCreateSquare(float inSize, int inPointCount, const ofPoint& inCenter, NSpline& outSpline);

// Creates a simple line
void				gCreateLine(float inWidth, int inPointCount, const ofPoint& inCenter, NSpline& outSpline);

// Creates a hexagon
void				gCreateHexagon(float inDiameter, int inPointCount, const ofPoint& inCenter, NSpline& outSpline);

// Creates a spline from a file
void				gCreateSplineFromFile(const std::string& inFile, int inPointCount, NSpline& outSpline, const ofPoint& inCenter = {0,0,0}, float scale = 1.0f);
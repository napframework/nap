#include <Spline/nofSplineUtils.h>
#include <ofPath.h>
#include <nofUtils.h>
#include <ofxSvg.h>
#include <nap/logger.h>

// Spline type names
static const std::vector<std::string> gSplineTypeNames = { "Circle", "Triangle", "Square", "Line", "Hexagon" };

/**
@brief Returns the correct name for the given type
**/
const std::string& gGetSplineTypeName(SplineType inType)
{
	// Get index
	int idx = static_cast<int>(inType);
	assert(idx < int(gSplineTypeNames.size()));
	return gSplineTypeNames[idx];
}


/**
@brief Returns the spline type based on the given name
**/
const SplineType gGetSplineType(const std::string& inType)
{
	int idx = 0;
	for (const auto& name : gSplineTypeNames)
	{
		if (name == inType)
			return static_cast<SplineType>(idx);
		idx++;
	}
	return SplineType::Invalid;
}


/**
@brief Creates a circular NSpline
**/
void gCreateCircle(float inDiameter, int inPointCount, const ofPoint& inCenter, NSpline& outSpline)
{
	ofPolyline line;
	
	// Create line from arc
	line.arc(inCenter, inDiameter / 2.0f, inDiameter / 2.0f, 0, 360, inPointCount);
	
	// Close it
	line.setClosed(true);

	// Set poly line
	outSpline.SetPolyLine(line);
}


/**
@brief Creates a triangle
**/
void gCreateEqualTriangle(float inSize, int inPointCount, const ofPoint& inCenter, NSpline& outSpline)
{
	// Calculate offset
	float offset = inSize / 2.0f;
	
	// Get first 2 points
	ofVec3f point_one(inCenter.x + offset, inCenter.y, inCenter.z);
	ofVec3f point_two(inCenter.y - offset, inCenter.y, inCenter.z);

	// Subtract and normalize
	ofVec3f point_diff = point_one - point_two;
	ofVec3f point_diff_n = point_diff.getNormalized();
	float edge_length = point_diff.length();

	// Rotate normalized vector
	point_diff_n.rotate(60, ofVec3f(0.0f, 0.0f, 1.0f));
	
	// Calculate 3rd point
	ofVec3f point_thr = point_two + (point_diff_n*edge_length);

	// center triangle
	float y_offset = point_thr.y / 2;
	point_one.y -= y_offset;
	point_two.y -= y_offset;
	point_thr.y -= y_offset;

	// Add vertices for every side (equally distributing point count)
	ofPolyline or_line, re_line;
	or_line.addVertex(point_one);
	or_line.addVertex(point_two);
	or_line.addVertex(point_thr);
	or_line.close();

	gUpsamplePolyLine(or_line, inPointCount, re_line);

	// Set line
	outSpline.SetPolyLine(re_line);
}



/**
@brief Upsamples the polyline equally over every edge (not smart)
**/
void gUpsamplePolyLine(const ofPolyline& inPolyline, int inCount, ofPolyline& outPolyLine)
{
	// If there are an equal amount or less vertices, don't do anything
	if (inCount <= int(inPolyline.size()))
	{
		outPolyLine = inPolyline;
		return;
	}

	// Make sure vertices are present
	if (inPolyline.size() < 2)
	{
		outPolyLine = inPolyline;
		return;
	}

	// Figure out the amount of edges, closed lines have one extra edge (connecting first to last)
	int edge_count = inPolyline.isClosed() ? inPolyline.size() : inPolyline.size() - 1;

	// Calculate the total amount of pointer for every side
	int pps = inCount / edge_count;

	// Clear existing poly line
	outPolyLine.clear();

	// Create buffer
	std::vector<ofVec3f> buffer;
	buffer.reserve(inCount);

	for (int i = 0; i < edge_count; i++)
	{
		// Get edge points
		ofPoint point_one = inPolyline.getVertices()[i];
		ofPoint point_two = i + 1 >= int(inPolyline.size()) ? inPolyline.getVertices()[0] : inPolyline.getVertices()[i + 1];

		// calculate point to point normal
		ofVec3f normal = (point_two - point_one).getNormalized();

		// step increment
		float inc = (point_two - point_one).length() / float(pps);

		// Add edge vertices
		for (int p = 0; p < pps; p++)
		{
			ofVec3f new_point = point_one + (normal * float(p) * inc);
			buffer.emplace_back(new_point);
		}
	}

	// Set vertices in one go
	outPolyLine.addVertices(buffer);

	// Close line if original was closed
	if (inPolyline.isClosed())
	{
		outPolyLine.setClosed(true);
		return;
	}
	
	// Compensate for the last vertex when resampling open curves
	outPolyLine.addVertex(inPolyline.getVertices()[inPolyline.size() - 1]);
	outPolyLine.setClosed(false);
}



/**
@brief Creates a rectangle
**/
void gCreateRectangle(float inWidth, float inHeight, int inPointCount, const ofPoint& inCenter, NSpline& outSpline)
{
	// Create rectangle
	ofPoint center(inCenter.x-(inWidth/2.0f), inCenter.y-(inHeight/2.0f), inCenter.z);
	ofRectangle rect(center, inWidth, inHeight);

	// Polyline from rectangle
	ofPolyline or_line = ofPolyline::fromRectangle(rect);
	ofPolyline re_line;

	// Convert line
	gUpsamplePolyLine(or_line, inPointCount, re_line);

	outSpline.SetPolyLine(re_line);
}



/**
@brief Creates a square
**/
void gCreateSquare(float inSize, int inPointCount, const ofPoint& inCenter, NSpline& outSpline)
{
	gCreateRectangle(inSize, inSize, inPointCount, inCenter, outSpline);
}



/**
@brief Creates a line
**/
void gCreateLine(float inWidth, int inPointCount, const ofPoint& inCenter, NSpline& outSpline)
{
	float offset = inWidth / 2;
	ofPolyline or_line, re_line;
	or_line.addVertex(inCenter.x - offset, inCenter.y, inCenter.z);
	or_line.addVertex(inCenter.x + offset, inCenter.y, inCenter.z);
	or_line.setClosed(false);

	// Upsample line
	gUpsamplePolyLine(or_line, inPointCount, re_line);

	// Set
	outSpline.SetPolyLine(re_line);
}



/**
@brief Creates a hexagon
**/
void gCreateHexagon(float inDiameter, int inPointCount, const ofPoint& inCenter, NSpline& outSpline)
{
	ofPolyline line;

	// Create line from arc
	line.arc(inCenter, inDiameter/2.0f, inDiameter/ 2.0f, 0, 360, 6);

	// Rotate 90*
	std::vector<ofPoint>& verts = line.getVertices();
	for (auto& vert : verts)
	{
		vert = vert.rotate(90, ofVec3f(0.0f, 0.0f, 1.0f));
	}

	ofPolyline out_line;
	gUpsamplePolyLine(line, inPointCount, out_line);

	// Close it
	out_line.setClosed(true);

	// Set poly line
	outSpline.SetPolyLine(out_line);
}



// Creates a spline from an svg file
void gCreateSplineFromFile(const std::string& inFile, int inPointCount, NSpline& outSpline, const ofPoint& inCenter)
{
	// Load the file
	ofxSVG svg_file;
	svg_file.load(inFile);

	// Ensure we have at least one path
	if (svg_file.getNumPath() == 0)
	{
		nap::Logger::warn("no paths found in file: %s", inFile.c_str());
		return;
	}

	// Pick first one
	ofPath& sampled_path = svg_file.getPathAt(0);
	if (svg_file.getNumPath() > 1)
	{
		nap::Logger::warn("multiple paths found in file: %s, picking first one", inFile.c_str());
	}

	// Make sure it has an outline
	if (!sampled_path.hasOutline())
	{
		nap::Logger::warn("file has no outline: %s", inFile.c_str());
		return;
	}

	// Get the points
	const std::vector<ofPolyline>& path_lines = sampled_path.getOutline();	
	if (path_lines.empty())
	{
		nap::Logger::warn("unable to extract outline from file: %s", inFile.c_str());
		return;
	}

	// Notify based on multiple poly lines (often only 1)
	if (path_lines.size() > 1)
	{
		nap::Logger::warn("multiple polylines extracted from path in file: %s, picking first one", inFile.c_str());
	}

	// Get the resampled line (based on point count)
	ofPolyline resampled_line = path_lines[0].getResampledByCount(inPointCount);

	// Center
	ofPoint bbox_point = resampled_line.getBoundingBox().getCenter();
	ofPoint center_point = bbox_point - inCenter;
	for (auto& v : resampled_line.getVertices())
	{
		v -= center_point;
	}

	// Set it
	outSpline.SetPolyLine(resampled_line);
}

#pragma once

// Nap includes
#include <nap.h>

// of Includes
#include <ofMain.h>
#include <ofEasyCam.h>

// Naivi of includes
#include <Spline/nofSpline.h>
#include <Spline/nofSplineUtils.h>

// Defines
using OFVectorMap = std::unordered_map<std::string, ofVec3f>;



// forward declares
namespace nap
{
	enum class LfoType;

	bool convert_string_to_ofVec3f(const std::string& inValue, ofVec3f& outValue);
	bool convert_ofVec3f_to_string(const ofVec3f& inValue, std::string& outValue);
	bool convert_string_to_ofVec2f(const std::string& inValue, ofVec2f& outValue);
	bool convert_ofVec2f_to_string(const ofVec2f& inValue, std::string& outValue);
	bool convert_string_to_SplineType(const std::string& inValue, SplineType& outValue);
	bool convert_SplineType_to_string(const SplineType& inValue, std::string& outValue);
	bool convert_string_to_ofFloatColor(const std::string& inValue, ofFloatColor& outValue);
	bool convert_ofFloatColor_to_string(const ofFloatColor& inValue, std::string& outValue);
	bool convert_string_to_ofEasyCam(const std::string& inValue, ofEasyCam& outValue);
	bool convert_ofEasyCam_to_string(const ofEasyCam& inValue, std::string& outValue);
	bool convert_OFVectorMap_to_string(const OFVectorMap& inValue, std::string& outValue);
	bool convert_string_to_OFVectorMap(const std::string& inValue, OFVectorMap& outValue);

}

/**
@brief All the of nap enabled attributes
**/

// Vec3 attribute
RTTI_DECLARE_DATA(ofVec3f);

// Vec 2 attribute
RTTI_DECLARE_DATA(ofVec2f);

// Spline Attribute
RTTI_DECLARE_DATA(NSpline);

// Spline Type Attribute
RTTI_DECLARE_DATA(SplineType)

// Color Type Attribute
RTTI_DECLARE_DATA(ofFloatColor)

// Color Array Attribute
RTTI_DECLARE_DATA(std::vector<ofFloatColor>)

// Easy Camera Attribute
RTTI_DECLARE_DATA(ofEasyCam)

// LFO Type
RTTI_DECLARE_DATA(nap::LfoType)

// Texture
RTTI_DECLARE_DATA(ofTexture)

// Vector map
RTTI_DECLARE_DATA(OFVectorMap)




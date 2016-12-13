#pragma once

#include <nap.h>

/**
@brief 2d integer vector that can be used as an attribute
**/
class ofVec2i
{
	RTTI_ENABLE()
public:
	ofVec2i(int inx, int iny) : x(inx), y(iny) { }
	ofVec2i() { }
	int x = 0;
	int y = 0;

	bool operator==(const ofVec2i& v) const { return x == v.x && y == v.y; }
	bool operator!=(const ofVec2i& v) const { return !(x == v.x && y == v.y); }
};


/**
@brief type-converters for serialization
**/
namespace nap
{
	bool convert_string_to_ofVec2i(const std::string& inValue, ofVec2i& outValue);
	bool convert_ofVec2i_to_string(const ofVec2i& inValue, std::string& outValue);
}

RTTI_DECLARE_DATA(ofVec2i)
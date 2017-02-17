// NAP includes
#include <nap/stringutils.h>

#include "napofattributes.h"
#include "napofsplinemodulationcomponent.h"

// Core includes
#include <nap/attribute.h>
#include <nap/logger.h>

// Std includes
#include <vector>
#include <assert.h>

namespace nap
{
	// ofVec3f

	bool convert_string_to_ofVec3f(const std::string& inValue, ofVec3f& outValue)
	{
		std::vector<std::string> out_values;
		gSplitString(inValue, ',', out_values);
		if (out_values.size() != 3)
			return false;
		outValue.x = atof(out_values[0].c_str());
		outValue.y = atof(out_values[1].c_str());
		outValue.z = atof(out_values[2].c_str());
		return true;
	}

	bool convert_ofVec3f_to_string(const ofVec3f& inValue, std::string& outValue)
	{
		std::ostringstream ss;
		ss << inValue.x;
		ss << ",";
		ss << inValue.y;
		ss << ",";
		ss << inValue.z;
		outValue = ss.str();
		return true;
	}

	// ofVec2f

	bool convert_string_to_ofVec2f(const std::string& inValue, ofVec2f& outValue)
	{
		std::vector<std::string> out_values;
		gSplitString(inValue, ',', out_values);
		if (out_values.size() != 2)
			return false;
		outValue.x = atof(out_values[0].c_str());
		outValue.y = atof(out_values[1].c_str());
		return true;
	}

	bool convert_ofVec2f_to_string(const ofVec2f& inValue, std::string& outValue)
	{
		std::ostringstream ss;
		ss << inValue.x;
		ss << ",";
		ss << inValue.y;
		outValue = ss.str();
		return true;
	}

	// SplineType

	bool convert_string_to_SplineType(const std::string& inValue, SplineType& outValue)
	{
		outValue = gGetSplineType(inValue);
		return true;
	}

	bool convert_SplineType_to_string(const SplineType& inValue, std::string& outValue)
	{
		outValue = gGetSplineTypeName(inValue);
		return true;
	}

	// ofFloatColor
	
	bool convert_string_to_ofFloatColor(const std::string& inValue, ofFloatColor& outValue)
	{
		std::vector<std::string> split;
		gSplitString(inValue, ',', split);
		if (split.size() != 3)
			return false;
		outValue.r = atof(split[0].c_str());
		outValue.g = atof(split[1].c_str());
		outValue.b = atof(split[2].c_str());
		return true;
	}

	bool convert_ofFloatColor_to_string(const ofFloatColor& inValue, std::string& outValue) 
	{
		std::ostringstream ss;
		ss << inValue.r;
		ss << ",";
		ss << inValue.g;
		ss << ",";
		ss << inValue.b;
		outValue = ss.str();
		return false;
	}


	// Vectormap	
	bool convert_OFVectorMap_to_string(const OFVectorMap& inValue, std::string& outValue)
	{
		std::ostringstream ss;
		for (auto it = inValue.begin(); it != inValue.end(); it++)
		{
			ss << (*it).first;
			ss << ':';
			
			// Get
			const ofVec3f& value = (*it).second;
			ss << value.x;
			ss << '|';
			ss << value.y;
			ss << '|';
			ss << value.z;
			
			// Add delimiter if necessary
			if (std::next(it) != inValue.end())
			{
				ss << ',';
			}
		}
		outValue = ss.str();
		return true;
	}

	bool convert_string_to_OFVectorMap(const std::string& inValue, OFVectorMap& outValue)
	{
		outValue.clear();
		std::vector<std::string> out_string;
		gSplitString(inValue, ',', out_string);
		for (auto& pair : out_string)
		{
			std::vector<std::string> keyvalue;
			gSplitString(pair, ':', keyvalue);
			if (keyvalue.size() != 2)
				return false;
			
			// Split vector
			std::vector<std::string> values;
			gSplitString(keyvalue[1], '|', values);
			if (values.size() != 3)
				return false;
			
			// Create vec
			ofVec3f new_vec;
			new_vec.x = (float)atof(values[0].c_str());
			new_vec.y = (float)atof(values[1].c_str());
			new_vec.z = (float)atof(values[2].c_str());

			// Add
			outValue.emplace(std::make_pair(keyvalue[0], new_vec));
		}
		return true;
	}

    //////////////////////////////////////////////////////////////////////////
    // Numeric Attribute Clamp Specialization
    //////////////////////////////////////////////////////////////////////////

    template <>
    ofVec3f nap::NumericAttribute<ofVec3f>::clampValue(const ofVec3f& value, const ofVec3f& min, const ofVec3f& max)
    {
        ofVec3f return_v;
        return_v.x = std::max(min.x, std::min(value.x, max.x));
        return_v.y = std::max(min.y, std::min(value.y, max.y));
        return_v.z = std::max(min.z, std::min(value.z, max.z));
        return return_v;
    }


    template <>
    ofVec2f nap::NumericAttribute<ofVec2f>::clampValue(const ofVec2f& value, const ofVec2f& min, const ofVec2f&max)
    {
        ofVec2f return_v;
        return_v.x = std::max(min.x, std::min(value.x, max.x));
        return_v.y = std::max(min.y, std::min(value.y, max.y));
        return return_v;
    }


    template <>
    ofVec4f nap::NumericAttribute<ofVec4f>::clampValue(const ofVec4f& value, const ofVec4f& min, const ofVec4f& max)
    {
        ofVec4f return_v;
        return_v.x = std::max(min.x, std::min(value.x, max.x));
        return_v.y = std::max(min.y, std::min(value.y, max.y));
        return_v.z = std::max(min.z, std::min(value.z, max.z));
        return_v.w = std::max(min.w, std::min(value.w, max.w));
        return return_v;
    }

}
//////////////////////////////////////////////////////////////////////////

RTTI_DEFINE_NUMERIC_DATA(ofVec4f)
RTTI_DEFINE_NUMERIC_DATA(ofVec3f)
RTTI_DEFINE_NUMERIC_DATA(ofVec2f)
RTTI_DEFINE_DATA(NSpline)
RTTI_DEFINE_DATA(SplineType)
RTTI_DEFINE_DATA(ofFloatColor)
RTTI_DEFINE_DATA(std::vector<ofFloatColor>)
RTTI_DEFINE_DATA(nap::LfoType)
RTTI_DEFINE(ofTexture)
RTTI_DEFINE(nap::Attribute<ofTexture*>)
RTTI_DEFINE(ofTexture*)
RTTI_DEFINE_DATA(OFVectorMap)

//////////////////////////////////////////////////////////////////////////

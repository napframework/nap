// Local includes
#include <ofVec2i.h>

// NAP includes
#include <nap/stringutils.h>

namespace nap
{
	// string to ofVec2i
	bool convert_string_to_ofVec2i(const std::string& inValue, ofVec2i& outValue)
	{
		std::vector<std::string> out_values;
		gSplitString(inValue, ',', out_values);
		if (out_values.size() != 2)
			return false;
		outValue.x = atoi(out_values[0].c_str());
		outValue.y = atoi(out_values[1].c_str());
		return true;
	}


	// ofVec2i to string
	bool convert_ofVec2i_to_string(const ofVec2i& inValue, std::string& outValue)
	{
		std::ostringstream ss;
		ss << inValue.x;
		ss << ",";
		ss << inValue.y;
		outValue = ss.str();
		return true;
	}
}

RTTI_DEFINE_DATA(ofVec2i)
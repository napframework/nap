#include "napofblendtype.h"
#include <nap/logger.h>
#include <algorithm>
#include <GL/glew.h>

RTTI_DEFINE_DATA(nap::OFBlendType);

namespace nap
{
	// All the blend mode names
	const OFBlendModeMap gBlendTypes = 
	{
		{ OFBlendType::SRC_ALPHA, { GL_SRC_ALPHA, "SRC_ALPHA" } },
		{ OFBlendType::ONE_MINUS_SRC_ALPHA, {GL_ONE_MINUS_SRC_ALPHA, "ONE_MINUS_SRC_ALPHA" } },
		{ OFBlendType::ONE, { GL_ONE ,"ONE" } },
		{ OFBlendType::ONE_MINUS_DST_COLOR, { GL_ONE_MINUS_DST_COLOR, "ONE_MINUS_DST_COLOR"} },
		{ OFBlendType::SRC_COLOR, { GL_SRC_COLOR, "SRC_COLOR" } },
		{ OFBlendType::DST_COLOR, { GL_DST_COLOR , "DST_COLOR" } }
	};


	// Converts a blend mode in to a string
	bool convert_ofblendtype_to_string(const OFBlendType& inValue, std::string& outValue)
	{
		OFBlendType mode = inValue;
		const auto& it = gBlendTypes.find(inValue);
		if (it == gBlendTypes.end())
		{
			Logger::warn("Unable to find blend mode in map");
			return false;
		}
		outValue = it->second.mBindingName;
		return true;
	}


	// Converts a string in to a blend mode
	bool convert_string_to_ofblendtype(const std::string& inValue, OFBlendType& outValue)
	{
		// Find the right blend mode
		auto it = std::find_if(gBlendTypes.begin(), gBlendTypes.end(), [&](const auto& vt) {
			return vt.second.mBindingName == inValue; });

		if (it == gBlendTypes.end())
		{
			Logger::warn("Unable to find blend mode with name: " + inValue);
			return false;
		}

		// Set out value
		outValue = it->first;
		return true;
	}
}
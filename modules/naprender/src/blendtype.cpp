// Local Includes
#include "blendtype.h"

// External Includes
#include <nap.h>
#include <algorithm>
#include <GL/glew.h>


namespace nap
{
	// All the blend mode names
	const OFBlendModeMap gBlendTypes =
	{
		{ BlendType::SRC_ALPHA,{ GL_SRC_ALPHA, "SRC_ALPHA" } },
		{ BlendType::ONE_MINUS_SRC_ALPHA,{ GL_ONE_MINUS_SRC_ALPHA, "ONE_MINUS_SRC_ALPHA" } },
		{ BlendType::ONE,{ GL_ONE ,"ONE" } },
		{ BlendType::ONE_MINUS_DST_COLOR,{ GL_ONE_MINUS_DST_COLOR, "ONE_MINUS_DST_COLOR" } },
		{ BlendType::SRC_COLOR,{ GL_SRC_COLOR, "SRC_COLOR" } },
		{ BlendType::DST_COLOR,{ GL_DST_COLOR , "DST_COLOR" } }
	};


	// Converts a blend mode in to a string
	bool convert_blendtype_to_string(const BlendType& inValue, std::string& outValue)
	{
		BlendType mode = inValue;
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
	bool convert_string_to_blendtype(const std::string& inValue, BlendType& outValue)
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

RTTI_DEFINE_DATA(nap::BlendType)
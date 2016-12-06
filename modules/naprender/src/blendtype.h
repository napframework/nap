#pragma once

// External Includes
#include <nap/rttinap.h>
#include <nap/configure.h>
#include <unordered_map>
#include <nap/arrayattribute.h>
#include <nap/attribute.h>

namespace nap
{
	/**
	 * render available blend types, derived from common GL blend types
	 */
	enum class BlendType : nap::uint
	{
		SRC_ALPHA,
		ONE_MINUS_SRC_ALPHA,
		ONE,
		ONE_MINUS_DST_COLOR,
		SRC_COLOR,
		DST_COLOR,
	};

	// Defines a blend mode together with a name
	struct BlendDefinition
	{
		BlendDefinition(nap::uint inGLid, const std::string& inName) :
			mGLid(inGLid),
			mBindingName(inName) { }

		nap::uint		mGLid;
		std::string		mBindingName;
	};

	// All the blend modes as names (in order as defined above)
	using OFBlendModeMap = std::unordered_map<BlendType, BlendDefinition>;

	// Map containing all available blend modes
	extern const nap::OFBlendModeMap gBlendTypes;

	// Type converters
	bool convert_string_to_blendtype(const std::string& inValue, BlendType& outValue);
	bool convert_blendtype_to_string(const BlendType& inValue, std::string& outValue);
}

namespace std
{
	template<>
	struct hash<nap::BlendType> {
		size_t operator()(const nap::BlendType &k) const {
			return hash<int>()((int)k);
		}
	};
}

RTTI_DECLARE_DATA(nap::BlendType)


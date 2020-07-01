// Local Includes
#include "websocketutils.h"

// External Includes
#include <rtti/typeinfo.h>

namespace nap
{
	RTTI_BEGIN_ENUM(nap::EWebSocketLogLevel)
		RTTI_ENUM_VALUE(nap::EWebSocketLogLevel::None,		"None"),
		RTTI_ENUM_VALUE(nap::EWebSocketLogLevel::Debug,		"Debug"),
		RTTI_ENUM_VALUE(nap::EWebSocketLogLevel::Library,	"Library"),
		RTTI_ENUM_VALUE(nap::EWebSocketLogLevel::Info,		"Info"),
		RTTI_ENUM_VALUE(nap::EWebSocketLogLevel::Warning,	"Warning"),
		RTTI_ENUM_VALUE(nap::EWebSocketLogLevel::Error,		"Error"),
		RTTI_ENUM_VALUE(nap::EWebSocketLogLevel::Fatal,		"Fatal"),
		RTTI_ENUM_VALUE(nap::EWebSocketLogLevel::All,		"All")
	RTTI_END_ENUM


	uint32 computeWebSocketLogLevel(EWebSocketLogLevel level)
	{
		// All levels will be logged
		if (level == EWebSocketLogLevel::All)
			return static_cast<uint32>(EWebSocketLogLevel::All);

		uint32 comp_v = static_cast<uint32>(level);
		uint32 retu_v = 0;

		// Iterate over all the enum values and combine into a single field
		rttr::enumeration log_enum = RTTI_OF(nap::EWebSocketLogLevel).get_enumeration();
		for (auto value : log_enum.get_values())
		{	
			// Skip levels below the one requested and the all field
			// Levels equal or above the requested log level are added
			uint32 cur_v = value.to_uint32();
			if(cur_v >= comp_v && cur_v != static_cast<uint32>(EWebSocketLogLevel::All))
				retu_v |= cur_v;
		}
		return retu_v;
	}
}
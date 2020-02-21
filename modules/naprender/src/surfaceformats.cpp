#include "surfaceformats.h"
#include "rtti/typeinfo.h"

RTTI_BEGIN_ENUM(nap::ESurfaceChannels)
	RTTI_ENUM_VALUE(nap::ESurfaceChannels::R,		"R"),
	RTTI_ENUM_VALUE(nap::ESurfaceChannels::RGB,		"RGB"),
	RTTI_ENUM_VALUE(nap::ESurfaceChannels::RGBA,	"RGBA"),
	RTTI_ENUM_VALUE(nap::ESurfaceChannels::BGR,		"BGR"),
	RTTI_ENUM_VALUE(nap::ESurfaceChannels::BGRA,	"BGRA")
RTTI_END_ENUM

RTTI_BEGIN_ENUM(nap::ESurfaceDataType)
	RTTI_ENUM_VALUE(nap::ESurfaceDataType::BYTE,		"Byte"),
	RTTI_ENUM_VALUE(nap::ESurfaceDataType::USHORT,		"Short"),
	RTTI_ENUM_VALUE(nap::ESurfaceDataType::FLOAT,		"Float")
RTTI_END_ENUM

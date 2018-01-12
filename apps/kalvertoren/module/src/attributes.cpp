#include <utility/datetimeutils.h>
#include <rtti/typeinfo.h>

RTTI_BEGIN_ENUM(nap::utility::EDay)
	RTTI_ENUM_VALUE(nap::utility::EDay::Monday,		"Monday"),
	RTTI_ENUM_VALUE(nap::utility::EDay::Tuesday,	"Tuesday"),
	RTTI_ENUM_VALUE(nap::utility::EDay::Wednesday,	"Wednesday"),
	RTTI_ENUM_VALUE(nap::utility::EDay::Thursday,	"Thursday"),
	RTTI_ENUM_VALUE(nap::utility::EDay::Friday,		"Friday"),
	RTTI_ENUM_VALUE(nap::utility::EDay::Saturday,	"Saturday"),
	RTTI_ENUM_VALUE(nap::utility::EDay::Sunday,		"Sunday")
RTTI_END_ENUM

RTTI_BEGIN_ENUM(nap::utility::EMonth)
	RTTI_ENUM_VALUE(nap::utility::EMonth::January,	"January"),
	RTTI_ENUM_VALUE(nap::utility::EMonth::February,	"February"),
	RTTI_ENUM_VALUE(nap::utility::EMonth::March,	"March"),
	RTTI_ENUM_VALUE(nap::utility::EMonth::April,	"April"),
	RTTI_ENUM_VALUE(nap::utility::EMonth::May,		"May"),
	RTTI_ENUM_VALUE(nap::utility::EMonth::June,		"June"),
	RTTI_ENUM_VALUE(nap::utility::EMonth::July,		"July"),
	RTTI_ENUM_VALUE(nap::utility::EMonth::August,	"August"),
	RTTI_ENUM_VALUE(nap::utility::EMonth::September,"September"),
	RTTI_ENUM_VALUE(nap::utility::EMonth::October,	"October"),
	RTTI_ENUM_VALUE(nap::utility::EMonth::November, "November"),
	RTTI_ENUM_VALUE(nap::utility::EMonth::December, "December")
RTTI_END_ENUM
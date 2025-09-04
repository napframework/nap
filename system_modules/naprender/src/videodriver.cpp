/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local includes
#include "videodriver.h"

// External includes
#include <rtti/typeinfo.h>
#include <utility/stringutils.h>
#include <assert.h>

RTTI_BEGIN_ENUM(nap::EVideoDriver)
	RTTI_ENUM_VALUE(nap::EVideoDriver::Default,		"System Default"),
	RTTI_ENUM_VALUE(nap::EVideoDriver::Windows,		"Windows"),
	RTTI_ENUM_VALUE(nap::EVideoDriver::X11,			"X11"),
	RTTI_ENUM_VALUE(nap::EVideoDriver::Wayland,		"Wayland")
RTTI_END_ENUM

namespace  nap
{
	std::string toString(EVideoDriver driver)
	{
		return driver == EVideoDriver::Unknown ? "Unknown" : 
			RTTI_OF(EVideoDriver).get_enumeration().value_to_name(driver).to_string();
	}


	EVideoDriver fromString(const std::string& driverName)
	{
		auto videos_enum = RTTI_OF(nap::EVideoDriver).get_enumeration();
		auto driver_name = utility::toLower(driverName);

		for (const auto& option : videos_enum.get_values())
		{
			auto option_name =  utility::toLower(videos_enum.value_to_name(option).to_string());
			if (option_name == driver_name)
			{
				auto driver = option.get_value<EVideoDriver>();
				return driver;
			}
		}
		return EVideoDriver::Unknown;
	}
}

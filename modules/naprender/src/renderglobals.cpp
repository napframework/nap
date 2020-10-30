/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "renderglobals.h"

// External Includes
#include <sstream>

namespace nap
{
	const std::string vertexid::getUVName(int uvChannel)
	{
		std::ostringstream stream;
		stream << "UV" << uvChannel;
		return stream.str();
	}


	const std::string vertexid::getColorName(int colorChannel)
	{
		std::ostringstream stream;
		stream << "Color" << colorChannel;
		return stream.str();
	}


	const std::string vertexid::shader::getUVInputName(int channel)
	{
		std::ostringstream stream;
		stream << "in_UV" << channel;
		return stream.str();
	}


	const std::string vertexid::shader::getColorInputName(int channel)
	{
		std::ostringstream stream;
		stream << "in_Color" << channel;
		return stream.str();
	}
}
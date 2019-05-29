#pragma once

#include "flexblockdata.h"
#include "utility/errorstate.h"

#include <vector>
#include <memory>

namespace nap
{
	namespace flexreader
	{
		std::vector<FlexblockShapePtr> readShapes(std::string filename, utility::ErrorState& errorState);
		bool readSizes(std::string filename, std::vector<FlexblockSizePtr>& outSizes, utility::ErrorState& errorState);
	}
}
	
#pragma once

#include "flexblockdata.h"
#include "utility/errorstate.h"

#include <vector>
#include <memory>

namespace nap
{
	namespace flexreader
	{
		std::vector<std::shared_ptr<FlexblockShape>> readShapes(std::string filename, utility::ErrorState& errorState);
		std::vector<std::shared_ptr<FlexblockSize>> readSizes(std::string filename, utility::ErrorState& errorState);
	}
}
	
#pragma once

// STL includes
#include <string>
#include <vector>

// Local includes
#include "utility/dllexport.h"
#include "utility/errorstate.h"

#define PROJECT_INFO_FILENAME "project.json"

namespace nap
{
	struct ProjectInfo
	{
		std::string					mTitle;					// Project title
		std::string					mVersion;				// Project version
		std::vector<std::string>	mModules;				// Project modules
		std::string					mAlternativeDataPath;	// Path for data directory if other than 'data' next to binary
	};

	/**
	 * Deserialize a set of objects and their data from the specified JSON string
	 *
	 * @param json The JSON to deserialize from
	 * @param result The result of deserialization
	 * @param errorState The error state of deserialization
	 *
	 * @return True if deserialization succeeded, false if not. In case of failure, @a errorState contains detailed error info.
	 */
	bool NAPAPI deserializeProjectInfoJSON(const std::string& json, ProjectInfo& result, utility::ErrorState& errorState);
	
	/**
	 * Read and deserialize a set of objects and their data from the specified JSON file
	 *
	 * @param json The JSON file to deserialize from
	 * @param result The result of deserialization
	 * @param errorState The error state of deserialization
	 *
	 * @return True if deserialization succeeded, false if not. In case of failure, @a errorState contains detailed error info.
	 */
	bool NAPAPI loadProjectInfoFromJSON(ProjectInfo& result, utility::ErrorState& errorState);

} //< End Namespace nap

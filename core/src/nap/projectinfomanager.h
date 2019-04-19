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
	class Core;

	struct ProjectInfo
	{
		std::string					mTitle;					// Project title
		std::string					mVersion;				// Project version
		std::vector<std::string>	mModules;				// Project modules
		std::string					mAlternativeDataPath;	// Path for data directory if other than 'data' next to binary
	};

	/**
	 * Deserialize project information from the specified JSON string
	 *
	 * @param json The JSON to deserialize from
	 * @param result The resulting project information
	 * @param errorState The error state of deserialization
	 *
	 * @return True if deserialization succeeded, false if not. In case of failure, @a errorState contains detailed error info.
	 */
	bool NAPAPI deserializeProjectInfoJSON(const std::string& json, ProjectInfo& result, utility::ErrorState& errorState);
	
	/**
	 * Read and deserialize and project information from JSON file
	 *
	 * @param result The resulting project information
	 * @param errorState The error state of deserialization
	 *
	 * @return True if read and deserialization succeeded, false if not. In case of failure, @a errorState contains detailed error info.
	 */
 	bool NAPAPI loadProjectInfoFromFile(const Core &core, ProjectInfo &result, utility::ErrorState &errorState);

    /**
     * Read and deserialize and project information from JSON stream
     *
     * @param result The resulting project information
     * @param errorState The error state of deserialization
     * @param in The input stream to read from
     * @return True if read and deserialization succeeded, false if not. In case of failure, @a errorState contains detailed error info.
     */
	bool NAPAPI loadProjectInfoFromStream(const Core& core, ProjectInfo& result, utility::ErrorState& err, std::istream& in);

} //< End Namespace nap

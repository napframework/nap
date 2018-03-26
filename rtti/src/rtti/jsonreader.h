#pragma once

// Local Includes
#include "rtti.h"
#include "rtti/path.h"
#include "deserializeresult.h"
#include "epropertyvalidationmode.h"

// External Includes
#include "utility/dllexport.h"

// STL includes
#include <string>

namespace nap
{
	namespace utility
	{
		class ErrorState;
	}

	namespace rtti
	{
		class Factory;

		/**
		 * Deserialize a set of objects and their data from the specified JSON string
		 *
		 * @param json The JSON to deserialize from
		 * @param propertyValidationMode Whether missing required properties should be treated as errors
		 * @param result The result of deserialization
		 * @param errorState The error state of deserialization
		 *
		 * @return True if deserialization succeeded, false if not. In case of failure, @a errorState contains detailed error info.
		 */
		bool NAPAPI deserializeJSON(const std::string& json, EPropertyValidationMode propertyValidationMode, Factory& factory, DeserializeResult& result, utility::ErrorState& errorState);

		/**
		 * Read and deserialize a set of objects and their data from the specified JSON file
		 *
		 * @param json The JSON file to deserialize from
		 * @param propertyValidationMode Whether missing required properties should be treated as errors
		 * @param result The result of deserialization
		 * @param errorState The error state of deserialization
		 *
		 * @return True if deserialization succeeded, false if not. In case of failure, @a errorState contains detailed error info.
		 */
		bool NAPAPI readJSONFile(const std::string& path, EPropertyValidationMode propertyValidationMode, Factory& factory, DeserializeResult& result, utility::ErrorState& errorState);
	} //< End Namespace nap

}

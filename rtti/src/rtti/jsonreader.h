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
		 * @param json The JSON to deserialize
		 * @param propertyValidationMode whether missing required properties should be treated as errors
		 * @param pointerPropertyMode controls ownership of the created objects. Use 'NoRawPointers' in process and 'OnlyRawPointers' out of process.
		 * @param factory RTTI object factory. 
		 * @param result the result of the deserialization operation
		 * @param errorState contains the error when de-serialization fails.
		 *
		 * @return true if deserialization succeeded, false if not. In case of failure the errorState contains detailed error info.
		 */
		bool NAPAPI deserializeJSON(const std::string& json, EPropertyValidationMode propertyValidationMode, EPointerPropertyMode pointerPropertyMode, Factory& factory, DeserializeResult& result, utility::ErrorState& errorState);

		/**
		 * Read and deserialize a set of objects and their data from the specified JSON file
		 *
		 * @param path The JSON file to deserialize
		 * @param propertyValidationMode Whether missing required properties should be treated as errors
		 * @param pointerPropertyMode controls ownership of the created objects. Use 'NoRawPointers' in process and 'OnlyRawPointers' out of process.
		 * @param factory RTTI object factory.
		 * @param result The result of the deserialization operation
		 * @param errorState contains the error when deserialization fails.
		 *
		 * @return true if deserialization succeeded, false if not. In case of failure the errorState contains detailed error info.
		 */
		bool NAPAPI readJSONFile(const std::string& path, EPropertyValidationMode propertyValidationMode, EPointerPropertyMode pointerPropertyMode, Factory& factory, DeserializeResult& result, utility::ErrorState& errorState);

	} //< End Namespace nap

}

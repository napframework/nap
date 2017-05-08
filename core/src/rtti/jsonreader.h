#pragma once

// RTTI includes
#include <rtti/rtti.h>
#include <rtti/rttipath.h>
#include <rtti/rttireader.h>

// STL includes
#include <string>

namespace nap
{
	class Object;
	struct InitResult;

	/**
	 * Deserialize a set of objects and their data from the specified JSON string
	 *
	 * @param json The JSON to deserialize from
	 * @param result The result of deserialization
	 * @param initResult The error state of deserialization
	 *
	 * @return True if deserialization succeeded, false if not. In case of failure, @a initResult contains detailed error info.
	 */
	bool deserializeJSON(const std::string& json, RTTIDeserializeResult& result, nap::InitResult& initResult);

	/**
	 * Read and deserialize a set of objects and their data from the specified JSON file
	 *
	 * @param json The JSON file to deserialize from
	 * @param result The result of deserialization
	 * @param initResult The error state of deserialization
	 *
	 * @return True if deserialization succeeded, false if not. In case of failure, @a initResult contains detailed error info.
	 */
	bool readJSONFile(const std::string& filename, RTTIDeserializeResult& result, nap::InitResult& initResult);
} //< End Namespace nap


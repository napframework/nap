#pragma once

// NAP includes
#include <utility/memorystream.h>
#include "utility/dllexport.h"


// RTTI includes
#include <rtti/rtti.h>
#include <rtti/path.h>
#include <rtti/deserializeresult.h>

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
		class Object;
		class Factory;

		/**
		 * Check whether the binary file at the specified path matches the current binary version (i.e. is up-to-date)
		 *
		 * @param file The binary file to check
		 *
		 * @return True if the binary versions of the types in the file are up-to-date, false if not.
		 */
		bool NAPAPI checkBinaryVersion(const std::string& path);

		/**
		 * Deserialize a set of objects and their data from the specified stream
		 *
		 * @param stream The stream to deserialize from
		 * @param result The result of deserialization
		 * @param errorState The error state of deserialization
		 *
		 * @return True if deserialization succeeded, false if not. In case of failure, @a errorState contains detailed error info.
		 */
		bool NAPAPI deserializeBinary(utility::MemoryStream& stream, Factory& factory, DeserializeResult& result, utility::ErrorState& errorState);

		/**
		 * Deserialize a set of objects and their data from the specified file
		 *
		 * @param file The file to deserialize from
		 * @param result The result of deserialization
		 * @param errorState The error state of deserialization
		 *
		 * @return True if deserialization succeeded, false if not. In case of failure, @a errorState contains detailed error info.
		 */
		bool NAPAPI readBinary(const std::string& path, Factory& factory, DeserializeResult& result, utility::ErrorState& errorState);
	}

}

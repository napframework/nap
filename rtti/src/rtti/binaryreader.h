/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local includes
#include "rtti.h"
#include "path.h"
#include "deserializeresult.h"

// External includes
#include <utility/memorystream.h>
#include <utility/dllexport.h>
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
		 * @param path path to the binary file.
		 * @return true if the binary versions of the types in the file are up-to-date, false if not.
		 */
		bool NAPAPI checkBinaryVersion(const std::string& path);

		/**
		 * Deserialize a set of objects and their data from the specified stream
		 * @param stream the stream to deserialize from
		 * @param factory the RTTI object factory.
		 * @param result The result of the deserialization process
		 * @param errorState contains the error if deserialization fails.
		 * @return true if deserialization succeeded, false if not. In case of failure, errorState contains detailed error info.
		 */
		bool NAPAPI deserializeBinary(utility::MemoryStream& stream, Factory& factory, DeserializeResult& result, utility::ErrorState& errorState);

		/**
		 * Deserialize a set of objects and their data from the specified file
		 * @param path path to file.
		 * @param factory the RTTI object factory.
		 * @param result The result of the deserialization process.
		 * @param errorState contains the error if deserialization fails.
		 * @return true if deserialization succeeded, false if not. In case of failure, errorState contains detailed error info.
		 */
		bool NAPAPI readBinary(const std::string& path, Factory& factory, DeserializeResult& result, utility::ErrorState& errorState);
	}

}

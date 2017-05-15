#pragma once

// NAP includes
#include <nap/memorystream.h>

// RTTI includes
#include <rtti/rtti.h>
#include <rtti/rttipath.h>
#include <rtti/rttireader.h>

// STL includes
#include <string>

namespace nap
{
	class Object;
	class ErrorState;
	class Factory;

	/**
	 * Deserialize a set of objects and their data from the specified stream
	 *
	 * @param stream The stream to deserialize from
	 * @param result The result of deserialization
	 * @param errorState The error state of deserialization
	 *
	 * @return True if deserialization succeeded, false if not. In case of failure, @a errorState contains detailed error info.
	 */
	bool deserializeBinary(MemoryStream& stream, Factory& factory, RTTIDeserializeResult& result, ErrorState& errorState);

	/**
	 * Deserialize a set of objects and their data from the specified file
	 *
	 * @param file The file to deserialize from
	 * @param result The result of deserialization
	 * @param errorState The error state of deserialization
	 *
	 * @return True if deserialization succeeded, false if not. In case of failure, @a errorState contains detailed error info.
	 */
	bool readBinary(const std::string& path, Factory& factory, RTTIDeserializeResult& result, ErrorState& errorState);
} //< End Namespace nap


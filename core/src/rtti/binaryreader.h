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
	struct InitResult;

	/**
	 * Deserialize a set of objects and their data from the specified stream
	 *
	 * @param stream The stream to deserialize from
	 * @param result The result of deserialization
	 * @param initResult The error state of deserialization
	 *
	 * @return True if deserialization succeeded, false if not. In case of failure, @a initResult contains detailed error info.
	 */
	bool deserializeObjects(MemoryStream& stream, RTTIDeserializeResult& result, InitResult& initResult);
} //< End Namespace nap


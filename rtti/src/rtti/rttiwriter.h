#pragma once

#include "rtti/typeinfo.h"
#include <vector>
#include "utility/dllexport.h"
#include "rttiutilities.h"

namespace nap
{
	namespace utility
	{
		class ErrorState;
	}

	namespace rtti
	{
		class RTTIObject;

		/**
		 * This is the interface used by serializeObjects to serialize RTTI objects.
		 * Having this interface allows the object hierarchy traversal logic to remain separate from the actual writing logic.
		 * This in turn means that the same interface (serializeObjects) can be used to write to a variety of formats (JSON, binary, BSON, etc).
		 */
		class NAPAPI RTTIWriter
		{
		public:

			/**
			 * Called when serialization starts, but before any objects have been written (i.e. start of 'document')
			 *
			 * @param rootObjects The list of root objects that will be written to the writer
			 */
			virtual bool start(const ObjectList& rootObjects) = 0;

			/**
			 * Called when serialization is finished, after everything has been written (i.e. end of 'document')
			 */
			virtual bool finish() = 0;

			/**
			 * Called when a root object of the specified type is about to be written
			 */
			virtual bool startRootObject(const rtti::TypeInfo& type) = 0;

			/**
			 * Called when a root object has been completely written
			 */
			virtual bool finishRootObject() = 0;

			/**
			 * Called when a compound (i.e. struct nested inside a root object) of the specified type is about to be written
			 */
			virtual bool startCompound(const rtti::TypeInfo& type) = 0;

			/**
			 * Called when a compound has been completely written
			 */
			virtual bool finishCompound() = 0;

			/**
			 * Called when an array of the specified length is about to be written. Note that the elements are written in a separate call (writePointer or writePrimitive)
			 */
			virtual bool startArray(int length) = 0;

			/**
			 * Called when an array has been completely written
			 */
			virtual bool finishArray() = 0;

			/**
			 * Called to write a property of the specified name. Note that the value for the property is written in a separate call (writePointer or writePrimitive)
			 */
			virtual bool writeProperty(const std::string& propertyName) = 0;

			/**
			 * Called to write a pointer to an object with the specified ID
			 */
			virtual bool writePointer(const std::string& pointeeID) = 0;

			/**
			 * Called to write a primitive type with the specified value
			 */
			virtual bool writePrimitive(const rtti::TypeInfo& type, const rtti::Variant& value) = 0;

			/**
			 * Called to determine if this writer supports writing pointers nested in the object pointing to them (embedded pointers)
			 */
			virtual bool supportsEmbeddedPointers() const = 0;
		};

		/**
		 * Serialize a set of objects to the specified writer. This function does all the traversal logic, the actual writing is done by the RTTIWriter passed in.
		 */
		bool NAPAPI serializeObjects(const ObjectList& rootObjects, RTTIWriter& writer, utility::ErrorState& errorState);
	}
}

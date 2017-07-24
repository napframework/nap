#pragma once

#include "rttiwriter.h"

#include <rapidjson/prettywriter.h>
#include <rapidjson/document.h>

namespace nap
{
	namespace rtti
	{
		class JSONWriter : public RTTIWriter
		{
		public:
			JSONWriter();

			/**
			 * Get the generated json
			 */
			std::string GetJSON();

			/**
			 * Called to determine if this writer supports writing pointers nested in the object pointing to them (embedded pointers)
			 */
			bool supportsEmbeddedPointers() const override { return true; }

			/**
			 * Called when serialization starts, but before any objects have been written (i.e. start of 'document')
			 */
			bool start() override;

			/**
			 * Called when serialization is finished, after everything has been written (i.e. end of 'document')
			 */
			bool finish() override;

			/**
			 * Called when a root object of the specified type is about to be written
			 */
			bool startRootObject(const rtti::TypeInfo& type) override;

			/**
			 * Called when a root object has been completely written
			 */
			bool finishRootObject() override;

			/**
			 * Called when a compound (i.e. struct nested inside a root object) of the specified type is about to be written
			 */
			bool startCompound(const rtti::TypeInfo& type) override;

			/**
			 * Called when a compound has been completely written
			 */
            bool finishCompound() override;

			/**
			 * Called when an array of the specified length is about to be written. Note that the elements are written in a separate call (writePointer or writePrimitive)
			 */
			bool startArray(int length) override;

			/**
			 * Called when an array has been completely written
			 */
			bool finishArray() override;

			/**
			 * Called to write a property of the specified name. Note that the value for the property is written in a separate call (writePointer or writePrimitive)
			 */
			bool writeProperty(const std::string& propertyName) override;

			/**
			 * Called to write a pointer to an object with the specified ID
			 */
			bool writePointer(const std::string& pointeeID) override;

			/**
			 * Called to write a primitive type with the specified value
			 */
			bool writePrimitive(const rtti::TypeInfo& type, const rtti::Variant& value) override;

		private:
			rapidjson::StringBuffer								mStringBuffer;	// The string buffer we're writing to
			rapidjson::PrettyWriter<rapidjson::StringBuffer>	mWriter;		// The json writer itself
		};
	}
}

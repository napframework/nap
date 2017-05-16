#pragma once

#include "rttiwriter.h"

#include <rapidjson/prettywriter.h>
#include <rapidjson/document.h>

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
		virtual bool supportsEmbeddedPointers() const { return true; }

		/**
		 * Called when serialization starts, but before any objects have been written (i.e. start of 'document')
		 */
		virtual bool start() override;

		/**
		 * Called when serialization is finished, after everything has been written (i.e. end of 'document')
		 */
		virtual bool finish() override;

		/**
		 * Called when a root object of the specified type is about to be written
		 */
		virtual bool startRootObject(const rtti::TypeInfo& type) override;

		/**
		 * Called when a root object has been completely written
		 */
		virtual bool finishRootObject() override;

		/**
		 * Called when a compound (i.e. struct nested inside a root object) of the specified type is about to be written
		 */
		virtual bool startCompound(const rtti::TypeInfo& type) override;

		/**
		 * Called when a compound has been completely written
		 */
		virtual bool finishCompound() override;

		/**
		 * Called when an array of the specified length is about to be written. Note that the elements are written in a separate call (writePointer or writePrimitive)
		 */
		virtual bool startArray(int length) override;

		/**
		 * Called when an array has been completely written
		 */
		virtual bool finishArray() override;

		/**
		 * Called to write a property of the specified name. Note that the value for the property is written in a separate call (writePointer or writePrimitive)
		 */
		virtual bool writeProperty(const std::string& propertyName) override;

		/**
		 * Called to write a pointer to an object with the specified ID
		 */
		virtual bool writePointer(const std::string& pointeeID) override;

		/** 
		 * Called to write a primitive type with the specified value
		 */
		virtual bool writePrimitive(const rtti::TypeInfo& type, const rtti::Variant& value) override;

	private:
		rapidjson::StringBuffer								mStringBuffer;	// The string buffer we're writing to
		rapidjson::PrettyWriter<rapidjson::StringBuffer>	mWriter;		// The json writer itself
	};
}

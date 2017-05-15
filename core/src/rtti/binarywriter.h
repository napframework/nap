#pragma once

#include "rttiwriter.h"
#include <vector>

namespace nap
{
	class BinaryWriter : public nap::RTTIWriter
	{
	public:
		/**
		 * Get the buffer
		 */
		const std::vector<uint8_t>& getBuffer() const
		{
			return mBuffer;
		}

		/**
		 * Called to determine if this writer supports writing pointers nested in the object pointing to them (embedded pointers)
		 */
		virtual bool supportsEmbeddedPointers() const { return false; }

		/**
		 * Called when serialization starts, but before any objects have been written (i.e. start of 'document')
		 */
		virtual bool start() override;

		/**
		 * Called when serialization is finished, after everything has been written (i.e. end of 'document')
		 */
		virtual bool finish() override { return true; }

		/**
		 * Called when a root object of the specified type is about to be written
		 */
		virtual bool startRootObject(const RTTI::TypeInfo& type) override;

		/**
		 * Called when a root object has been completely written
		 */
		virtual bool finishRootObject() override { return true; }

		/**
		 * Called when a compound (i.e. struct nested inside a root object) of the specified type is about to be written
		 */
		virtual bool startCompound(const RTTI::TypeInfo& type) override { return true; }

		/**
		 * Called when a compound has been completely written
		 */
		virtual bool finishCompound() override { return true; }

		/**
		 * Called when an array of the specified length is about to be written. Note that the elements are written in a separate call (writePointer or writePrimitive)
		 */
		virtual bool startArray(int length) override;

		/**
		 * Called when an array has been completely written
		 */
		virtual bool finishArray() override { return true; }

		/**
		 * Called to write a property of the specified name. Note that the value for the property is written in a separate call (writePointer or writePrimitive)
		 */
		virtual bool writeProperty(const std::string& propertyName) override { return true; }

		/**
		 * Called to write a pointer to an object with the specified ID
		 */
		virtual bool writePointer(const std::string& pointeeID) override;

		/** 
		 * Called to write a primitive type with the specified value
		 */
		virtual bool writePrimitive(const RTTI::TypeInfo& type, const RTTI::Variant& value) override;

	private:
		/**
		 * Ensure there's enough room in the buffer to write the specified amount of bytes
		 *
		 * @param numBytes The number of bytes to make room for
		 */
		void ensureHasRoom(uint32_t numBytes);
		
		/**
		 * Write data of the specified number of bytes to the buffer
		 *
		 * @param data The data to write
		 * @param numBytes The length of the data
		 */
		void write(const void* data, uint32_t length);

		/**
		 * Helper function to write primitives of type T
		 */
		template <class T>
		void write(const T& data)
		{
			write(&data, sizeof(T));
		}

		/**
		 * Write the string to the buffer
		 *
		 * @param string The string to write
		 */
		void writeString(const std::string& string)
		{
			writeString(string.data(), string.length());
		}

		/**
		 * Write the string to the buffer
		 *
		 * @param string The string to write
		 * @param length The length of the string
		 */
		void writeString(const char* string, size_t length)
		{
			write(length);
			write(string, length);
		}

	private:
		std::vector<uint8_t>	mBuffer;
		uint8_t*				mWritePointer = nullptr;
	};
}

/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "writer.h"
#include <utility/dllexport.h>
#include <vector>

namespace nap
{
	namespace rtti
	{
		class NAPAPI BinaryWriter : public Writer
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
			bool supportsEmbeddedPointers() const override { return false; }

			/**
			 * Called when serialization starts, but before any objects have been written (i.e. start of 'document')
			 */
			bool start(const ObjectList& rootObjects) override;

			/**
			 * Called when serialization is finished, after everything has been written (i.e. end of 'document')
			 */
			bool finish() override { return true; }

			/**
			 * Called when a root object of the specified type is about to be written
			 */
			bool startRootObject(const rtti::TypeInfo& type) override;

			/**
			 * Called when a root object has been completely written
			 */
			bool finishRootObject() override { return true; }

			/**
			 * Called when a compound (i.e. struct nested inside a root object) of the specified type is about to be written
			 */
			bool startCompound(const rtti::TypeInfo& type) override { return true; }

			/**
			 * Called when a compound has been completely written
			 */
			bool finishCompound() override { return true; }

			/**
			 * Called when an array of the specified length is about to be written. Note that the elements are written in a separate call (writePointer or writePrimitive)
			 */
			bool startArray(int length) override;

			/**
			 * Called when an array has been completely written
			 */
			bool finishArray() override { return true; }

			/**
			 * Called to write a property of the specified name. Note that the value for the property is written in a separate call (writePointer or writePrimitive)
			 */
			bool writeProperty(const std::string& propertyName) override { return true; }

			/**
			 * Called to write a pointer to an object with the specified ID
			 */
			bool writePointer(const std::string& pointeeID) override;

			/**
			 * Called to write a primitive type with the specified value
			 */
			bool writePrimitive(const rtti::TypeInfo& type, const rtti::Variant& value) override;

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
			void writeString(const std::string& string);

			/**
			 * Write the string to the buffer
			 *
			 * @param string The string to write
			 * @param length The length of the string
			 */
			void writeString(const char* string, size_t length);

			/**
			 * Get the current position in the stream
			 *
			 * @return The position in the stream
			 */
			size_t getPosition() const;


			/**
			 * Seek to the specified position in the stream
			 */
			void seek(size_t position);

		private:
			std::vector<uint8_t>	mBuffer;
			uint8_t*				mWritePointer = nullptr;
		};
	}
}

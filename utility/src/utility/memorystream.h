/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <string>
#include <memory>
#include <assert.h>
#include <cstring>

namespace nap
{
	namespace utility
	{
		/**
		 * A MemoryStream allows you to read data from a memory buffer
		 */
		class MemoryStream
		{
		public:
			/**
			 * Constructor
			 *
			 * @param buffer The buffer to read from
			 * @param length The length of the buffer we're reading from
			 */
			MemoryStream(const uint8_t* buffer, uint32_t length) :
				mBuffer(buffer),
				mLength(length),
				mReadPos(buffer)
			{
			}

			/**
			 * Checks whether all data in this stream has been read
			 *
			 * @return True if all data has been read, false if not
			 */
			bool isDone() const
			{
				return mReadPos >= (mBuffer + mLength);
			}

			/**
			 * Checks whether the specified amount of bytes is available for reading
			 *
			 * @param size The number of bytes desired for reading
			 * @return True if the specified number of bytes can be read, false if not
			 */
			bool hasAvailable(uint32_t size)
			{
				return (mLength - (mReadPos - mBuffer)) >= size;
			}

			/**
			 * Function to read the specified number of bytes from the stream to a buffer
			 *
			 * @param data The buffer to read to
			 * @param length The number of bytes to read
			 */
			void read(void* data, uint32_t length)
			{
				assert(hasAvailable(length));

				std::memcpy(data, mReadPos, length);
				mReadPos += length;
			}

			/**
			 * Helper function to read a primitive of type T from the stream
			 *
			 * @param data The buffer to read to
			 */
			template<class T>
			void read(T& data)
			{
				read(&data, sizeof(T));
			}

			/**
			 * Helper function to read a primitive of type T from the stream (returns by value)
			 *
			 * @return The data that was read
			 */
			template<class T>
			const T read()
			{
				T value;
				read(value);

				return value;
			}

			/**
			 * Helper function to read a string from the stream
			 *
			 * @param string The string to read the data to
			 */
			void readString(std::string& string)
			{
				// Read the length of the string
				size_t length;
				read(length);

				// Resize string and read data
				string.resize(length);
				read((void*)string.data(), length);
			}

		private:
			const uint8_t*	mBuffer;	// The buffer we're reading from
			uint32_t		mLength;	// The length of the buffer
			const uint8_t*	mReadPos;	// Current position in the buffer we're reading from
		};
	}
}

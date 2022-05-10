/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External includes
#include <nap/core.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Sent to an endpoint by an UDPClient or created by the UDPServer upon receiving data.
	 * Light object that can be copied and moved.
	 */
	struct NAPAPI UDPPacket final
	{
	public:
		// Default constructor
		UDPPacket() = default;

		// Default copy constructor
		UDPPacket(const UDPPacket& other) = default;

		// Default copy assignment operator
		UDPPacket& operator=(const UDPPacket& other) = default;

		// Move constructor
		UDPPacket(UDPPacket&& other) noexcept							{ mBuffer = std::move(other.mBuffer); }

		// Move assignment operator
		UDPPacket& operator=(UDPPacket&& other) noexcept				{ mBuffer = std::move(other.mBuffer); return *this;  }

		/**
		 * UDPPacket constructor copies the contents of string into buffer
		 */
		UDPPacket(const std::string& string) noexcept					{ std::copy(string.begin(), string.end(), std::back_inserter(mBuffer)); }

		/**
		 * UDPPacket constructor moves the contents of supplied buffer if rvalue
		 * @param buffer the buffer to be copied
		 */
		UDPPacket(std::vector<nap::uint8>&& buffer) : mBuffer(std::move(buffer)){}

		/**
		 * UDPPacket constructor copies the contents of supplied buffer
		 * @param buffer the buffer to be copied
		 */
		UDPPacket(const std::vector<nap::uint8>& buffer) : mBuffer(buffer) {}

		/**
		 * returns const reference to vector holding data
		 * @return the data
		 */
		const std::vector<nap::uint8>& data() const { return mBuffer; }

		/**
		 * returns size of data buffer
		 * @return size of data buffer
		 */
		size_t size() const{ return mBuffer.size(); }

		/**
		 * @return string with contents of internal buffer
		 */
		std::string toString() const{ return std::string(mBuffer.begin(), mBuffer.end()); }
	private:
		std::vector<nap::uint8> mBuffer; ///< Vector containing packet data
	};
}

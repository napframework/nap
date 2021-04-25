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
	 * A UdpPacket can be send by a UdpClient to an endpoint, or created by the UdpServer upon receiving data
	 * A UdpPacket can be moved or copied
	 */
	struct NAPAPI UdpPacket
	{
	public:
		/**
		 * UdpPacket constructor
		 */
		UdpPacket(){}

		/**
		 * UdpPacket constructor copies the contents of supplied buffer
		 * @param buffer the buffer to be copied
		 */
		UdpPacket(std::vector<nap::uint8> buffer) : mBuffer(std::move(buffer)){}

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
	private:
		std::vector<nap::uint8> mBuffer; ///< Vector containing packet data
	};
}

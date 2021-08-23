/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "artdmxpacketevent.h"

// External Includes
#include <cstring>


namespace nap
{
	ArtDmxPacketEvent::ArtDmxPacketEvent(uint8_t sequence, uint8_t physical, uint16_t portAddress)
		: mSequence(sequence)
		, mPhysical(physical)
		, mNet(static_cast<uint8_t>((portAddress & 0b0111111100000000) >> 8))
		, mSubNet(static_cast<uint8_t>((portAddress & 0b000000011110000) >> 4))
		, mUniverse(static_cast<uint8_t>(portAddress & 0b000000000001111))
		, mPortAddress(portAddress)
	{

	}


	void ArtDmxPacketEvent::setData(const uint8_t* data, size_t size)
	{
		mData.resize(size);
		std::memcpy(mData.data(), data, size);
	}
}

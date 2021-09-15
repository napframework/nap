/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "artnetevent.h"

// External Includes
#include <cassert>
#include <cstring>


namespace nap
{
	ArtNetEvent::ArtNetEvent(uint8 sequence, uint8 physical, uint16 portAddress)
		: mSequence(sequence)
		, mPhysical(physical)
		, mNet(static_cast<uint8>((portAddress & 0b0111111100000000) >> 8))
		, mSubNet(static_cast<uint8>((portAddress & 0b000000011110000) >> 4))
		, mUniverse(static_cast<uint8>(portAddress & 0b000000000001111))
		, mPortAddress(portAddress)
	{ }


	void ArtNetEvent::setData(const uint8* data, size_t size)
	{
		mData.resize(size);
		std::memcpy(mData.data(), data, size);
	}


	uint8 ArtNetEvent::getChannelByIndex(uint16 index) const
	{
		assert(index < getChannelCount() && index >= 0);
		return mData.at(index);
	}
}

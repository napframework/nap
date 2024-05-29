/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <nap/event.h>
#include <cstdint>
#include <vector>
#include <memory>
#include <nap/numeric.h>

namespace nap
{
	/**
	* Represents a single received ArtDmx packet event,
	* carrying a payload of zero-start code DMX512 data.
	*/
	class NAPAPI ArtNetEvent : public Event
	{
		RTTI_ENABLE(Event)
	public:

		ArtNetEvent() = delete;

		/**
		* ArtNetEvent constructor. Creates an ArtDmx packet event identified by a Sequence field,
		* Physical field and Port-Address (which will be parsed to the Net, Sub-Net and Universe).
		* @param sequence the Sequence field of the ArtDmx packet
		* @param physical the Physical field of the ArtDmx packet
		* @param portAddress the Port-Address of the ArtDmx packet
		*/
		ArtNetEvent(uint8 sequence, uint8 physical, uint16 portAddress);

		/**
		* Sets the DMX512 channels for this ArtDmx packet.
		* @param data pointer to the raw data buffer
		* @param size the length of the data buffer in bytes
		*/
		void setData(const uint8* data, size_t size);

		/**
		* The Sequence field is an 8-bit number that is designed to show the order in which packets were
		* originated. When Art-Net is transferred over media such as the Internet, it is possible for packets
		* to arrive at their destination out of sequence. This field allows the receiver to trap such errors.
		* The generating device increments this number for every packet sent to a specific Port-Address.
		* The number increments from 1 to 255 and then rolls over to 1 and repeats.
		* This is because the value 0 is reserved to show that Sequence is not implemented.
		* @return the Sequence field of the ArtDmx packet
		*/
		uint8 getSequence() const { return mSequence; };

		/**
		* The Physical field is an 8-bit number that defines the physical port that generated the packet.
		* This number is limited to the range 0 to 3.
		* It is intended to be purely informative and is not used to define the destination of the packet.
		* @return the Physical field of the ArtDmx packet
		*/
		uint8 getPhysical() const { return mPhysical; };

		/**
		* A group of 16 consecutive Sub-Nets or 256 consecutive Universes is referred to as a Net. There are 128 Nets in total.
		* @return the Net of the ArtDmx packet
		*/
		uint8 getNet() const { return mNet; };

		/**
		* A group of 16 consecutive Universes is referred to as a Sub-Net.
		* @return the Sub-Net of the ArtDmx packet
		*/
		uint8 getSubNet() const { return mSubNet; };

		/**
		* A single DMX512 frame of 512 channels.
		* @return the Universe of the ArtDmx packet
		*/
		uint8 getUniverse() const { return mUniverse; };

		/**
		* One of the 32,768 possible addresses to which a DMX frame can be directed.
		* The Port-Address is a 15-bit number composed of Net + Sub-Net + Universe.
		* @return the Port-Address of the ArtDmx packet
		*/
		uint16 getPortAddress() const { return mPortAddress; };

		/**
		* The amount of channels present in the DMX512 frame.
		* @return the channel count of the ArtDmx packet
		*/
		size_t getChannelCount() const { return mData.size(); };

		/**
		* Get a pointer to the raw ArtDmx channel data
		* @return the pointer to the raw ArtDmx channel data
		*/
		const uint8* getChannelData() const { return mData.data(); };

		/**
		* Retrieve the 8-bit value of a single DMX512 channel by its 1-based channel number.
		* @param number the 1-based number of the DMX512 channel, ranging from 1 to the return value of getChannelCount
		* @return the 8-bit value of a single DMX512 channel in the ArtDmx packet
		*/
		uint8 getChannelByNumber(uint16 number) const { return getChannelByIndex(number - 1); };

		/**
		* Retrieve the 8-bit value of a single DMX512 channel by its 0-based channel index.
		* @param index the 0-based index of the DMX512 channel, ranging from 0 to the return value of getChannelCount - 1
		* @return the 8-bit value of a single DMX512 channel in the ArtDmx packet
		*/
		uint8 getChannelByIndex(uint16 index) const;

		/**
		* Retrieve the 8-bit value of a single DMX512 channel by its 0-based channel index.
		* @param index the 0-based index of the DMX512 channel, ranging from 0 to the return value of getChannelCount - 1
		* @return the 8-bit value of a single DMX512 channel in the ArtDmx packet
		*/
		uint8 operator[](uint16 index) { return getChannelByIndex(index); }

		/**
		* Retrieve the 8-bit value of a single DMX512 channel by its 0-based channel index.
		* @param index the 0-based index of the DMX512 channel, ranging from 0 to the return value of @getChannelCount - 1
		* @return the 8-bit value of a single DMX512 channel in the ArtDmx packet
		*/
		const uint8 operator[](uint16 index) const { return getChannelByIndex(index); }

	private:

		uint8 mSequence;
		uint8 mPhysical;
		uint8 mNet;
		uint8 mSubNet;
		uint8 mUniverse;
		uint16 mPortAddress;
		std::vector<uint8> mData;
	};

	using ArtNetEventPtr = std::unique_ptr<nap::ArtNetEvent>;
}

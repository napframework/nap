/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <nap/event.h>
#include <cstdint>
#include <vector>
#include <memory>


namespace nap
{
	/**
	* Represents a single received ArtDmx packet,
	* carrying a payload of zero-start code DMX512 data.
	*/
	class NAPAPI ArtDmxPacketEvent : public Event
	{
		RTTI_ENABLE(Event)
	public:

		ArtDmxPacketEvent() = delete;

		/**
		* ArtDmxPacketEvent constructor. Creates an ArtDmx packet identified by a Sequence field,
		* Physical field and Port-Address (which will be parsed to the Net, Sub-Net and Universe).
		* @param sequence the Sequence field of the ArtDmx packet
		* @param physical the Physical field of the ArtDmx packet
		* @param portAddress the Port-Address of the ArtDmx packet
		*/
		ArtDmxPacketEvent(uint8_t sequence, uint8_t physical, uint16_t portAddress);

		/**
		* Sets the DMX512 channels for this ArtDmx packet.
		* @param data pointer to the raw data buffer
		* @param size the length of the data buffer in bytes
		*/
		void setData(const unsigned char* data, size_t size);

		/**
		* The Sequence field is an 8-bit number that is designed to show the order in which packets were
		* originated. When Art-Net is transferred over media such as the Internet, it is possible for packets
		* to arrive at their destination out of sequence. This field allows the receiver to trap such errors.
		* The generating device increments this number for every packet sent to a specific Port-Address.
		* The number increments from 1 to 255 and then rolls over to 1 and repeats.
		* This is because the value 0 is reserved to show that Sequence is not implemented.
		* @return the Sequence field of the ArtDmx packet
		*/
		uint8_t getSequence() const { return mSequence; };

		/**
		* The Physical field is an 8-bit number that defines the physical port that generated the packet.
		* This number is limited to the range 0 to 3.
		* It is intended to be purely informative and is not used to define the destination of the packet.
		* @return the Physical field of the ArtDmx packet
		*/
		uint8_t getPhysical() const { return mPhysical; };

		/**
		* A group of 16 consecutive Sub-Nets or 256 consecutive Universes is referred to as a Net. There are 128 Nets in total.
		* @return the Net of the ArtDmx packet
		*/
		uint8_t getNet() const { return mNet; };

		/**
		* A group of 16 consecutive Universes is referred to as a Sub-Net.
		* @return the Sub-Net of the ArtDmx packet
		*/
		uint8_t getSubNet() const { return mSubNet; };

		/**
		* A single DMX512 frame of 512 channels.
		* @return the Universe of the ArtDmx packet
		*/
		uint8_t getUniverse() const { return mUniverse; };

		/**
		* One of the 32,768 possible addresses to which a DMX frame can be directed.
		* The Port-Address is a 15-bit number composed of Net + Sub-Net + Universe.
		* @return the Port-Address of the ArtDmx packet
		*/
		uint16_t getPortAddress() const { return mPortAddress; };

		/**
		* The amount of channels present in the DMX512 frame.
		* @return the channel count of the ArtDmx packet
		*/
		size_t getChannelCount() const { return mData.size(); };

		/**
		* Retrieve the 8-bit value of a single DMX512 channel by its 1-based channel number.
		* @param number the 1-based number of the DMX512 channel, ranging from 1 to the return value of @getChannelCount
		* @return the 8-bit value of a single DMX512 channel in the ArtDmx packet
		*/
		uint8_t getChannelByNumber(uint16_t number) const { return mData.at(number - 1); };

		/**
		* Retrieve the 8-bit value of a single DMX512 channel by its 0-based channel index.
		* @param number the 0-based index of the DMX512 channel, ranging from 0 to the return value of @getChannelCount - 1
		* @return the 8-bit value of a single DMX512 channel in the ArtDmx packet
		*/
		uint8_t getChannelByIndex(uint16_t index) const { return mData.at(index); };

	private:

		uint8_t mSequence;
		uint8_t mPhysical;
		uint8_t mNet;
		uint8_t mSubNet;
		uint8_t mUniverse;
		uint16_t mPortAddress;
		std::vector<uint8_t> mData;
	};

	using ArtDmxPacketEventPtr = std::unique_ptr<nap::ArtDmxPacketEvent>;
}

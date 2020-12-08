/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "websocketticket.h"

// External Includes
#include <rtti/jsonreader.h>
#include <rtti/jsonwriter.h>
#include <rtti/rtticast.h>
#include <bitset>

// nap::websocketticket run time class definition 
RTTI_BEGIN_CLASS(nap::WebSocketTicket)
	RTTI_PROPERTY("UserName", &nap::WebSocketTicket::mUsername, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Password", &nap::WebSocketTicket::mPassword, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	bool WebSocketTicket::init(utility::ErrorState& errorState)
	{
		if (errorState.check(mUsername.empty(), "%s: no username specified", mID.c_str()))
			return false;

		if (errorState.check(mPassword.empty(), "%s: no password specified", mID.c_str()))
			return false;

		return true;
	}


	bool WebSocketTicket::toBinaryString(std::string& outString, utility::ErrorState& error)
	{
		// Serialize to binary
		rtti::JSONWriter writer;
		if (!rtti::serializeObjects({ this }, writer, error))
			return false;

		// Convert to bitwise string
		std::string json(writer.GetJSON());
		for (auto character : json)
			outString += std::bitset<8>(character).to_string();
		return true;
	}


	WebSocketTicket* WebSocketTicket::fromBinaryString(const std::string& binaryString, rtti::DeserializeResult& result, utility::ErrorState& error)
	{
		// Make sure it's a binary stream
		if (!error.check(!binaryString.empty() && binaryString.size() % 8 == 0, "invalid binary bit-stream"))
			return nullptr;

		// Convert entire bitset into byte array
		std::vector<uint8_t> vec;
		try 
		{
			vec.reserve(binaryString.size() / 8);
			for (int i = 0; i < binaryString.size(); i += 8)
			{
				vec.emplace_back(std::bitset<8>(binaryString.substr(i, i + 8)).to_ulong());
			}
		}
		catch (std::exception& e)
		{
			error.fail("invalid binary bit-stream: %s", e.what());
			return nullptr;
		}

		// De-serialize binary ticket
		rtti::Factory factory;
		std::string json(vec.begin(), vec.end());

		// De-serialize
		if (!rtti::deserializeJSON(json, rtti::EPropertyValidationMode::AllowMissingProperties, rtti::EPointerPropertyMode::OnlyRawPointers, factory, result, error))
			return nullptr;

		// Ensure we have at least 1 object
		if (error.check(result.mReadObjects.empty(), "no ticket in object list"))
			return nullptr;

		// First object should be the ticket
		if (!error.check(result.mReadObjects[0]->get_type() == RTTI_OF(WebSocketTicket),
			"extracted object not a: ", RTTI_OF(WebSocketTicket).get_name().to_string().c_str()))
			return nullptr;

		// Now get the first item as a ticket
		WebSocketTicket* ticket = rtti_cast<WebSocketTicket>(result.mReadObjects[0].get());
		if (!error.check(ticket != nullptr, 
			"extracted object not a: ", RTTI_OF(WebSocketTicket).get_name().to_string().c_str()))
			return nullptr;

		return ticket;
	}


	WebSocketTicketHash WebSocketTicket::toHash() const
	{
		return WebSocketTicketHash(*this);
	}


	WebSocketTicketHash::WebSocketTicketHash(const WebSocketTicket& ticket) :
		mHash(ticket.mUsername + ticket.mPassword)
	{

	}
}
#include "websocketticket.h"
#include <rtti/binarywriter.h>
#include <rtti/binaryreader.h>
#include <utility/memorystream.h>
#include <rtti/deserializeresult.h>
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
		return true;
	}


	bool WebSocketTicket::toBinaryString(std::string& outString, utility::ErrorState& error)
	{
		// Serialize to binary
		rtti::BinaryWriter binaryWriter;
		if (!rtti::serializeObjects({ this }, binaryWriter, error))
			return false;

		// Convert to bitwise string
		std::string binary_string(binaryWriter.getBuffer().begin(), binaryWriter.getBuffer().end());

		for (auto character : binary_string)
			outString += std::bitset<8>(character).to_string();
		return true;
	}


	bool WebSocketTicket::fromBinaryString(const std::string& binaryString, utility::ErrorState& error)
	{
		// Convert entire bitset into byte array
		std::vector<uint8_t> vec;
		vec.reserve(binaryString.size() / 8);
		for (int i = 0; i < binaryString.size(); i += 8)
			vec.emplace_back(std::bitset<8>(binaryString.substr(i, i + 8)).to_ulong());

		// De-serialize binary ticket
		rtti::Factory factory;
		rtti::DeserializeResult deserialize_result;
		utility::MemoryStream stream(vec.data(), vec.size());

		// De-serialize
		if (!rtti::deserializeBinary(stream, factory, deserialize_result, error))
			return false;

		// Ensure we have at least 1 object
		if (error.check(deserialize_result.mReadObjects.empty(), "no ticket in object list"))
			return false;

		// First object should be the ticket
		if (!error.check(deserialize_result.mReadObjects[0]->get_type() == RTTI_OF(WebSocketTicket),
			"extracted object not a: ", RTTI_OF(WebSocketTicket).get_name().to_string().c_str()))
			return false;

		// Now get the first item as a ticket
		WebSocketTicket* ticket = rtti_cast<WebSocketTicket>(deserialize_result.mReadObjects[0].get());
		if (!error.check(ticket != nullptr, 
			"extracted object not a: ", RTTI_OF(WebSocketTicket).get_name().to_string().c_str()))
			return false;

		// Copy object
		rtti::copyObject(*ticket, *this);

		// Done
		return true;
	}
}
#include "websocketticket.h"
#include <rtti/binarywriter.h>
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
}
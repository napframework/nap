#pragma once

// Local Includes
#include "apimessage.h"

// External Includes
#include <rtti/deserializeresult.h>
#include <rtti/factory.h>

namespace nap
{
	/**
	 * Extracts APIMessage objects from a json string.
	 * @param json the json string to extract messages from
	 * @param result contains the de-serialized result, owner of the messages
	 * @param factory contains information on how to construct a message
	 * @param outMessages list of messages extracted from json, can be empty
	 * @param error contains the error if extraction fails
	 * @return if extraction succeeded or not.
	 */
	NAPAPI bool extractMessages(const std::string& json, rtti::DeserializeResult& result, rtti::Factory& factory, std::vector<APIMessage*>& outMessages, utility::ErrorState& error);
}

// Local Includes
#include "apiservice.h"
#include "apievent.h"
#include "apicomponent.h"

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/error/en.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::APIService)
	RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

namespace nap
{

	APIService::APIService(ServiceConfiguration* configuration) : Service(configuration)
	{
	}


	APIService::~APIService()
	{
	}


	bool APIService::sendFloat(const char* id, float value, utility::ErrorState* error)
	{
		APICallEventPtr ptr = std::make_unique<APICallEvent>(id);
		ptr->addArgument<APIFloat>(value);
		return forward(std::move(ptr), *error);
	}


	bool APIService::sendString(const char* id, const char* value, utility::ErrorState* error)
	{
		APICallEventPtr ptr = std::make_unique<APICallEvent>(id);
		ptr->addArgument<APIString>(value);
		return forward(std::move(ptr), *error);
	}


	bool APIService::sendInt(const char* id, int value, utility::ErrorState* error)
	{
		APICallEventPtr ptr = std::make_unique<APICallEvent>(id);
		ptr->addArgument<APIInt>(value);
		return forward(std::move(ptr), *error);
	}


	bool APIService::sendByte(const char* id, nap::uint8 value, utility::ErrorState* error)
	{
		APICallEventPtr ptr = std::make_unique<APICallEvent>(id);
		ptr->addArgument<APIByte>(value);
		return forward(std::move(ptr), *error);
	}


	bool APIService::sendBool(const char* id, bool value, utility::ErrorState* error)
	{
		APICallEventPtr ptr = std::make_unique<APICallEvent>(id);
		ptr->addArgument<APIBool>(value);
		return forward(std::move(ptr), *error);
	}


	bool APIService::sendLong(const char* id, long long value, utility::ErrorState* error)
	{
		APICallEventPtr ptr = std::make_unique<APICallEvent>(id);
		ptr->addArgument<APILong>(value);
		return forward(std::move(ptr), *error);
	}


	bool APIService::sendChar(const char* id, char value, utility::ErrorState* error)
	{
		APICallEventPtr ptr = std::make_unique<APICallEvent>(id);
		ptr->addArgument<APIChar>(value);
		return forward(std::move(ptr), *error);
	}


	bool APIService::send(const char* id, utility::ErrorState* error)
	{
		APICallEventPtr ptr = std::make_unique<APICallEvent>(id);
		return forward(std::move(ptr), *error);
	}


	static int getLine(const std::string& json, size_t offset)
	{
		int line = 1;
		size_t line_offset = 0;
		while (true)
		{
			line_offset = json.find('\n', line_offset);
			if (line_offset == std::string::npos || line_offset > offset)
				break;
			++line;
			line_offset += 1;
		}
		return line;
	}


	bool APIService::sendJSON(const char* json, utility::ErrorState* error)
	{
		std::string json_str(json);

		// Try to parse the json file
		rapidjson::Document document;
		rapidjson::ParseResult parse_result = document.Parse(json_str.c_str());
		if (!parse_result)
		{
			error->fail("Error parsing json: %s (line: %d)", rapidjson::GetParseError_En(parse_result.Code()), getLine(json_str, parse_result.Offset()));
			return false;
		}

		// Read objects
		rapidjson::Value::ConstMemberIterator call = document.FindMember("Call");

		if (!error->check(call != document.MemberEnd(), "Unable to find required 'Call' field"))
			return false;

		rapidjson::Value::ConstMemberIterator id = call->value.FindMember("mID");
		if (!error->check(id != document.MemberEnd(), "Unable to find required 'mID' field"))
			return false;

		rapidjson::Value::ConstMemberIterator signature = call->value.FindMember("Signature");
		if (!error->check(signature != document.MemberEnd(), "Unable to find required 'Signature' field"))
			return false;

		// Read properties of signature

		// TODO: parse JSON, create argument list and send!
		return true;
	}


	bool APIService::sendIntArray(const char* id, int* array, int length, utility::ErrorState* error)
	{
		// Copy data into new array
		std::vector<int> values(length);
		std::memcpy(values.data(), array, sizeof(int)*length);

		// Move construct argument and send.
		APICallEventPtr ptr = std::make_unique<APICallEvent>(id);
		ptr->addArgument<APIIntArray>(std::move(values));
		return forward(std::move(ptr), *error);
	}


	bool APIService::sendFloatArray(const char* id, float* array, int length, utility::ErrorState* error)
	{
		// Copy data into new array
		std::vector<float> values(length);
		std::memcpy(values.data(), array, sizeof(float)*length);

		// Move construct argument and send.
		APICallEventPtr ptr = std::make_unique<APICallEvent>(id);
		ptr->addArgument<APIFloatArray>(std::move(values));
		return forward(std::move(ptr), *error);
	}


	bool APIService::sendByteArray(const char* id, uint8_t* array, int length, utility::ErrorState* error)
	{
		// Copy data into new array
		std::vector<uint8_t> values(length);
		std::memcpy(values.data(), array, sizeof(uint8_t)*length);

		// Move construct argument and send.
		APICallEventPtr ptr = std::make_unique<APICallEvent>(id);
		ptr->addArgument<APIByteArray>(std::move(values));
		return forward(std::move(ptr), *error);
	}


	bool APIService::sendStringArray(const char* id, const char** array, int length, utility::ErrorState* error)
	{
		// Create array of strings out of raw data
		std::vector<std::string> values;
		values.reserve(length);
		for(int i=0; i<length; i++)
			values.emplace_back(std::move(std::string(array[i])));

		// Move construct argument and send.
		APICallEventPtr ptr = std::make_unique<APICallEvent>(id);
		ptr->addArgument<APIStringArray>(std::move(values));
		return forward(std::move(ptr), *error);
	}

	
	void APIService::registerAPIComponent(APIComponentInstance& apicomponent)
	{
		mAPIComponents.emplace_back(&apicomponent);
	}


	void APIService::removeAPIComponent(APIComponentInstance& apicomponent)
	{
		auto found_it = std::find_if(mAPIComponents.begin(), mAPIComponents.end(), [&](const auto& it)
		{
			return it == &apicomponent;
		});
		assert(found_it != mAPIComponents.end());
		mAPIComponents.erase(found_it);
	}


	bool APIService::forward(APICallEventPtr apiEvent, utility::ErrorState& error)
	{
		for (auto& api_comp : mAPIComponents)
		{
			if (api_comp->accepts(apiEvent->getID()))
			{
				return api_comp->call(std::move(apiEvent), error);
			}
		}

		// No component accepted the call!
		return error.check(false, "%s: No matching call found for: %s", this->get_type().get_name().data(), apiEvent->getID().c_str());
	}


	bool APIService::init(utility::ErrorState& error)
	{
		return true;
	}


	void APIService::shutdown()
	{

	}
}
// Local Includes
#include "apiservice.h"
#include "apievent.h"
#include "apicomponent.h"

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


	bool APIService::sendFloat(const char* action, float value, utility::ErrorState* error)
	{
		APICallEventPtr ptr = std::make_unique<APICallEvent>(action);
		ptr->addArgument<APIFloat>(value);
		return forward(std::move(ptr), *error);
	}


	bool APIService::sendString(const char* action, const char* value, utility::ErrorState* error)
	{
		APICallEventPtr ptr = std::make_unique<APICallEvent>(action);
		ptr->addArgument<APIString>(value);
		return forward(std::move(ptr), *error);
	}


	bool APIService::sendInt(const char* action, int value, utility::ErrorState* error)
	{
		APICallEventPtr ptr = std::make_unique<APICallEvent>(action);
		ptr->addArgument<APIInt>(value);
		return forward(std::move(ptr), *error);
	}


	bool APIService::sendByte(const char* action, nap::uint8 value, utility::ErrorState* error)
	{
		APICallEventPtr ptr = std::make_unique<APICallEvent>(action);
		ptr->addArgument<APIByte>(value);
		return forward(std::move(ptr), *error);
	}


	bool APIService::sendBool(const char* action, bool value, utility::ErrorState* error)
	{
		APICallEventPtr ptr = std::make_unique<APICallEvent>(action);
		ptr->addArgument<APIBool>(value);
		return forward(std::move(ptr), *error);
	}


	bool APIService::sendLong(const char* action, long long value, utility::ErrorState* error)
	{
		APICallEventPtr ptr = std::make_unique<APICallEvent>(action);
		ptr->addArgument<APILong>(value);
		return forward(std::move(ptr), *error);
	}


	bool APIService::sendChar(const char* action, char value, utility::ErrorState* error)
	{
		APICallEventPtr ptr = std::make_unique<APICallEvent>(action);
		ptr->addArgument<APIChar>(value);
		return forward(std::move(ptr), *error);
	}


	bool APIService::send(const char* action, utility::ErrorState* error)
	{
		APICallEventPtr ptr = std::make_unique<APICallEvent>(action);
		return forward(std::move(ptr), *error);
	}


	bool APIService::sendJSON(const char* action, const char** json, utility::ErrorState* error)
	{
		// TODO: parse JSON, create argument list and send!
		return true;
	}


	bool APIService::sendIntArray(const char* action, int* array, int length, utility::ErrorState* error)
	{
		// Copy data into new array
		std::vector<int> values(length);
		std::memcpy(values.data(), array, sizeof(int)*length);

		// Move construct argument and send.
		APICallEventPtr ptr = std::make_unique<APICallEvent>(action);
		ptr->addArgument<APIIntArray>(std::move(values));
		return forward(std::move(ptr), *error);
	}


	bool APIService::sendFloatArray(const char* action, float* array, int length, utility::ErrorState* error)
	{
		// Copy data into new array
		std::vector<float> values(length);
		std::memcpy(values.data(), array, sizeof(float)*length);

		// Move construct argument and send.
		APICallEventPtr ptr = std::make_unique<APICallEvent>(action);
		ptr->addArgument<APIFloatArray>(std::move(values));
		return forward(std::move(ptr), *error);
	}


	bool APIService::sendByteArray(const char* action, uint8_t* array, int length, utility::ErrorState* error)
	{
		// Copy data into new array
		std::vector<uint8_t> values(length);
		std::memcpy(values.data(), array, sizeof(uint8_t)*length);

		// Move construct argument and send.
		APICallEventPtr ptr = std::make_unique<APICallEvent>(action);
		ptr->addArgument<APIByteArray>(std::move(values));
		return forward(std::move(ptr), *error);
	}


	bool APIService::sendStringArray(const char* action, const char** array, int length, utility::ErrorState* error)
	{
		// Create array of strings out of raw data
		std::vector<std::string> values;
		values.reserve(length);
		for(int i=0; i<length; i++)
			values.emplace_back(std::move(std::string(array[i])));

		// Move construct argument and send.
		APICallEventPtr ptr = std::make_unique<APICallEvent>(action);
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
			if (api_comp->accepts(apiEvent->getAction()))
			{
				return api_comp->call(std::move(apiEvent), error);
			}
		}

		// No component accepted the call!
		return error.check(false, "%s: No matching call found for: %s", this->get_type().get_name().data(), apiEvent->getAction().c_str());
	}


	bool APIService::init(utility::ErrorState& error)
	{
		return true;
	}


	void APIService::shutdown()
	{

	}
}
// Local Includes
#include "apiservice.h"
#include "apievent.h"
#include "apicomponent.h"

// External Includes
#include <rtti/jsonreader.h>
#include <rtti/defaultlinkresolver.h>
#include <nap/core.h>

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


	bool APIService::sendJSON(const char* json, utility::ErrorState* error)
	{
		// Get factory
		nap::rtti::DeserializeResult result;
		auto& factory = getCore().getResourceManager()->getFactory();

		// De-serialize json cmd
		if (!rtti::deserializeJSON(json, rtti::EPropertyValidationMode::DisallowMissingProperties, factory, result, *error))
			return false;

		// Resolve links
		if (!rtti::DefaultLinkResolver::sResolveLinks(result.mReadObjects, result.mUnresolvedPointers, *error))
			return false;

		// Fetch all api signatures
		std::vector<nap::APISignature*> signatures;
		for (const auto& object : result.mReadObjects)
		{
			// Check if it's a call signature
			if (!object->get_type().is_derived_from(RTTI_OF(nap::APISignature)))
				continue;

			// Cast to signature and add as possible event
			nap::APISignature* signature = rtti_cast<nap::APISignature>(object.get());
			assert(signature != nullptr);
			if (signature != nullptr)
				signatures.emplace_back(signature);
		}

		// Error when json doesn't contain any signature objects
		if (!error->check(!signatures.empty(), "JSON doesn't contain any nap::Signature objects"))
			return false;

		// Iterate over every signature, extract arguments (APIFloat etc.) and send as new event.
		// Note that we make a copy of the argument because the original arguments are owned by the de-serialized result
		bool succeeded = true;
		for (auto& signature : signatures)
		{
			APICallEventPtr ptr = std::make_unique<APICallEvent>(signature->mID);
			for (auto& arg : signature->mArguments)
			{
				// Create copy using RTTR
				rtti::Variant arg_copy = arg->get_type().create();
				assert(arg_copy.is_valid());
				
				// Wrap result in unique ptr
				std::unique_ptr<APIBaseValue> copy(arg_copy.get_value<APIBaseValue*>());

				// Copy all properties as we know they contain the same ones
				rtti::copyObject(*arg, *copy);

				// Add API value as argument
				ptr->addArgument(std::move(copy));
			}

			// Forward, keep track of result
			if (!forward(std::move(ptr), *error))
				succeeded = false;
		}

		// Notify user if all signatures were send successfully.
		if (!error->check(succeeded, "Unable to forward all JSON requests"))
			return false;
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
		return error.check(false, "%s: No matching signature found for: %s", this->get_type().get_name().data(), apiEvent->getID().c_str());
	}


	bool APIService::init(utility::ErrorState& error)
	{
		return true;
	}


	void APIService::shutdown()
	{

	}
}
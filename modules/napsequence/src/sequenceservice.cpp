// External Includes
#include <nap/core.h>
#include <nap/resourcemanager.h>
#include <nap/logger.h>
#include <iostream>
#include <utility/stringutils.h>

// Local Includes
#include "sequenceservice.h"
#include "sequenceeventreceiver.h"
#include "sequenceplayeradapter.h"
#include "sequenceplayercurveadapter.h"
#include "sequenceplayerinput.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SequenceService)
RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

namespace nap
{
	static std::vector<std::unique_ptr<rtti::IObjectCreator>(*)(SequenceService*)>& getObjectCreators()
	{
		static std::vector<std::unique_ptr<rtti::IObjectCreator>(*)(SequenceService* service)> vector;
		return vector;
	}

	bool SequenceService::registerObjectCreator(std::unique_ptr<rtti::IObjectCreator>(*objectCreator)(SequenceService* service))
	{
		getObjectCreators().emplace_back(objectCreator);
		return true;
	}

	SequenceService::SequenceService(ServiceConfiguration* configuration) :
		Service(configuration)
	{
	}

	SequenceService::~SequenceService()
	{ }

	void SequenceService::registerObjectCreators(rtti::Factory& factory)
	{
		for(auto& objectCreator : getObjectCreators())
		{
			factory.addObjectCreator(objectCreator(this));
		}
	}

	bool SequenceService::init(nap::utility::ErrorState& errorState)
	{
		return true;
	}

	void SequenceService::update(double deltaTime)
	{
		for(auto& input : mInputs)
		{
			input->update(deltaTime);
		}
	}


	void SequenceService::registerInput(SequencePlayerInput& input)
	{
		auto found_it = std::find_if(mInputs.begin(), mInputs.end(), [&](const auto& it)
		{
		  return it == &input;
		});
		assert(found_it == mInputs.end()); // duplicate entry

		if(found_it == mInputs.end())
		{
			mInputs.emplace_back(&input);
		}
	}


	void SequenceService::removeInput(SequencePlayerInput& input)
	{
		auto found_it = std::find_if(mInputs.begin(), mInputs.end(), [&](const auto& it)
		{
		  return it == &input;
		});
		//assert(found_it != mInputs.end()); // not found

		if(found_it != mInputs.end())
		{
			mInputs.erase(found_it);
		}
	}
}
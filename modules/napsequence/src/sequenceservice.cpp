/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// External Includes
#include <nap/core.h>
#include <nap/resourcemanager.h>
#include <nap/logger.h>
#include <iostream>
#include <utility/stringutils.h>

// Local Includes
#include "sequenceplayeradapter.h"
#include "sequenceplayeroutput.h"
#include "sequenceservice.h"
#include "sequenceeditor.h"

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
		for(auto& output : mOutputs)
		{
			output->update(deltaTime);
		}

		for(auto& editor : mEditors)
		{
			editor->update(deltaTime);
		}
	}


	void SequenceService::registerOutput(SequencePlayerOutput& input)
	{
		auto found_it = std::find_if(mOutputs.begin(), mOutputs.end(), [&](const auto& it)
		{
		  return it == &input;
		});
		assert(found_it == mOutputs.end()); // duplicate entry

		if(found_it == mOutputs.end())
		{
			mOutputs.emplace_back(&input);
		}
	}


	void SequenceService::removeOutput(SequencePlayerOutput& input)
	{
		auto found_it = std::find_if(mOutputs.begin(), mOutputs.end(), [&](const auto& it)
		{
		  return it == &input;
		});

		if(found_it != mOutputs.end())
		{
			mOutputs.erase(found_it);
		}
	}


	void SequenceService::registerEditor(SequenceEditor& input)
	{
		auto found_it = std::find_if(mEditors.begin(), mEditors.end(), [&](const auto& it)
		{
		  return it == &input;
		});
		assert(found_it == mEditors.end()); // duplicate entry

		if(found_it == mEditors.end())
		{
			mEditors.emplace_back(&input);
		}
	}


	void SequenceService::removeEditor(SequenceEditor& input)
	{
		auto found_it = std::find_if(mEditors.begin(), mEditors.end(), [&](const auto& it)
		{
		  return it == &input;
		});

		if(found_it != mEditors.end())
		{
			mEditors.erase(found_it);
		}
	}
}
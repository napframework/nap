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
#include "sequenceguiservice.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SequenceGuiService)
		RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

namespace nap
{
	static std::vector<std::unique_ptr<rtti::IObjectCreator>(*)(SequenceGuiService*)>& getObjectCreators()
	{
		static std::vector<std::unique_ptr<rtti::IObjectCreator>(*)(SequenceGuiService* service)> vector;
		return vector;
	}


	bool SequenceGuiService::registerObjectCreator(std::unique_ptr<rtti::IObjectCreator>(*objectCreator)(SequenceGuiService* service))
	{
		getObjectCreators().emplace_back(objectCreator);
		return true;
	}


	SequenceGuiService::SequenceGuiService(ServiceConfiguration* configuration) :
		Service(configuration)
	{
	}


	SequenceGuiService::~SequenceGuiService() = default;


	void SequenceGuiService::registerObjectCreators(rtti::Factory& factory)
	{
		for(auto& objectCreator : getObjectCreators())
		{
			factory.addObjectCreator(objectCreator(this));
		}
	}


	bool SequenceGuiService::init(nap::utility::ErrorState& errorState)
	{
		return true;
	}


	void SequenceGuiService::update(double deltaTime)
	{
	}
}
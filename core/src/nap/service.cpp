/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "python.h"
#include "logger.h"
#include "service.h"
#include "core.h"

// External Includes
#include <utility/stringutils.h>

RTTI_DEFINE_BASE(nap::ServiceConfiguration)

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Service)
	RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

namespace nap
{
	Service::Service(ServiceConfiguration* configuration) :
		mConfiguration(configuration)
	{
	}

	/**
	@brief Destructor
	**/
	Service::~Service()
	{}


	/**
	@brief Returns the core in which service lives
	**/
	Core& Service::getCore()
	{
		assert(mCore);
		return *mCore;
	}


	const nap::Core& Service::getCore() const
	{
		assert(mCore);
		return *mCore;
	}


	std::string Service::getTypeName() const
	{
		return rtti::TypeInfo::get(*this).get_name().data();
	}


	std::string Service::getIniFilePath() const
	{
		return utility::stringFormat("%s/%s/%s",
			getCore().getProjectInfo()->getProjectDir().c_str(),
			iniDirectory, getIniFileName().c_str()
		);
	}


	std::string Service::getIniFileName() const
	{
		return utility::stringFormat("%s%s",
			utility::toLower(utility::stripNamespace(getTypeName())).c_str(),
			iniExtension
		);
	}
}

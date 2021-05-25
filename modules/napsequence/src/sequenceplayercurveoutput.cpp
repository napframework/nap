/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "sequenceplayercurveoutput.h"
#include "sequenceservice.h"
#include "sequenceplayercurveadapter.h"

#include <nap/logger.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SequencePlayerCurveOutput)
RTTI_PROPERTY("Parameter", &nap::SequencePlayerCurveOutput::mParameter, nap::rtti::EPropertyMetaData::Required)
RTTI_PROPERTY("Use Main Thread", &nap::SequencePlayerCurveOutput::mUseMainThread, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
	SequencePlayerCurveOutput::SequencePlayerCurveOutput(SequenceService& service)
		: SequencePlayerOutput(service)
	{
	}


	void SequencePlayerCurveOutput::update(double deltaTime)
	{
		for(auto* curve_adapter : mAdapters)
		{
			curve_adapter->setValue();
		}
	}


	void SequencePlayerCurveOutput::registerAdapter(SequencePlayerCurveAdapterBase* curveAdapter)
	{
		auto found_it = std::find_if(mAdapters.begin(), mAdapters.end(), [&](const auto& it)
		{
		  return it == curveAdapter;
		});
		assert(found_it == mAdapters.end()); // duplicate entry

		if(found_it == mAdapters.end())
		{
			mAdapters.emplace_back(curveAdapter);
		}
	}


	void SequencePlayerCurveOutput::removeAdapter(SequencePlayerCurveAdapterBase* curveAdapter)
	{
		auto found_it = std::find_if(mAdapters.begin(), mAdapters.end(), [&](const auto& it)
		{
			return it == curveAdapter;
		});

		if(found_it != mAdapters.end())
		{
			mAdapters.erase(found_it);
		}
	}
}
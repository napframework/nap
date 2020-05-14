#include "sequenceplayercurveoutput.h"
#include "sequenceservice.h"
#include "sequenceplayercurveadapter.h"

#include <nap/logger.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SequencePlayerCurveOutput)
RTTI_PROPERTY("Parameter", &nap::SequencePlayerCurveOutput::mParameter, nap::rtti::EPropertyMetaData::Default)
RTTI_PROPERTY("Use Main Thread", &nap::SequencePlayerCurveOutput::mUseMainThread, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
	static bool registerObjectCreator = SequenceService::registerObjectCreator([](SequenceService* service)->std::unique_ptr<rtti::IObjectCreator>
	{
		return std::make_unique<SequencePlayerCurveInputObjectCreator>(*service);
	});


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
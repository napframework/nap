#include "sequenceplayercurveoutput.h"
#include "sequenceservice.h"
#include "sequenceplayercurveadapter.h"
#include "sequenceutils.h"

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


	static bool registerDefaultTrackCreator = sequenceutils::registerDefaultTrackCreator(
		RTTI_OF(SequencePlayerCurveOutput), [](const SequencePlayerOutput* output) -> std::unique_ptr<SequenceTrack> {
			assert(RTTI_OF(SequencePlayerCurveOutput) == output->get_type()); // type mismatch

			// cast the output to a curve output
			const SequencePlayerCurveOutput* curve_output = static_cast<const SequencePlayerCurveOutput*>(output);

			// check the parameter
			// if its not null, check what type of parameter it is
			if (curve_output->mParameter != nullptr)
			{
				// vec2? create vec2 curve, else vec3
				if (curve_output->mParameter.get()->get_type() == RTTI_OF(ParameterVec2))
				{
					return std::make_unique<SequenceTrackCurveVec2>();
				}
				else if (curve_output->mParameter.get()->get_type() == RTTI_OF(ParameterVec3))
				{
					return std::make_unique<SequenceTrackCurveVec3>();
				}
			}

			// by default, always make a curve float track
			return std::make_unique<SequenceTrackCurveFloat>();
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
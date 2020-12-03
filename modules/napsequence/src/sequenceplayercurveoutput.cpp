/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "sequenceplayercurveoutput.h"
#include "sequenceservice.h"
#include "sequenceplayercurveadapter.h"
#include "sequenceutils.h"

#include <nap/logger.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SequencePlayerCurveOutput)
RTTI_PROPERTY("Parameter", &nap::SequencePlayerCurveOutput::mParameter, nap::rtti::EPropertyMetaData::Required)
RTTI_PROPERTY("Use Main Thread", &nap::SequencePlayerCurveOutput::mUseMainThread, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
	static bool register_object_creator = SequenceService::registerObjectCreator([](SequenceService* service)->std::unique_ptr<rtti::IObjectCreator>
	{
		return std::make_unique<SequencePlayerCurveInputObjectCreator>(*service);
	});


	static bool register_default_track_creator = sequenceutils::registerDefaultTrackCreator(
		RTTI_OF(SequencePlayerCurveOutput), [](const SequencePlayerOutput* output) -> std::unique_ptr<SequenceTrack> {
			assert(RTTI_OF(SequencePlayerCurveOutput) == output->get_type()); // type mismatch

			// cast the output to a curve output
			const SequencePlayerCurveOutput* curve_output = static_cast<const SequencePlayerCurveOutput*>(output);

			// check the parameter
			assert(curve_output->mParameter != nullptr); // parameter must be assigned

			// declare return ptr
			std::unique_ptr<SequenceTrack> sequence_track = nullptr;

		  	// check what type of parameter is being used and create a track that fits the parameter
			// ParameterVec2 = SequenceTrackCurveVec2
			// ParameterVec3 = SequenceTrackCurveVec3
			// ParameterFloat, ParameterLong, ParameterInt & ParameterDouble = SequenceTrackCurveFloat
			if (curve_output->mParameter.get()->get_type() == RTTI_OF(ParameterVec2))
			{
				sequence_track = std::make_unique<SequenceTrackCurveVec2>();
			}
			else if (curve_output->mParameter.get()->get_type() == RTTI_OF(ParameterVec3))
			{
				sequence_track = std::make_unique<SequenceTrackCurveVec3>();
			}else if( 	curve_output->mParameter.get()->get_type() == RTTI_OF(ParameterFloat) ||
						curve_output->mParameter.get()->get_type() == RTTI_OF(ParameterLong) ||
						curve_output->mParameter.get()->get_type() == RTTI_OF(ParameterInt) ||
						curve_output->mParameter.get()->get_type() == RTTI_OF(ParameterDouble) )
			{
				sequence_track = std::make_unique<SequenceTrackCurveFloat>();
			}

			assert(sequence_track != nullptr); // couldn't create default track with parameter type
			return sequence_track;
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
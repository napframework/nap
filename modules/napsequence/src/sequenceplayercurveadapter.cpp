/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "sequenceplayercurveadapter.h"
#include "sequenceplayercurveoutput.h"
#include "sequencetrackcurve.h"

namespace nap
{
	static bool sRegisterCurveFloatAdapter = SequencePlayerAdapter::registerFactory(RTTI_OF(SequenceTrackCurveFloat), [](const SequenceTrack& track, SequencePlayerOutput& output, const SequencePlayer& player)->std::unique_ptr<SequencePlayerAdapter>
	{
		assert(track.get_type() == RTTI_OF(SequenceTrackCurveFloat)); // type mismatch
		assert(output.get_type() == RTTI_OF(SequencePlayerCurveOutput)); //  type mismatch

		auto& curve_output = static_cast<SequencePlayerCurveOutput&>(output);

		if (curve_output.mParameter.get()->get_type() == RTTI_OF(ParameterFloat))
		{
            return std::make_unique<SequencePlayerCurveAdapter<float, ParameterFloat, float>>(track, curve_output);
		}

		if (curve_output.mParameter.get()->get_type() == RTTI_OF(ParameterLong))
		{
            return std::make_unique<SequencePlayerCurveAdapter<float, ParameterLong, long>>(track, curve_output);
		}

		if (curve_output.mParameter.get()->get_type() == RTTI_OF(ParameterDouble))
		{
            return std::make_unique<SequencePlayerCurveAdapter<float, ParameterDouble, double>>(track, curve_output);
		}

		if (curve_output.mParameter.get()->get_type() == RTTI_OF(ParameterInt))
		{
            return std::make_unique<SequencePlayerCurveAdapter<float, ParameterInt, int>>(track, curve_output);
		}

		assert(false); // no correct parameter type found!
		return nullptr;
	});


	static bool sRegisterCurveVec2Adapter = SequencePlayerAdapter::registerFactory(RTTI_OF(SequenceTrackCurveVec2), [](const SequenceTrack& track, SequencePlayerOutput& output, const SequencePlayer& player)->std::unique_ptr<SequencePlayerAdapter>
	{
		assert(track.get_type() == RTTI_OF(SequenceTrackCurveVec2)); // type mismatch
		assert(output.get_type() == RTTI_OF(SequencePlayerCurveOutput)); //  type mismatch

		auto& curve_output = static_cast<SequencePlayerCurveOutput&>(output);

		assert(curve_output.mParameter.get()->get_type() == RTTI_OF(ParameterVec2)); // type mismatch
		if (curve_output.mParameter.get()->get_type() == RTTI_OF(ParameterVec2))
		{
            return std::make_unique<SequencePlayerCurveAdapter<glm::vec2, ParameterVec2, glm::vec2>>(track, curve_output);
		}

		return nullptr;
	});


	static bool sRegisterCurveVec3Adapter = SequencePlayerAdapter::registerFactory(RTTI_OF(SequenceTrackCurveVec3), [](const SequenceTrack& track, SequencePlayerOutput& output, const SequencePlayer& player)->std::unique_ptr<SequencePlayerAdapter>
	{
		assert(track.get_type() == RTTI_OF(SequenceTrackCurveVec3)); // type mismatch
		assert(output.get_type() == RTTI_OF(SequencePlayerCurveOutput)); //  type mismatch

		auto& curve_output = static_cast<SequencePlayerCurveOutput&>(output);

		assert(curve_output.mParameter.get()->get_type() == RTTI_OF(ParameterVec3)); // type mismatch
		if (curve_output.mParameter.get()->get_type() == RTTI_OF(ParameterVec3))
		{
			return std::make_unique<SequencePlayerCurveAdapter<glm::vec3, ParameterVec3, glm::vec3>>(track, curve_output);
		}

		return nullptr;
	});


	static bool sRegisterCurveVec4Adapter = SequencePlayerAdapter::registerFactory(RTTI_OF(SequenceTrackCurveVec4), [](const SequenceTrack& track, SequencePlayerOutput& output, const SequencePlayer& player)->std::unique_ptr<SequencePlayerAdapter>
	{
		nap::Logger::info("adapter not yet implemented!");
		return nullptr;
	});
}

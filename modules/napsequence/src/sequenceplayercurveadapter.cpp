#include "sequenceplayercurveadapter.h"
#include "sequenceplayercurveoutput.h"
#include "sequencetrackcurve.h"

namespace nap
{
	static bool sRegisterCurveFloatAdapter = SequencePlayerAdapter::registerFactory(RTTI_OF(SequenceTrackCurveFloat), [](SequenceTrack& track, SequencePlayerOutput& output)->std::unique_ptr<SequencePlayerAdapter>
	{
		assert(track.get_type() == RTTI_OF(SequenceTrackCurveFloat)); // type mismatch
		assert(output.get_type() == RTTI_OF(SequencePlayerCurveOutput)); //  type mismatch

		auto& curveTrack = static_cast<SequenceTrackCurveFloat&>(track);
		auto& curveOutput = static_cast<SequencePlayerCurveOutput&>(output);

		if (curveOutput.mParameter.get()->get_type() == RTTI_OF(ParameterFloat))
		{
			return std::move(std::make_unique<SequencePlayerCurveAdapter<float, ParameterFloat, float>>(track, curveOutput));
		}

		if (curveOutput.mParameter.get()->get_type() == RTTI_OF(ParameterLong))
		{
			return std::move(std::make_unique<SequencePlayerCurveAdapter<float, ParameterLong, long>>(track, curveOutput));
		}

		if (curveOutput.mParameter.get()->get_type() == RTTI_OF(ParameterDouble))
		{
			return std::move(std::make_unique<SequencePlayerCurveAdapter<float, ParameterDouble, double>>(track, curveOutput));
		}

		if (curveOutput.mParameter.get()->get_type() == RTTI_OF(ParameterInt))
		{
			return std::move(std::make_unique<SequencePlayerCurveAdapter<float, ParameterInt, int>>(track, curveOutput));
		}

		assert(false); // no correct parameter type found!
		return nullptr;
	});


	static bool sRegisterCurveVec2Adapter = SequencePlayerAdapter::registerFactory(RTTI_OF(SequenceTrackCurveVec2), [](SequenceTrack& track, SequencePlayerOutput& output)->std::unique_ptr<SequencePlayerAdapter>
	{
		assert(track.get_type() == RTTI_OF(SequenceTrackCurveVec2)); // type mismatch
		assert(output.get_type() == RTTI_OF(SequencePlayerCurveOutput)); //  type mismatch

		auto& curveTrack = static_cast<SequenceTrackCurveVec2&>(track);
		auto& curveOutput = static_cast<SequencePlayerCurveOutput&>(output);

		assert(curveOutput.mParameter.get()->get_type() == RTTI_OF(ParameterVec2)); // type mismatch
		if (curveOutput.mParameter.get()->get_type() == RTTI_OF(ParameterVec2))
		{
			return std::move(std::make_unique<SequencePlayerCurveAdapter<glm::vec2, ParameterVec2, glm::vec2>>(track, curveOutput));
		}

		return nullptr;
	});


	static bool sRegisterCurveVec3Adapter = SequencePlayerAdapter::registerFactory(RTTI_OF(SequenceTrackCurveVec3), [](SequenceTrack& track, SequencePlayerOutput& output)->std::unique_ptr<SequencePlayerAdapter>
	{
		assert(track.get_type() == RTTI_OF(SequenceTrackCurveVec3)); // type mismatch
		assert(output.get_type() == RTTI_OF(SequencePlayerCurveOutput)); //  type mismatch

		auto& curveTrack = static_cast<SequenceTrackCurveVec3&>(track);
		auto& curveOutput = static_cast<SequencePlayerCurveOutput&>(output);

		assert(curveOutput.mParameter.get()->get_type() == RTTI_OF(ParameterVec3)); // type mismatch
		if (curveOutput.mParameter.get()->get_type() == RTTI_OF(ParameterVec3))
		{
			return std::move(std::make_unique<SequencePlayerCurveAdapter<glm::vec3, ParameterVec3, glm::vec3>>(track, curveOutput));
		}

		return nullptr;
	});


	static bool sRegisterCurveVec4Adapter = SequencePlayerAdapter::registerFactory(RTTI_OF(SequenceTrackCurveVec4), [](SequenceTrack& track, SequencePlayerOutput& input)->std::unique_ptr<SequencePlayerAdapter>
	{
		nap::Logger::info("adapter not yet implemented!");
		return nullptr;
	});
}
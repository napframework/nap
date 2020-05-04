#include "sequenceplayercurveadapter.h"
#include "sequencetrackcurve.h"

namespace nap
{
	static bool sRegisterCurveFloatAdapter = SequencePlayerAdapter::registerFactory(RTTI_OF(SequenceTrackCurveFloat), [](SequenceTrack& track, SequencePlayerInput& input)->std::unique_ptr<SequencePlayerAdapter>
	{
		assert(track.get_type() == RTTI_OF(SequenceTrackCurveFloat)); // type mismatch
		assert(input.get_type() == RTTI_OF(SequencePlayerCurveInput)); //  type mismatch

		auto& curveTrack = static_cast<SequenceTrackCurveFloat&>(track);
		auto& curveInput = static_cast<SequencePlayerCurveInput&>(input);

		if (curveInput.mParameter->get_type() == RTTI_OF(ParameterFloat))
		{
			return std::move(std::make_unique<SequencePlayerCurveAdapter<float, ParameterFloat, float>>(track, static_cast<ParameterFloat&>(*curveInput.mParameter.get()), *curveInput.mSequenceService, curveInput.mUseMainThread));
		}

		if (curveInput.mParameter->get_type() == RTTI_OF(ParameterLong))
		{
			return std::move(std::make_unique<SequencePlayerCurveAdapter<float, ParameterLong, long>>(track, static_cast<ParameterLong&>(*curveInput.mParameter.get()), *curveInput.mSequenceService, curveInput.mUseMainThread));
		}

		if (curveInput.mParameter->get_type() == RTTI_OF(ParameterDouble))
		{
			return std::move(std::make_unique<SequencePlayerCurveAdapter<float, ParameterDouble, double>>(track, static_cast<ParameterDouble&>(*curveInput.mParameter.get()), *curveInput.mSequenceService, curveInput.mUseMainThread));
		}

		if (curveInput.mParameter->get_type() == RTTI_OF(ParameterInt))
		{
			return std::move(std::make_unique<SequencePlayerCurveAdapter<float, ParameterInt, int>>(track, static_cast<ParameterInt&>(*curveInput.mParameter.get()), *curveInput.mSequenceService, curveInput.mUseMainThread));
		}

		assert(false); // no correct parameter type found!
		return nullptr;
	});


	static bool sRegisterCurveVec2Adapter = SequencePlayerAdapter::registerFactory(RTTI_OF(SequenceTrackCurveVec2), [](SequenceTrack& track, SequencePlayerInput& input)->std::unique_ptr<SequencePlayerAdapter>
	{
		assert(track.get_type() == RTTI_OF(SequenceTrackCurveVec2)); // type mismatch
		assert(input.get_type() == RTTI_OF(SequencePlayerCurveInput)); //  type mismatch

		auto& curveTrack = static_cast<SequenceTrackCurveVec2&>(track);
		auto& curveInput = static_cast<SequencePlayerCurveInput&>(input);

		assert(curveInput.mParameter->get_type() == RTTI_OF(ParameterVec2)); // type mismatch
		if (curveInput.mParameter->get_type() == RTTI_OF(ParameterVec2))
		{
			return std::move(std::make_unique<SequencePlayerCurveAdapter<glm::vec2, ParameterVec2, glm::vec2>>(track, static_cast<ParameterVec2&>(*curveInput.mParameter.get()), *curveInput.mSequenceService, curveInput.mUseMainThread));
		}

		return nullptr;
	});


	static bool sRegisterCurveVec3Adapter = SequencePlayerAdapter::registerFactory(RTTI_OF(SequenceTrackCurveVec3), [](SequenceTrack& track, SequencePlayerInput& input)->std::unique_ptr<SequencePlayerAdapter>
	{
		assert(track.get_type() == RTTI_OF(SequenceTrackCurveVec3)); // type mismatch
		assert(input.get_type() == RTTI_OF(SequencePlayerCurveInput)); //  type mismatch

		auto& curveTrack = static_cast<SequenceTrackCurveVec3&>(track);
		auto& curveInput = static_cast<SequencePlayerCurveInput&>(input);

		assert(curveInput.mParameter->get_type() == RTTI_OF(ParameterVec3)); // type mismatch
		if (curveInput.mParameter->get_type() == RTTI_OF(ParameterVec3))
		{
			return std::move(std::make_unique<SequencePlayerCurveAdapter<glm::vec3, ParameterVec3, glm::vec3>>(track, static_cast<ParameterVec3&>(*curveInput.mParameter.get()), *curveInput.mSequenceService, curveInput.mUseMainThread));
		}

		return nullptr;
	});


	static bool sRegisterCurveVec4Adapter = SequencePlayerAdapter::registerFactory(RTTI_OF(SequenceTrackCurveVec4), [](SequenceTrack& track, SequencePlayerInput& input)->std::unique_ptr<SequencePlayerAdapter>
	{
		nap::Logger::info("adapter not yet implemented!");
		return nullptr;
	});
}
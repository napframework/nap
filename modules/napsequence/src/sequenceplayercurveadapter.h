#pragma once

// local includes
#include "sequenceplayeradapter.h"
#include "sequenceplayer.h"
#include "sequenceplayercurveinput.h"

// nap includes
#include <nap/logger.h>
#include <parametervec.h>
#include <parameternumeric.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	template<typename CURVE_TYPE>
	class SequencePlayerCurveAdapterBase : public SequencePlayerAdapter
	{
	public:
	private:
		static bool	sFactoryRegistered;
	};

	/**
	 * Responsible for translating the value read on a curve track, to a parameter
	 * When the user wants to do this on the main thread, it uses a SequencePlayerParameterSetter as an intermediate class to ensure thread safety,
	 * otherwise it sets the parameter value directly from the sequence player thread
	 */
	template<typename CURVE_TYPE, typename PARAMETER_TYPE, typename PARAMETER_VALUE_TYPE>
	class SequencePlayerCurveAdapter : public SequencePlayerCurveAdapterBase<CURVE_TYPE>
	{
	public:
		/**
		 * Constructor
		 * @param track reference to sequence track that holds curve information
		 * @param parameter reference to parameter that is assigned to this track
		 * @param service reference to the sequence service, needed to sync with main thread
		 * @param useMain thread, whether to sync with the main thread or not
		 */
		SequencePlayerCurveAdapter(
			SequenceTrack& track,
			PARAMETER_TYPE& parameter,
			SequenceService &service,
			bool useMainThread)
			: mParameter(parameter),
			mUseMainThread(useMainThread),
			mService(service)
		{
			assert(track.get_type().is_derived_from(RTTI_OF(SequenceTrackCurve<CURVE_TYPE>)));
			mTrack = static_cast<SequenceTrackCurve<CURVE_TYPE>*>(&track);

			if (mUseMainThread)
			{
				mSetter = std::make_unique<SequencePlayerParameterSetter<PARAMETER_TYPE, PARAMETER_VALUE_TYPE>>(service, mParameter);
			}
		}

		/**
		 * Deconstructor
		 */
		virtual ~SequencePlayerCurveAdapter() {}

		/**
		 * update
		 * called from sequence player thread
		 * @param time time in sequence player
		 */
		virtual void update(double time) override
		{
			for (const auto& segment : mTrack->mSegments)
			{
				if (time >= segment->mStartTime &&
					time < segment->mStartTime + segment->mDuration)
				{
					assert(segment.get()->get_type().is_derived_from(RTTI_OF(SequenceTrackSegmentCurve<CURVE_TYPE>)));
					const SequenceTrackSegmentCurve<CURVE_TYPE>& source = static_cast<const SequenceTrackSegmentCurve<CURVE_TYPE>&>(*segment.get());

					CURVE_TYPE sourceValue = source.getValue((time - source.mStartTime) / source.mDuration);
					PARAMETER_VALUE_TYPE value = static_cast<PARAMETER_VALUE_TYPE>(sourceValue * (mTrack->mMaximum - mTrack->mMinimum) + mTrack->mMinimum);

					if (!mUseMainThread)
						mParameter.setValue(value);
					else
					{
						assert(mSetter != nullptr);
						mSetter->storeValue(value);
					}

					break;
				}
			}
		}
	private:
		PARAMETER_TYPE&									mParameter;
		SequenceTrackCurve<CURVE_TYPE>*					mTrack;
		bool											mUseMainThread;
		SequenceService&								mService;
		std::unique_ptr<SequencePlayerParameterSetter<PARAMETER_TYPE, PARAMETER_VALUE_TYPE>>	mSetter;
	};


	using SequencePlayerCurveInputObjectCreator = rtti::ObjectCreator<SequencePlayerCurveInput, SequenceService>;


	template<>
	bool SequencePlayerCurveAdapterBase<float>::sFactoryRegistered = SequencePlayerAdapter::registerFactory(RTTI_OF(SequenceTrackCurveFloat), [](SequenceTrack& track, SequencePlayerInput& input)->std::unique_ptr<SequencePlayerAdapter>
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
	

	template<>
	bool SequencePlayerCurveAdapterBase<glm::vec2>::sFactoryRegistered = SequencePlayerAdapter::registerFactory(RTTI_OF(SequenceTrackCurveVec2), [](SequenceTrack& track, SequencePlayerInput& input)->std::unique_ptr<SequencePlayerAdapter>
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


	template<>
	bool SequencePlayerCurveAdapterBase<glm::vec3>::sFactoryRegistered = SequencePlayerAdapter::registerFactory(RTTI_OF(SequenceTrackCurveVec3), [](SequenceTrack& track, SequencePlayerInput& input)->std::unique_ptr<SequencePlayerAdapter>
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


	template<>
	bool SequencePlayerCurveAdapterBase<glm::vec4>::sFactoryRegistered = SequencePlayerAdapter::registerFactory(RTTI_OF(SequenceTrackCurveVec4), [](SequenceTrack& track, SequencePlayerInput& input)->std::unique_ptr<SequencePlayerAdapter>
	{
		nap::Logger::info("adapter not yet implemented!");
		return nullptr;
	});
}
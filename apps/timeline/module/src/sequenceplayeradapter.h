#pragma once

// local includes
#include "sequencetracksegmentevent.h"
#include "sequencetrack.h"
#include "sequenceplayerparametersetter.h"

// external includes
#include <nap/resource.h>
#include <parameter.h>
#include <parametervec.h>

namespace nap
{
	class SequencePlayerAdapter
	{
	public:
		SequencePlayerAdapter() {};
		virtual ~SequencePlayerAdapter() {}

		virtual void process(double time) = 0;
	};

	template<typename CURVE_TYPE, typename PARAMETER_TYPE, typename PARAMETER_VALUE_TYPE>
	class SequencePlayerCurveAdapter : public SequencePlayerAdapter
	{
	public:
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

		virtual ~SequencePlayerCurveAdapter() {}

		/**
		 * Called from player thread
		 */
		virtual void process(double time) override
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

	class SequencePlayerEventAdapter : public SequencePlayerAdapter
	{
	public:
		SequencePlayerEventAdapter(SequenceTrack& track, SequenceEventReceiver& receiver);

		virtual ~SequencePlayerEventAdapter() {}

		virtual void process(double time);
	private:
		SequenceTrack& mTrack;
		SequenceEventReceiver& mReceiver;
		std::unordered_set<SequenceTrackSegmentEvent*> mDispatchedEvents;
	};

}
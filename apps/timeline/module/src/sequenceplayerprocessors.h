#pragma once

// local includes
#include "sequencetracksegmentevent.h"
#include "sequencetrack.h"

// external includes
#include <nap/resource.h>
#include <parameter.h>
#include <parametervec.h>

namespace nap
{
	class SequenceService;
	class SequenceEventReceiver;
	class SequenceEvent;

	class SequencePlayerProcessorBase
	{
	public:
		SequencePlayerProcessorBase() {};

		virtual void process(double time) = 0;
	};

	class SequencePlayerParameterSetterBase
	{
	public:
		SequencePlayerParameterSetterBase(SequenceService& service);

		~SequencePlayerParameterSetterBase();

		virtual void setValue() = 0;
	protected:
		SequenceService&		mService;
		bool					mUpdate;
		std::mutex				mMutex;
	};

	template<typename PARAMETER_TYPE, typename PARAMETER_VALUE_TYPE>
	class SequencePlayerParameterSetter :
		public SequencePlayerParameterSetterBase
	{
	public:
		SequencePlayerParameterSetter(SequenceService& service, PARAMETER_TYPE& parameter)
			: SequencePlayerParameterSetterBase(service),
			 mParameter(parameter) {}

		/**
		 * Called from player thread
		 */
		void storeValue(PARAMETER_VALUE_TYPE value)
		{
			std::lock_guard<std::mutex> l(mMutex);

			mValue = value;
			mUpdate = true;
		}

		/**
		 * Called from service main thread
		 */
		virtual void setValue() override
		{
			std::lock_guard<std::mutex> l(mMutex);

			if (mUpdate)
			{
				mParameter.setValue(mValue);
				mUpdate = false;
			}
		}
	private:
		PARAMETER_TYPE&			mParameter;
		PARAMETER_VALUE_TYPE	mValue;
	};

	template<typename CURVE_TYPE, typename PARAMETER_TYPE, typename PARAMETER_VALUE_TYPE>
	class SequencePlayerProcessorCurve : public SequencePlayerProcessorBase
	{
	public:
		SequencePlayerProcessorCurve(
			SequenceTrack& track, 
			PARAMETER_TYPE& parameter,
			SequenceService &service,
			bool useMainThread)
			: mParameter(parameter),
			  mUseMainThread(useMainThread),
			  mTrack(static_cast<SequenceTrackCurve<CURVE_TYPE>&>(track)),
			  mService(service)
		{
			if (mUseMainThread)
			{
				mSetter = std::make_unique<SequencePlayerParameterSetter<PARAMETER_TYPE, PARAMETER_VALUE_TYPE>>(service, mParameter);
			}
		}

		/**
		 * Called from player thread
		 */
		virtual void process(double time) override
		{
			for (const auto& segment : mTrack.mSegments)
			{
				if (time >= segment->mStartTime &&
					time < segment->mStartTime + segment->mDuration)
				{
					assert(segment.get()->get_type().is_derived_from(RTTI_OF(SequenceTrackSegmentCurve<CURVE_TYPE>)));
					const SequenceTrackSegmentCurve<CURVE_TYPE>& source = static_cast<const SequenceTrackSegmentCurve<CURVE_TYPE>&>(*segment.get());

					CURVE_TYPE sourceValue = source.getValue((time - source.mStartTime) / source.mDuration);
					PARAMETER_VALUE_TYPE value = static_cast<PARAMETER_VALUE_TYPE>(sourceValue * (mTrack.mMaximum - mTrack.mMinimum) + mTrack.mMinimum);
					
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
		SequenceTrackCurve<CURVE_TYPE>&					mTrack;
		bool											mUseMainThread;
		SequenceService&								mService;
		std::unique_ptr<SequencePlayerParameterSetter<PARAMETER_TYPE, PARAMETER_VALUE_TYPE>>	mSetter;
	};

	class SequencePlayerProcessorEvent : public SequencePlayerProcessorBase
	{
	public:
		SequencePlayerProcessorEvent(SequenceTrack& track, SequenceEventReceiver& receiver);

		virtual void process(double time);
	private:
		SequenceTrack& mTrack;
		SequenceEventReceiver& mReceiver;
		std::unordered_set<SequenceTrackSegmentEvent*> mDispatchedEvents;
	};

}
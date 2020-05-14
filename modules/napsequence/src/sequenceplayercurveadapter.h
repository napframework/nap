#pragma once

// local includes
#include "sequenceplayer.h"
#include "sequenceplayeradapter.h"
#include "sequenceplayercurveoutput.h"
#include "sequencetrackcurve.h"

// nap includes
#include <nap/logger.h>
#include <parametervec.h>
#include <parameternumeric.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	class SequencePlayerCurveAdapterBase : public SequencePlayerAdapter
	{
		friend class SequencePlayerCurveOutput;
	private:
		virtual void setValue() = 0;
	};

	/**
	 * Responsible for translating the value read on a curve track, to a parameter
	 * When the user wants to do this on the main thread, it uses a SequencePlayerParameterSetter as an intermediate class to ensure thread safety,
	 * otherwise it sets the parameter value directly from the sequence player thread
	 */
	template<typename CURVE_TYPE, typename PARAMETER_TYPE, typename PARAMETER_VALUE_TYPE>
	class SequencePlayerCurveAdapter : public SequencePlayerCurveAdapterBase
	{
	public:
		/**
		 * Constructor
		 * @param track reference to sequence track that holds curve information
		 * @param parameter reference to parameter that is assigned to this track
		 * @param service reference to the sequence service, needed to sync with main thread
		 * @param useMain thread, whether to sync with the main thread or not
		 */
		SequencePlayerCurveAdapter(SequenceTrack& track, SequencePlayerCurveOutput& output)
			:	mParameter(static_cast<PARAMETER_TYPE&>(*output.mParameter.get())), mOutput(output)
		{
			assert(track.get_type().is_derived_from(RTTI_OF(SequenceTrackCurve<CURVE_TYPE>)));
			mTrack = static_cast<SequenceTrackCurve<CURVE_TYPE>*>(&track);

			if (mOutput.mUseMainThread)
			{
				mSetFunction = &SequencePlayerCurveAdapter::storeParameterValue;
				mOutput.registerAdapter(this);
			}else
			{
				mSetFunction = &SequencePlayerCurveAdapter::setParameterValue;
			}
		}

		/**
		 * Deconstructor
		 */
		virtual ~SequencePlayerCurveAdapter()
		{
			mOutput.removeAdapter(this);
		}

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

					(*this.*mSetFunction)(value);

					break;
				}
			}
		}
	private:
		virtual void setValue() override
		{
			std::unique_lock<std::mutex> l(mMutex);
			mParameter.setValue(mStoredValue);
		}

		/**
		 * Directly sets parameter value, not thread safe
		 * @param value the value
		 */
		void setParameterValue(PARAMETER_VALUE_TYPE& value)
		{
			mParameter.setValue(value);
		}

		/**
		 * Uses parameter setter to set parameter value, value will be set from main thread, thread safe
		 * @param value the value
		 */
		void storeParameterValue(PARAMETER_VALUE_TYPE& value)
		{
			std::unique_lock<std::mutex> l(mMutex);
			mStoredValue = value;
		}

		PARAMETER_TYPE&									mParameter;
		SequenceTrackCurve<CURVE_TYPE>*					mTrack;
		bool											mUseMainThread;
		SequencePlayerCurveOutput&						mOutput;
		std::mutex										mMutex;
		PARAMETER_VALUE_TYPE							mStoredValue;

		void (SequencePlayerCurveAdapter::*mSetFunction)(PARAMETER_VALUE_TYPE& value);
	};
}
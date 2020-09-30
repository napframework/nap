/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

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

	/**
	 * Base class for curve adapters
	 * setValue must be called from main thread to set value of parameter on main thread
	 */
	class SequencePlayerCurveAdapterBase : public SequencePlayerAdapter
	{
		friend class SequencePlayerCurveOutput;
	private:
		virtual void setValue() = 0;
	};

	/**
	 * Responsible for translating the value read on a curve track, to a parameter
	 * When the user wants to do this on the main thread, it uses a SequencePlayerCurveOutput as an intermediate class to ensure thread safety,
	 * SequencePlayerCurveOutput will then call setValue()
	 * otherwise it sets the parameter value directly from the sequence player thread
	 */
	template<typename CURVE_TYPE, typename PARAMETER_TYPE, typename PARAMETER_VALUE_TYPE>
	class SequencePlayerCurveAdapter : public SequencePlayerCurveAdapterBase
	{
	public:
		/**
		 * Constructor
		 * @param track reference to sequence track that holds curve information
		 * @param output reference to curve output
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
		 * called from sequence player thread
		 * @param time time in sequence player
		 */
		virtual void tick(double time) override
		{
			for (const auto& segment : mTrack->mSegments)
			{
				if (time >= segment->mStartTime && time < segment->mStartTime + segment->mDuration)
				{
					// get the segment we need
					assert(segment.get()->get_type().is_derived_from(RTTI_OF(SequenceTrackSegmentCurve<CURVE_TYPE>)));
					const SequenceTrackSegmentCurve<CURVE_TYPE>& source = static_cast<const SequenceTrackSegmentCurve<CURVE_TYPE>&>(*segment.get());

					// retrieve the source value
					CURVE_TYPE source_value = source.getValue((time - source.mStartTime) / source.mDuration);
					
					// cast it to a parameter value
					PARAMETER_VALUE_TYPE value = static_cast<PARAMETER_VALUE_TYPE>(source_value * (mTrack->mMaximum - mTrack->mMinimum) + mTrack->mMinimum);

					// call set or store function
					(*this.*mSetFunction)(value);

					break;
				}
			}
		}
	private:
		/**
		 * setValue gets called from main thread and sets the parameter value
		 */
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
		 * Uses SequencePlayerCurveOutput  to set parameter value, value will be set from main thread with function setValue(), thread safe
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
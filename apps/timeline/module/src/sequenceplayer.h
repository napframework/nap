#pragma once

// internal includes
#include "sequence.h"
#include "sequenceeventreceiver.h"
#include "SequenceTrackSegmentEvent.h"

// external includes
#include <nap/resource.h>
#include <parameter.h>
#include <parametervec.h>
#include <future>
#include <mutex>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	class SequencePlayerProcessorBase;

	/**
	 */
	class NAPAPI SequencePlayer : public Resource
	{
		friend class SequenceEditor;
		friend class SequenceEditorController;

		RTTI_ENABLE(Resource)
	public:
		bool init(utility::ErrorState& errorState) override;

		bool save(const std::string& name, utility::ErrorState& errorState);

		bool load(const std::string& name, utility::ErrorState& errorState);

		void play();

		void stop();

		void pause();

		void setIsLooping(bool isLooping);

		void setPlayerTime(double time);

		void setPlaybackSpeed(float speed);

		double getPlayerTime() const;

		double getDuration() const;

		bool getIsPlaying() const;

		bool getIsLooping() const;

		bool getIsPaused() const;

		float getPlaybackSpeed() const;

		virtual void onDestroy();

	public:
		// properties
		std::string			mDefaultShow;
		bool				mCreateDefaultShowOnFailure = true;
		float				mFrequency = 1000.0f;

		std::vector<ResourcePtr<Parameter>> mParameters;
		std::vector<ResourcePtr<SequenceEventReceiver>> mEventDispatchers;

		// Sequence Editor interface
		const Sequence& getSequenceConst() const;
	protected:
		// Sequence Editor interface
		Sequence& getSequence();

		bool createProcessor(const std::string& parameterID, const std::string& trackID);

		void onUpdate();

		//
		std::vector<std::unique_ptr<rtti::Object>>	mReadObjects;
		std::unordered_set<std::string>				mReadObjectIDs;

		//
		std::future<void>	mUpdateTask;
		std::mutex			mLock;

		//
		Sequence* mSequence = nullptr;

		bool mUpdateThreadRunning;
		bool mIsPlaying = false;
		bool mIsPaused = false;
		bool mIsLooping = false;
		float mSpeed = 1.0f;
		double mTime = 0.0;

		// used to calculate delta time in onUpdate
		std::chrono::high_resolution_clock mTimer;
		std::chrono::time_point<std::chrono::high_resolution_clock> mBefore;
	private:
		std::unique_lock<std::mutex> lock();

		std::unordered_map<std::string, std::unique_ptr<SequencePlayerProcessorBase>> mProcessors;
	};

	class SequencePlayerProcessorBase
	{
	public:
		SequencePlayerProcessorBase() {};

		virtual void process(double time) = 0;
	};

	template<typename CURVE_TYPE, typename PARAMETER_TYPE, typename PARAMETER_VALUE_TYPE>
	class SequencePlayerProcessorCurve : public SequencePlayerProcessorBase
	{
	public:
		SequencePlayerProcessorCurve(SequenceTrack& track, PARAMETER_TYPE& parameter)
			: mParameter(parameter), mTrack(static_cast<SequenceTrackCurve<CURVE_TYPE>&>(track)) {}

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
					mParameter.setValue(value);

					break;
				}
			}
		}
	private:
		PARAMETER_TYPE&					mParameter;
		SequenceTrackCurve<CURVE_TYPE>&		mTrack;
	};

	class SequencePlayerProcessorEvent : public SequencePlayerProcessorBase
	{
	public:
		SequencePlayerProcessorEvent(SequenceTrack& track, SequenceEventReceiver& dispatcher)
			: mTrack(track), mDispatcher(dispatcher) {}

		virtual void process(double time)
		{
			assert(mTrack.get_type().is_derived_from(RTTI_OF(SequenceTrackEvent)));
			auto& eventTrack = static_cast<SequenceTrackEvent&>(mTrack);
			for (const auto& eventSegment : eventTrack.mSegments)
			{
				assert(eventSegment.get()->get_type().is_derived_from(RTTI_OF(SequenceTrackSegmentEvent)));
				SequenceTrackSegmentEvent& event = static_cast<SequenceTrackSegmentEvent&>(*eventSegment.get());

				if (time > event.mStartTime && !event.mDispatched)
				{
					event.mDispatched = true;
					mDispatcher.addEvent(event.createEvent());
				}
				else if (time < event.mStartTime && event.mDispatched)
				{
					event.mDispatched = false;
				}
			}
		}
	private:
		SequenceTrack& mTrack;
		SequenceEventReceiver& mDispatcher;
	};

}

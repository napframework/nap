#pragma once

// internal includes
#include "sequence.h"

// external includes
#include <nap/resource.h>
#include <parameter.h>
#include <parametervec.h>
#include <future>
#include <mutex>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

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

		/**
		*
		*/
		template<typename CURVE_TYPE, typename PARAMETER_TYPE, typename PARAMETER_VALUE_TYPE>
		class ProcessorCurve : public ProcessorBase
		{
		public:
			ProcessorCurve(SequenceTrack& track, PARAMETER_TYPE& parameter)
				: mParameter(parameter), mTrack(static_cast<SequenceTrackCurve<CURVE_TYPE>&>(track)) {}

			virtual void process(double time) override
			{
				for (const auto& segment : mTrack.mSegments)
				{
					if (time >= segment->mStartTime &&
						time < segment->mStartTime + segment->mDuration)
					{
						SequenceTrackSegmentCurve<CURVE_TYPE>& source = segment->getDerived<SequenceTrackSegmentCurve<CURVE_TYPE>>();
						CURVE_TYPE sourceValue = source.getValue((time - source.mStartTime) / source.mDuration);

						const CURVE_TYPE& maximum = static_cast<CURVE_TYPE>(mTrack.mMaximum);
						const CURVE_TYPE& minimum = static_cast<CURVE_TYPE>(mTrack.mMinimum);

						PARAMETER_VALUE_TYPE value = static_cast<PARAMETER_VALUE_TYPE>(sourceValue * (maximum - minimum));
						value += static_cast<PARAMETER_VALUE_TYPE>(minimum);
						mParameter.setValue(value);

						break;
					}
				}
			}
		private:
			PARAMETER_TYPE&					mParameter;
			SequenceTrackCurve<CURVE_TYPE>&		mTrack;
		};

		/**
		 * 
		 */
		class ProcessorBase
		{
		public:
			ProcessorBase() {};

			virtual void process(double time) = 0;
		};

		std::unordered_map<std::string, std::unique_ptr<ProcessorBase>> mProcessors;
	};
}

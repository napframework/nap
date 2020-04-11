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
		template<typename T>
		class ProcessorNumeric : public ProcessorBase
		{
		public:
			ProcessorNumeric(SequenceTrack& track, ParameterNumeric<T>& parameter)
				: mParameter(parameter), mTrack(track) {}

			virtual void process(double time) override
			{
				for (const auto& segment : mTrack.mSegments)
				{
					if (time >= segment->mStartTime &&
						time < segment->mStartTime + segment->mDuration)
					{
						SequenceTrackSegmentNumeric& source = segment->getDerived<SequenceTrackSegmentNumeric>();
						T value = source.mCurve->evaluate((time - source.mStartTime) / source.mDuration)
							* static_cast<float>(mParameter.mMaximum - mParameter.mMinimum)
							+ mParameter.mMinimum;
						
						mParameter.setValue(value);
						break;
					}
				}
			}
		private:
			ParameterNumeric<T>& mParameter;
			SequenceTrack&	mTrack;
		};

		/**
		 *
		 */
		template<typename T>
		class ProcessorVector : public ProcessorBase
		{
		public:
			ProcessorVector(SequenceTrack& track, ParameterVec<T>& parameter)
				: mParameter(parameter), mTrack(track) {}

			virtual void process(double time) override
			{
				for (const auto& segment : mTrack.mSegments)
				{
					if (time >= segment->mStartTime &&
						time < segment->mStartTime + segment->mDuration)
					{
						SequenceTrackSegmentVec<T>& source = segment->getDerived<SequenceTrackSegmentVec<T>>();
						T value;
						for (int i = 0; i < source.mCurves.size(); i++)
						{
							const T& maximum = static_cast<T>(mParameter.mMaximum);
							const T& minimum = static_cast<T>(mParameter.mMinimum);

							value[i] = source.mCurves[i]->evaluate(
										(time - source.mStartTime) / source.mDuration)
										* static_cast<float>(maximum[i] - minimum[i])
										+ minimum[i];
						}
						mParameter.setValue(value);
						break;
					}
				}
			}
		private:
			ParameterVec<T>&	mParameter;
			SequenceTrack&		mTrack;
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

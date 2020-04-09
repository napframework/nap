#pragma once

// internal includes
#include "sequence.h"

// external includes
#include <nap/resource.h>
#include <parameter.h>
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

		std::vector<ResourcePtr<ParameterFloat>> mParameters;
	protected:
		// Sequence Editor interface
		Sequence& getSequence();

		void onUpdate();

		template<typename T, typename V>
		void processSegmentNumeric(SequenceTrackSegment& segment, Parameter& parameter, double time);

		//
		std::vector<std::unique_ptr<rtti::Object>>	mReadObjects;
		std::unordered_set<std::string>				mReadObjectIDs;

		//
		std::future<void>	mUpdateTask;
		std::mutex			mLock;

		//
		std::map<std::string, Parameter*>	mTrackMap;

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
	};

	//////////////////////////////////////////////////////////////////////////
	// Template Definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename T, typename V>
	void SequencePlayer::processSegmentNumeric(
		SequenceTrackSegment& segment, Parameter& parameter, double time)
	{
		T& target = segment.getDerived<T>();
		ParameterNumeric<V>& parameterCast = static_cast<ParameterNumeric<V>&>(parameter);

		V value = target.mCurve->evaluate((mTime - target.mStartTime) / target.mDuration);
		parameterCast.setValue(value * (parameterCast.mMaximum - parameterCast.mMinimum) + parameterCast.mMinimum);
	}
}

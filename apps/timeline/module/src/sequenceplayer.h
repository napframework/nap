#pragma once

// internal includes
#include "sequence.h"
#include "sequenceeventreceiver.h"
#include "sequenceplayerprocessors.h"


// external includes
#include <rtti/factory.h>
#include <nap/device.h>
#include <parameter.h>
#include <parametervec.h>
#include <future>
#include <mutex>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	class SequenceService;

	/**
	 */
	class NAPAPI SequencePlayer : public Device
	{
		friend class SequenceEditor;
		friend class SequenceEditorController;
		friend class SequenceService;

		RTTI_ENABLE(Device)
	public:
		// Constructor used by factory
		SequencePlayer(SequenceService& service);

		bool init(utility::ErrorState& errorState) override;

		bool save(const std::string& name, utility::ErrorState& errorState);

		bool load(const std::string& name, utility::ErrorState& errorState);

		void setIsPlaying(bool isPlaying);

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

		virtual void stop() override;

		virtual bool start(utility::ErrorState& errorState) override;

	public:
		// properties
		std::string			mDefaultShow;
		bool				mCreateDefaultShowOnFailure = true;
		float				mFrequency = 1000.0f;
		bool				mSetParametersOnMainThread = true;

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
	
		SequenceService& mSequenceService;
	};

	using SequencePlayerObjectCreator = rtti::ObjectCreator<SequencePlayer, SequenceService>;
}

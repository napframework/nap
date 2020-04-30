#pragma once

// internal includes
#include "sequence.h"
#include "sequenceeventreceiver.h"
#include "sequenceplayeradapter.h"


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
	// forward declares
	class SequenceService;

	/**
	 * SequencePlayer
	 * The sequence player is responsible for loading / playing and saving a sequence
	 * The player dispatches a thread which reads the sequence. Actions for each track of the sequence are handle by SequencePlayerAdapters
	 * A Sequence can only be edited by a derived class from SequenceController
	 * Sequence Player owns all Sequence objects 
	 * GUI can be linked to a SequencePlayer ( see SequencePlayerGUI )
	 */
	class NAPAPI SequencePlayer : public Device
	{
		friend class SequenceEditor;
		friend class SequenceEditorController;
		friend class SequenceService;

		RTTI_ENABLE(Device)
	public:
		/**
		 * Constructor used by factory
		 */
		SequencePlayer(SequenceService& service);

		/**
		 * init 
		 * evaluates the data of the player. It loads the linked default sequence. 
		 * Upon failure of loading show, it creates a new default ( empty ) sequence
		 *
		 * @param errorState contains information about eventual failure 
		 * @return true if data valid
		 */
		bool init(utility::ErrorState& errorState) override;

		/**
		 * save
		 * Saves current sequence to disk
		 * @param name of the sequence
		 * @param errorState contains error upon failure
		 * @return true on success
		 */
		bool save(const std::string& name, utility::ErrorState& errorState);

		/**
		 * load
		 * Load a sequence
		 * @param name of the sequence
		 * @param errorState contains error upon failure
		 * @return true on success
		 */
		bool load(const std::string& name, utility::ErrorState& errorState);

		/**
		 * setIsPlaying
		 * Play or stop the player. Note that player can still be paused, so adapters will be called but time will not advance
		 * @param isPlaying true is start playing
		 */
		void setIsPlaying(bool isPlaying);

		/**
		 * setIsPaused
		 * Pauses the player. Note that if we are still playing, adapters will still get called but time will not advance ( when paused )
		 * @param isPaused paused
		 */
		void setIsPaused(bool isPaused);

		/**
		 * setIsLooping
		 * Start from beginning when player reaches end of sequence
		 * @param isLooping loop
		 */
		void setIsLooping(bool isLooping);

		/**
		 * setPlayingTime
		 * sets player time manually
		 * @param time the new time
		 */
		void setPlayerTime(double time);

		/**
		 * setPlaybackSpeed
		 * sets playback speed ( 1.0 is normal speed )
		 * @param speed speed
		 */
		void setPlaybackSpeed(float speed);

		/**
		 * getPlayerTime
		 * @return the current player time
		 */
		double getPlayerTime() const;

		/**
		 * getDuration
		 * @return gets sequence total duration
		 */
		double getDuration() const;

		/**
		 * getIsPlaying
		 * @return are we playing?
		 */
		bool getIsPlaying() const;

		/**
		 * getIsLooping
		 * @return are we looping?
		 */
		bool getIsLooping() const;

		/**
		 * getIsPaused
		 * @return are we paused ?
		 */
		bool getIsPaused() const;

		/**
		 * getPlaybackSpeed
		 * @return current playback speed
		 */
		float getPlaybackSpeed() const;

		/**
		 * stop
		 * called before deconstruction. This stops the actual player thread. To stop the player but NOT the player thread call setIsPlaying( false )
		 */
		virtual void stop() override;

		/**
		 * start
		 * starts player thread, called after successfully initialization
		 * This starts the actual player thread
		 */
		virtual bool start(utility::ErrorState& errorState) override;

		/**
		 * getSequenceConst
		 * @return get const reference to sequence
		 */
		const Sequence& getSequenceConst() const;
	public:
		// properties
		std::string			mDefaultSequence; ///< Property: 'Default Sequence' linked default Sequence file
		bool				mCreateDefaultShowOnFailure = true; ///< Property: 'Create Sequence on Failure' when true, the init will successes upon failure of loading default sequence and create an empty sequence
		float				mFrequency = 1000.0f; ///< Property: 'Frequency' frequency of player thread
		bool				mSetParametersOnMainThread = true;  ///< Property: 'Set parameters on main thread' when true, any animated parameters will be set on main thread and not player thread
		std::vector<ResourcePtr<Parameter>> mParameters;  ///< Property: 'Parameters' linked parameters
		std::vector<ResourcePtr<SequenceEventReceiver>> mEventReceivers; ///< Property: 'Event Receivers' linked event receivers
	private:
		/**
		 * getSequence
		 * returns reference to sequence
		 */
		Sequence& getSequence();

		/**
		 * createAdapter
		 * creates an adapter with string objectID for track with trackid. This searches the list of appropriate adapter types for the corresponding track id and creates it if available
		 * @param objectID the id of the adapter object
		 * @param trackID the id of the track
		 * @param player lock, the player needs to be locked, to ensure this, pass a unique lock
		 */
		bool createAdapter(const std::string& objectID, const std::string& trackID, const std::unique_lock<std::mutex>& playerLock);

		/**
		 * onUpdate
		 * The threaded update function
		 */
		void onUpdate();

		/**
		 * lock
		 * creates lock on mMutex
		 */
		std::unique_lock<std::mutex> lock();
	private:
		template<typename CURVE_TYPE, typename PARAMETER_TYPE, typename PARAMETER_VALUE_TYPE>
		std::unique_ptr<SequencePlayerAdapter> createParameterAdapter(SequenceTrack& track, Parameter& parameter);

		// read objects from sequence
		std::vector<std::unique_ptr<rtti::Object>>	mReadObjects;

		// read object ids from sequence
		std::unordered_set<std::string>				mReadObjectIDs;

		// the update task
		std::future<void>	mUpdateTask;

		// mutex
		std::mutex			mLock;

		// raw pointer to loaded sequence
		Sequence* mSequence = nullptr;

		// true when thread is running
		bool mUpdateThreadRunning;

		// is playing
		bool mIsPlaying = false;

		// is paused
		bool mIsPaused = false;

		// is looping
		bool mIsLooping = false;

		// speed
		float mSpeed = 1.0f;

		// current time
		double mTime = 0.0;

		// used to calculate delta time in onUpdate
		std::chrono::high_resolution_clock mTimer;
		std::chrono::time_point<std::chrono::high_resolution_clock> mBefore;

		// list of instantiated adapters
		std::unordered_map<std::string, std::unique_ptr<SequencePlayerAdapter>> mAdapters;

		// reference to service
		SequenceService& mSequenceService;

		struct PairHash
		{
			template <class T1, class T2>
			std::size_t operator() (const std::pair<T1, T2> &pair) const
			{
				return std::hash<T1>()(pair.first) ^ std::hash<T2>()(pair.second);
			}
		};

		static std::unordered_map<rttr::type, std::function<std::unique_ptr<SequencePlayerAdapter>(SequencePlayer&, SequenceTrack&, const std::string&)>> sCreateAdapterMap;
		static std::unordered_map<std::pair<rttr::type, rttr::type>, std::function<std::unique_ptr<SequencePlayerAdapter>(SequencePlayer&, SequenceTrack&, Parameter&)>, PairHash> sCreateCurveAdapterMap;
		static std::function<std::unique_ptr<SequencePlayerAdapter>(SequencePlayer&, SequenceTrack&, const std::string&)> sCreateCurveAdapterFunction;
		static std::function<std::unique_ptr<SequencePlayerAdapter>(SequencePlayer&, SequenceTrack&, const std::string&)> sCreateEventAdapterFunction;
	};

	using SequencePlayerObjectCreator = rtti::ObjectCreator<SequencePlayer, SequenceService>;
}

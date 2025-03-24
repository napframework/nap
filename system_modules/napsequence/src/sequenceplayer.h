/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// internal includes
#include "sequence.h"
#include "sequenceplayeradapter.h"
#include "sequenceplayeroutput.h"
#include "sequenceservice.h"
#include "sequenceplayerclock.h"

// external includes
#include <rtti/factory.h>
#include <nap/device.h>
#include <nap/signalslot.h>
#include <future>
#include <mutex>
#include <nap/timer.h>
#include <concurrentqueue.h>
#include <nap/numeric.h>

namespace nap
{
    //////////////////////////////////////////////////////////////////////////

    // forward declares
    class SequenceService;

    /**
     * The sequence player is responsible for loading / playing and saving a sequence
     * The player dispatches a thread which reads the sequence. Actions for each track of the sequence are handle by SequencePlayerAdapters
     * A Sequence can only be edited by a derived class from SequenceController and SequenceEditor
     * Sequence Player owns all Sequence objects
     */
    class NAPAPI SequencePlayer : public Device
    {
        friend class SequenceEditor;
        friend class SequenceController;

    RTTI_ENABLE(Device)
    public:
        /**
         * Constructor used by factory
         */
        SequencePlayer(SequenceService& service);

        /**
         * Evaluates the data of the player. It loads the linked default sequence.
         * Upon failure of loading show, it can create a new default ( empty ) sequence
         * @param errorState contains information about eventual failure
         * @return true if data valid
         */
        bool init(utility::ErrorState& errorState) override;

        /**
         * Saves current sequence to disk
         * @param name of the sequence
         * @param errorState contains error upon failure
         * @return true on success
         */
        bool save(const std::string& name, utility::ErrorState& errorState);

        /**
         * Load a sequence
         * @param name of the sequence
         * @param errorState contains error upon failure
         * @return true on success
         */
        bool load(const std::string& name, utility::ErrorState& errorState);

        /**
         * Load a sequence serialized as a binary blob
         * @param buffer reference to binary data
         * @param errorState contains error upon failure
         * @return true on success
         */
        bool loadBinary(const std::vector<nap::uint8>& buffer, utility::ErrorState& errorState);

        /**
         * Play or stop the player. Note that player can still be paused, so adapters will be called but time will not advance
         * @param isPlaying true is start playing
         */
        void setIsPlaying(bool isPlaying);

        /**
         * Pauses the player. Note that if we are still playing, adapters will still get called but time will not advance ( when paused )
         * @param isPaused paused
         */
        void setIsPaused(bool isPaused);

        /**
         * Start from beginning when player reaches end of sequence
         * @param isLooping loop
         */
        void setIsLooping(bool isLooping);

        /**
         * sets player time manually
         * @param time the new time
         */
        void setPlayerTime(double time);

        /**
         * sets playback speed ( 1.0 is normal speed )
         * @param speed speed
         */
        void setPlaybackSpeed(float speed);

        /**
         * @return the current player time
         */
        double getPlayerTime() const;

        /**
         * @return gets sequence total duration
         */
        double getDuration() const;

        /**
         * @return are we playing?
         */
        bool getIsPlaying() const;

        /**
         * @return are we looping?
         */
        bool getIsLooping() const;

        /**
         * @return are we paused ?
         */
        bool getIsPaused() const;

        /**
         * @return current playback speed
         */
        float getPlaybackSpeed() const;

        /**
         * called before deconstruction. This stops the actual player thread. To stop the player but NOT the player thread call setIsPlaying( false )
         */
        void stop() override;

        /**
         * starts player thread, called after successfully initialization
         * This starts the actual player thread
         */
        bool start(utility::ErrorState& errorState) override;

        /**
         * returns a const reference to the current sequence, not thread-safe. Only call this from the main thread
         * @return get const reference to sequence
         */
        const Sequence& getSequenceConst() const;

        /**
         * @return returns current sequence filename, not thread-safe. Only call this from the main thread
         */
        const std::string& getSequenceFilename() const;

        // signals
        /***
         * playbackSpeedChanged signal is dispatched when setPlaybackSpeed(float) method is called on SequencePlayer
         * This is useful when you want to sync or invoke other methods with the SequencePlayer
         * Note: Signal is dispatched from whichever thread is calling the setPlaybackSpeed(float) method on SequencePlayer
         */
        Signal<SequencePlayer&, float> playbackSpeedChanged;

        /***
         * playerTimeChanged signal is dispatched when setPlayerTime(double) method is called on SequencePlayer
         * This is useful when you want to sync or invoke other methods with the SequencePlayer
         * Note: Signal is dispatched from whichever thread is calling the setPlayerTime(double) method on SequencePlayer
         */
        Signal<SequencePlayer&, double> playerTimeChanged;

        /***
         * playStateChanged signal is dispatched when setIsPlaying(bool) method is called on SequencePlayer
         * This is useful when you want to sync or invoke other methods with the SequencePlayer
         * Note: Signal is dispatched from whichever thread is calling the setIsPlaying(bool) method on SequencePlayer
         * Note: Play state doesn't mean Paused or not. The SequencePlayer can be Paused but still be Playing, in which case time doesn't get updated but adapter will still be called
         */
        Signal<SequencePlayer&, bool> playStateChanged;

        /***
         * pauseStateChanged signal is dispatched when setIsPaused(bool) method is called on SequencePlayer
         * This is useful when you want to sync or invoke other methods with the SequencePlayer
         * Note: Signal is dispatched from whichever thread is calling the setIsPaused(bool) method on SequencePlayer
         * Note: Play state doesn't mean Paused or not. The SequencePlayer can be Paused but still be Playing, in which case time doesn't get updated but adapter will still be called
         */
        Signal<SequencePlayer&, bool> pauseStateChanged;

        /***
         *	preTick Signal is triggered on player thread, before updating the adapters
         */
        Signal<SequencePlayer&> preTick;

        /**
         * postTick Signal is triggered on player thread, after updating the adapters
         */
        Signal<SequencePlayer&> postTick;

        /***
         * sequenceLoaded signal is dispatched when the load(name) method succeeds on SequencePlayer
         */
        Signal<SequencePlayer&, std::string> sequenceLoaded;

        /**
         * edited Signal is dispatched when the Sequence has been edited
         */
        Signal<SequencePlayer&> edited;

        // properties
        std::string mSequenceFileName; ///< Property: 'Default Sequence' linked default Sequence file
        bool mCreateEmptySequenceOnLoadFail = true; ///< Property: 'Create Sequence on Failure' when true, the init will successes upon failure of loading default sequence and create an empty sequence
        std::vector<ResourcePtr<SequencePlayerOutput>> mOutputs;  ///< Property: 'Outputs' linked outputs
        ResourcePtr<SequencePlayerClock> mClock;	///< Property: 'Clock' Controls timing of playback
    protected:
        /**
         * adptersCreated Signal is triggered from main thread, after creating adapters
         * This is useful for creating your own custom outputs & adapters for custom tracks if necessary
         * You should do so only when writing your own player extended on SequencePlayer
         * It passes a reference to a lambda function that you can call to add an adapter to mAdapters member of
         * SequencePlayer
         */
        Signal<std::function<void(const std::string&, std::unique_ptr<SequencePlayerAdapter>)>&> adaptersCreated;

        // Reference to sequence service
        SequenceService& mService;
    private:
        /**
         * returns reference to sequence, can only be called internally or by friend class ( SequenceEditor or SequenceController )
         */
        Sequence& getSequence();

        /**
         * creates an adapter with string objectID for track with trackid.
         * This searches the list of appropriate adapter types for the corresponding track id and creates one if available
         * @param objectID the id of the adapter object
         * @param trackID the id of the track
         */
        bool createAdapter(const std::string& objectID, const std::string& trackID);

        void tick(double deltaTime);

        // read objects from sequence
        std::vector<std::unique_ptr<rtti::Object>> mReadObjects;

        // read object ids from sequence
        std::unordered_set<std::string> mReadObjectIDs;

        /**
         * creates adapters for all assigned adapter ids for tracks
         * this function gets called by the player when player starts playing
         */
        void createAdapters();

        /**
         * destroys all created adapters, gets called on stop
         */
        void destroyAdapters();

        /**
         * performs given action when mutex is unlocked, makes sure edit action on sequence are thread safe
         * @param action the edit action
         */
        void performEditAction(std::function<void()> &action);

        // mutex
        std::mutex mMutex;

        // raw pointer to loaded sequence
        Sequence *mSequence = nullptr;

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

        // list of instantiated adapters
        std::unordered_map<std::string, std::unique_ptr<SequencePlayerAdapter>> mAdapters;
    };

    using SequencePlayerObjectCreator = rtti::ObjectCreator<SequencePlayer, SequenceService>;
}

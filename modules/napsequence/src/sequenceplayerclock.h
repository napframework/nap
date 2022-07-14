/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// external includes
#include <future>
#include <nap/resource.h>
#include <nap/signalslot.h>
#include <nap/timer.h>
#include <rtti/rtti.h>
#include <rtti/factory.h>

namespace nap
{
    //////////////////////////////////////////////////////////////////////////

    // forward declares
    class SequenceService;

    /**
     * The SequencePlayerClock is used by the sequence player to get its update calls
     * The idea behind this is that we can synchronize the SequencePlayer to any threads, external sources or clocks
     */
    class NAPAPI SequencePlayerClock : public Resource
    {
    RTTI_ENABLE(Resource)
    public:
        // default constructor and deconstructor
        SequencePlayerClock() = default;

        ~SequencePlayerClock() = default;

        // make this class explicitly non-copyable
        SequencePlayerClock(const SequencePlayerClock &) = delete;

        SequencePlayerClock &operator=(const SequencePlayerClock &) = delete;

        /**
         * Start needs to be overloaded. The update slot is the slot that needs to be called on update with a delta time
         * DeltaTime needs to be in seconds
         * @param updateSlot the update slot from the SequencePlayer that needs to be called
         */
        virtual void start(Slot<double> &updateSlot) = 0;

        /**
         * Called by the SequencePlayer when player is deconstructed
         */
        virtual void stop() = 0;

    protected:
        // the slot
        Slot<double> mSlot;
    };

    /**
     * SequencePlayerStandardClock is being updated by the SequenceService on update, this means that the SequencePlayer
     * is being updated on the main thread
     */
    class NAPAPI SequencePlayerStandardClock : public SequencePlayerClock
    {
        friend class SequenceService;

    RTTI_ENABLE(SequencePlayerClock)
    public:
        /**
         * Constructor
         * @param service reference to sequence service
         */
        SequencePlayerStandardClock(SequenceService &service);

        // make this class explicitly non-copyable
        SequencePlayerStandardClock(const SequencePlayerStandardClock &) = delete;

        SequencePlayerStandardClock &operator=(const SequencePlayerStandardClock &) = delete;

        /**
         * Called by sequence player upon initialization
         * @param updateSlot the update slot from the SequencePlayer that needs to be called
         */
        void start(Slot<double> &updateSlot) override;

        /**
         * Called by the SequencePlayer when player is deconstructed
         */
        void stop() override;

    private:
        /**
         * called from SequenceService
         * @param deltaTime
         */
        void update(double deltaTime);

        // reference to service
        SequenceService &mService;
    };

    // factory method shortcut
    using SequencePlayerStandClockObjectCreator = rtti::ObjectCreator<SequencePlayerStandardClock, SequenceService>;

    /**
     * The SequencePlayerIndependentClock can be used to let the SequencePlayer be updated by a thread started by the
     * SequencePlayerIndependentClock.
     */
    class NAPAPI SequencePlayerIndependentClock : public SequencePlayerClock
    {
    RTTI_ENABLE(SequencePlayerClock)
    public:
        /**
         * Constructor
         */
        SequencePlayerIndependentClock() = default;

        /**
         * Destructor
         */
        ~SequencePlayerIndependentClock() = default;

        // make this class explicitly non-copyable
        SequencePlayerIndependentClock(const SequencePlayerIndependentClock &) = delete;

        SequencePlayerIndependentClock &operator=(const SequencePlayerIndependentClock &) = delete;

        /**
         * Initialization method
         * @param errorState contains any errors
         * @return true on success
         */
        bool init(utility::ErrorState &errorState) override;

        /**
         * Called by sequence player upon initialization
         * @param updateSlot the update slot from the SequencePlayer that needs to be called
         */
        void start(Slot<double> &updateSlot) override;

        /**
         * Called by the SequencePlayer when player is deconstructed
         */
        void stop() override;

        // properties
        float mFrequency = 1000.0f; ///< Property: 'Frequency' the update frequency in times per second (Hz)
    private:
        void onUpdate();

        // the task
        std::future<void> mUpdateTask;

        // bool indicating if thread is running
        std::atomic_bool mRunning = {false};

        // previous timestamp, used to calculate deltaTime
        SteadyTimeStamp mBefore;
    };
}

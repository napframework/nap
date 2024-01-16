/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Std includes
#include <set>

// Nap includes
#include <nap/service.h>
#include <concurrentqueue.h>

// Midi includes
#include "midievent.h"

namespace nap {

    // Forward declarations
    class MidiInputPort;
    class MidiInputComponentInstance;


    /**
     * Service keeping tracking of opened midi input ports and processing incoming messages.
     */
    class NAPAPI MidiService : public nap::Service
    {
    RTTI_ENABLE(nap::Service)

        friend class MidiInputPort;
        friend class MidiInputComponentInstance;

    public:
        MidiService(ServiceConfiguration* configuration);

        // Initialization
        bool init(nap::utility::ErrorState& errorState) override;

        /**
         * Enqueue a freshly received midi event from the input thread.
         */
        void enqueueEvent(std::unique_ptr<MidiEvent> event) { mEventQueue.enqueue(std::move(event)); }

        /**
         * Processes all received midi events
         */
        void update(double deltaTime) override;

    private:
        // Used by input component to register itself to receive incoming midi events
        void registerInputComponent(MidiInputComponentInstance& component) { mInputComponents.emplace(&component); }

        // Used by input component to unregister itself.
        void unregisterInputComponent(MidiInputComponentInstance& component) { mInputComponents.erase(&component); }

        std::set<MidiInputComponentInstance*> mInputComponents; // all registered midi input components

        /**
         * lock-free concurrent queue to store incoming midi events before processing them on the main thread.
         */
        moodycamel::ConcurrentQueue<std::unique_ptr<MidiEvent>> mEventQueue;
    };

}
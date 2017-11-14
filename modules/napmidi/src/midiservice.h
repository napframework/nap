#pragma once

// Std includes
#include <set>

// Third party includes
#include <RtMidi.h>

// Nap includes
#include <nap/service.h>
#include <nap/concurrentqueue.h>

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
        MidiService() = default;
        
        // Initialization
        bool init(nap::utility::ErrorState& errorState);
        
        /**
         * Returns number of available midi input ports on the system.
         */
        int getInputPortCount();
        
        /**
         * Returns the name of a given input port number on this system.
         */
        std::string getInputPortName(int portNumber);
        
        /**
         * Returns the number of an input port with the given name.
         * Returns -1 when no port with this name has been found.
         */
        int getInputPortNumber(const std::string& portName);
        
        /**
         * Returns the number of available midi output ports on the system.
         */
        int getOutputPortCount();
        
        /**
         * Returns the name of a given output port number on this system.
         */
        std::string getOutputPortName(int portNumber);

        /**
         * Returns the number of an output port with the given name.
         * Returns -1 when no port with this name has been found.
         */
        int getOutputPortNumber(const std::string& portName);
        
        /**
         * Logs a list of all available midi input and output ports on this system.
         */
        void printPorts();
        
        /**
         * Processes all received midi events
         */
        void update(double deltaTime) override;
        
    protected:
        void registerObjectCreators(rtti::Factory& factory) override final;
        
    private:
         // Used by midi input port to register itself with this service.
        void registerInputPort(MidiInputPort& port) { mInputPorts.emplace(&port); }
        
         // Used by midi input port to unregister itself with this service
        void unregisterInputPort(MidiInputPort& port) { mInputPorts.erase(&port); }
        
         // Used by input component to register itself to receive incoming midi events
        void registerInputComponent(MidiInputComponentInstance& component) { mInputComponents.emplace(&component); }
        
         // Used by input component to unregister itself.
        void unregisterInputComponent(MidiInputComponentInstance& component) { mInputComponents.erase(&component); }
        
         // Used by midi input port to enqueue a freshly received midi event from the input thread.
        void enqueueEvent(std::unique_ptr<MidiEvent> event) { mEventQueue.enqueue(std::move(event)); }
        
        std::unique_ptr<RtMidiIn> mMidiIn = nullptr; // used to poll for available input ports
        std::unique_ptr<RtMidiOut> mMidiOut = nullptr; // used to poll available output ports.
        
        std::set<MidiInputPort*> mInputPorts; // all registered midi input ports
        std::set<MidiInputComponentInstance*> mInputComponents; // all registered midi input components
        
        /**
         * lock-free concurrent queue to store incoming midi events before processing them on the main thread.
         */
        moodycamel::ConcurrentQueue<std::unique_ptr<MidiEvent>> mEventQueue;
    };
        
}

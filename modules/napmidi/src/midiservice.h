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

    
    class NAPAPI MidiService : public nap::Service {
        RTTI_ENABLE(nap::Service)
        
        friend class MidiInputPort;
        friend class MidiInputComponentInstance;
        
    public:
        MidiService() = default;
        
        // Initialization
        bool init(nap::utility::ErrorState& errorState);
        
        int getInputPortCount();
        std::string getInputPortName(int portNumber);
        
        int getOutputPortCount();
        std::string getOutputPortName(int portNumber);

        void printPorts();
        
        /**
         * Processes all received midi events
         */
        void update();
        
    protected:
        void registerObjectCreators(rtti::Factory& factory) override final;
        
    private:
        void registerInputPort(MidiInputPort& port) { mInputPorts.emplace(&port); }
        void unregisterInputPort(MidiInputPort& port) { mInputPorts.erase(&port); }
        
        void registerInputComponent(MidiInputComponentInstance& component) { mInputComponents.emplace(&component); }
        void unregisterInputComponent(MidiInputComponentInstance& component) { mInputComponents.erase(&component); }
        
        void enqueueEvent(std::unique_ptr<MidiEvent> event) { mEventQueue.enqueue(std::move(event)); }
        
        std::unique_ptr<RtMidiIn> mMidiIn = nullptr;
        std::unique_ptr<RtMidiOut> mMidiOut = nullptr;
        std::set<MidiInputPort*> mInputPorts;
        std::set<MidiInputComponentInstance*> mInputComponents;
        
        moodycamel::ConcurrentQueue<std::unique_ptr<MidiEvent>> mEventQueue;
    };
        
}

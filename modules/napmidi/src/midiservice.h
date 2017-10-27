#pragma once

// Std includes
#include <set>

// Third party includes
#include <RtMidi.h>

// Nap includes
#include <nap/service.h>

namespace nap {
    
    // Forward declarations
    class MidiInputPort;

    
    class NAPAPI MidiService : public nap::Service {
        RTTI_ENABLE(nap::Service)
        
        friend class MidiInputPort;
        
    public:
        MidiService() = default;
        
        // Initialization
        bool init(nap::utility::ErrorState& errorState);
        
        int getInputPortCount();
        std::string getInputPortName(int portNumber);
        
        int getOutputPortCount();
        std::string getOutputPortName(int portNumber);

        void printPorts();
        
        void registerInputPort(MidiInputPort& port) { mInputPorts.emplace(&port); }
        void unregisterInputPort(MidiInputPort& port) { mInputPorts.erase(&port); }

    protected:
        void registerObjectCreators(rtti::Factory& factory) override final;
        
    private:
        std::unique_ptr<RtMidiIn> mMidiIn = nullptr;
        std::unique_ptr<RtMidiOut> mMidiOut = nullptr;
        std::set<MidiInputPort*> mInputPorts;
    };
        
}

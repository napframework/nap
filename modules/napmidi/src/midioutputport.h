#pragma once

#include <rtti/rttiobject.h>
#include <utility/dllexport.h>

#include <RtMidi.h>

#include "midiservice.h"

namespace nap {
    
    class NAPAPI MidiOutputPort : public rtti::RTTIObject {
        RTTI_ENABLE(rtti::RTTIObject)
        
    public:
        MidiOutputPort() = default;
        MidiOutputPort(MidiService& service);
        virtual ~MidiOutputPort() = default;
        
        bool init(utility::ErrorState& errorState) override;
        
        MidiService& getService() { return *mService; }
        
        int mPortNumber = 0;
        
        void sendEvent(const MidiEvent& event);
        
    private:
        RtMidiOut midiOut;
        MidiService* mService = nullptr;
        std::vector<unsigned char> outputData;
    };
    
    // Object creator used for constructing the the OSC receiver
    using MidiOutputPortObjectCreator = rtti::ObjectCreator<MidiOutputPort, MidiService>;
    
}

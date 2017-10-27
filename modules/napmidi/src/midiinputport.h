#pragma once

#include <rtti/rttiobject.h>
#include <utility/dllexport.h>

#include <RtMidi.h>

#include "midiservice.h"

namespace nap {
    
    class NAPAPI MidiInputPort : public rtti::RTTIObject {
        RTTI_ENABLE(rtti::RTTIObject)
        
    public:
        MidiInputPort() = default;
        MidiInputPort(MidiService& service);
        virtual ~MidiInputPort();
        
        bool init(utility::ErrorState& errorState) override;        
        
        int mPortNumber = 0;
        bool mDebugOutput = false;
        
    private:
        RtMidiIn midiIn;
        MidiService* mService = nullptr;
    };
    
    // Object creator used for constructing the the OSC receiver
    using MidiInputPortObjectCreator = rtti::ObjectCreator<MidiInputPort, MidiService>;
    
}

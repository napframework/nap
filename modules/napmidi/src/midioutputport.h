#pragma once

#include <rtti/rttiobject.h>
#include <utility/dllexport.h>

#include <RtMidi.h>

#include "midiservice.h"

namespace nap {
    
    /**
     * Opens and manages a midi output port that midi messages can be sent to.
     */
    class NAPAPI MidiOutputPort : public rtti::RTTIObject {
        RTTI_ENABLE(rtti::RTTIObject)
        
    public:
        MidiOutputPort() = default;
        MidiOutputPort(MidiService& service);
        virtual ~MidiOutputPort() = default;
        
        bool init(utility::ErrorState& errorState) override;
        
        MidiService& getService() { return *mService; }
        
        int mPortNumber = 0; /**< The port number that midi messages will be sent through by this object */
        
        /**
         * Sends a midi event through this output port.
         */
        void sendEvent(const MidiEvent& event);
        
    private:
        RtMidiOut midiOut;
        MidiService* mService = nullptr;
        std::vector<unsigned char> outputData;
    };
    
    // Object creator used for constructing the the OSC receiver
    using MidiOutputPortObjectCreator = rtti::ObjectCreator<MidiOutputPort, MidiService>;
    
}

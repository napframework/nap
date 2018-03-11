#pragma once

#include <utility/dllexport.h>
#include <RtMidi.h>
#include <nap/resource.h>
#include "midiservice.h"

namespace nap {
    
    /**
     * Opens and manages a midi output port that midi messages can be sent to.
     */
    class NAPAPI MidiOutputPort : public Resource
    {
        RTTI_ENABLE(Resource)
    public:
        MidiOutputPort() = default;
        MidiOutputPort(MidiService& service);
        virtual ~MidiOutputPort() = default;
        
        bool init(utility::ErrorState& errorState) override;
        
        MidiService& getService() { return *mService; }
        
        std::string mPortName = 0; /**< The name of the port that midi messages will be sent through by this object */
        
        /**
         * Sends a midi event through this output port.
         */
        void sendEvent(const MidiEvent& event);
        
        /**
         * @return: the number of the midi port that messages will be sent through by this object
         */
        int getPortNumber() const { return mPortNumber; }
        
    private:
        RtMidiOut midiOut;
        MidiService* mService = nullptr;
        std::vector<unsigned char> outputData;
        int mPortNumber = -1;
    };
    
    // Object creator used for constructing the the OSC receiver
    using MidiOutputPortObjectCreator = rtti::ObjectCreator<MidiOutputPort, MidiService>;
    
}

#pragma once

#include <rtti/rttiobject.h>
#include <utility/dllexport.h>

#include <RtMidi.h>

#include "midiservice.h"

namespace nap {
    
    
    /**
     * Opens and manages a midi input port that will be listened to for incoming midi messages.
     * Messages will be parsed and passed on to the midi service for processing.
     */
    class NAPAPI MidiInputPort : public rtti::RTTIObject
    {
        RTTI_ENABLE(rtti::RTTIObject)
        
    public:
        MidiInputPort() = default;
        MidiInputPort(MidiService& service);
        virtual ~MidiInputPort();
        
        bool init(utility::ErrorState& errorState) override;
        
        /**
         * @return: the midi service that this input port is registered to
         */
        MidiService& getService() { return *mService; }
        
        std::string mPortName = ""; /**< The name of the port that will be listened to for incoming messages */
        bool mDebugOutput = false; /**< If true, incoming messages will be logged for debugging purposes */
        
        /**
         * Called internally by the midi callback.
         */
        void receiveEvent(std::unique_ptr<MidiEvent> event) { mService->enqueueEvent(std::move(event)); }
        
        /**
         * @return: The midi port number thas this object is listening to
         */
        int getPortNumber() const { return mPortNumber; }
        
    private:
        RtMidiIn midiIn;
        MidiService* mService = nullptr;
        int mPortNumber = -1;
    };
    
    // Object creator used for constructing the the OSC receiver
    using MidiInputPortObjectCreator = rtti::ObjectCreator<MidiInputPort, MidiService>;
    
}

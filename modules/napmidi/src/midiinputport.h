#pragma once

#include <rtti/rttiobject.h>
#include <utility/dllexport.h>
#include <RtMidi.h>
#include <resource.h>

#include "midiservice.h"

namespace nap
{
    
    
    /**
     * Opens and manages one or more midi input ports that will be listened to for incoming midi messages.
     * Messages will be parsed and passed on to the midi service for processing.
     */
    class NAPAPI MidiInputPort : public Resource
    {
        RTTI_ENABLE(Resource)
    public:
        MidiInputPort() = default;
        MidiInputPort(MidiService& service);
        virtual ~MidiInputPort();
        
        bool init(utility::ErrorState& errorState) override;
        
        /**
         * @return: the midi service that this input port is registered to
         */
        MidiService& getService() { return *mService; }
        
        /**
         * The name of the ports that will be listened to for incoming messages.
         * If left empty all available ports will be opened and listened to.
         */
        std::vector<std::string> mPortNames;
        
        bool mDebugOutput = false; /**< If true, incoming messages will be logged for debugging purposes */
        
        /**
         * Called internally by the midi callback.
         */
        void receiveEvent(std::unique_ptr<MidiEvent> event) { mService->enqueueEvent(std::move(event)); }
        
        /**
         * @return: The midi port number thas this object is listening to
         */
        const std::vector<int> getPortNumbers() const { return mPortNumbers; }
        
        /**
         * @return: A string summing up all ports listened to separated by ,
         */
        std::string getPortNames() const;
        
    private:
        std::vector<std::unique_ptr<RtMidiIn>> midiIns;
        MidiService* mService = nullptr;
        std::vector<int> mPortNumbers;
    };
    
    // Object creator used for constructing the the OSC receiver
    using MidiInputPortObjectCreator = rtti::ObjectCreator<MidiInputPort, MidiService>;
    
}

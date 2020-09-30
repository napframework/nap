#pragma once

#include "midiservice.h"

#include <rtti/object.h>
#include <utility/dllexport.h>
#include <nap/device.h>

// Forward Declares
class RtMidiIn;

namespace nap
{
    /**
     * Opens and manages one or more midi input ports that are monitored for incoming midi messages.
     * Messages will be parsed and passed on to the midi service for processing.
	 * When no 'Ports' are specified all ports are opened.
     */
    class NAPAPI MidiInputPort : public Device
    {
        RTTI_ENABLE(Device)
    public:
		MidiInputPort() = default;
        MidiInputPort(MidiService& service);

        /**
         * Starts the midi input port.
		 * @param errorState contains the error if the port could not be opened.
		 * @return if the input port opened successfully.
         */
        virtual bool start(utility::ErrorState& errorState) override;

        /**
         * Stops the midi input port.
         */
		virtual void stop() override;
        
        /**
         * @return: the midi service that this input port is registered to
         */
        MidiService& getService();
        
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
        
		/**
		 * The name of the ports that will be listened to for incoming messages.
		 * If left empty all available ports will be opened and listened to.
		 */
		std::vector<std::string> mPortNames;		///< Property: 'Ports' names of ports this object opens and listens to. When left empty all ports are opened.
		bool mDebugOutput = false;					///< Property: 'EnableDebugOutput' If true, incoming messages will be logged for debugging purposes

    private:
        std::vector<std::unique_ptr<RtMidiIn>> mMidiIns;
        MidiService* mService = nullptr;
        std::vector<int> mPortNumbers;
    };
    
    // Object creator used for constructing the the OSC receiver
    using MidiInputPortObjectCreator = rtti::ObjectCreator<MidiInputPort, MidiService>;
    
}

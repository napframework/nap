#include "midioutputport.h"

#include "midievent.h"

#include <nap/logger.h>

RTTI_BEGIN_CLASS(nap::MidiOutputPort)
    RTTI_PROPERTY("Port", &nap::MidiOutputPort::mPortName, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

namespace nap
{
    
    
    MidiOutputPort::MidiOutputPort(MidiService& service) : 
		mService(&service)
    {
    }
    
    
    bool MidiOutputPort::start(utility::ErrorState& errorState)
    {
        try 
		{
            mPortNumber = mService->getOutputPortNumber(mPortName);
            if (mPortNumber < 0)
            {
                errorState.fail("Midi output port not found: " + mPortName);
                return false;
            }
            midiOut.openPort(mPortNumber);
            return true;
        }
        catch(RtMidiError& error) {
            errorState.fail(error.getMessage());
            return false;
        }
    }
    

	void MidiOutputPort::stop()
	{
		midiOut.closePort();
	}
    
    void MidiOutputPort::sendEvent(const MidiEvent& event)
    {
        outputData = event.getData();
        try {
            midiOut.sendMessage(&outputData);
        }
        catch(RtMidiError& error) {
            nap::Logger::fatal(error.getMessage());
        }
    }

    
}

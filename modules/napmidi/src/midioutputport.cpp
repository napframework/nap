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
            mMidiOut.openPort(mPortNumber);
            return true;
        }
        catch(RtMidiError& error) 
		{
            errorState.fail(error.getMessage());
            return false;
        }
    }
    

	void MidiOutputPort::stop()
	{
		mMidiOut.closePort();
	}
    

	nap::MidiService& MidiOutputPort::getService()
	{
		return *mService;
	}


	void MidiOutputPort::sendEvent(const MidiEvent& event)
    {
        mOutputData = event.getData();
        try 
		{
            mMidiOut.sendMessage(&mOutputData);
        }
        catch(RtMidiError& error) 
		{
            nap::Logger::fatal(error.getMessage());
        }
    }

    
	int MidiOutputPort::getPortNumber() const
	{
		return mPortNumber;
	}

}

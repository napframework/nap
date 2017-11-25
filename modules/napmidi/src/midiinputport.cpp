#include "midiinputport.h"

#include "midievent.h"

#include <nap/logger.h>

RTTI_BEGIN_CLASS(nap::MidiInputPort)
    RTTI_PROPERTY("Port", &nap::MidiInputPort::mPortName, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("EnableDebugOutput", &nap::MidiInputPort::mDebugOutput, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
    
    
    void midiCallback(double deltatime, std::vector<unsigned char> *message, void *userData)
    {
        auto inputPort = static_cast<MidiInputPort*>(userData);
        auto event = std::make_unique<MidiEvent>(*message, inputPort->mID);
        if (inputPort->mDebugOutput)
            nap::Logger::info("midi input on " + inputPort->mPortName + ": " + event->toString());
        inputPort->receiveEvent(std::move(event));
    }
    
    
    MidiInputPort::MidiInputPort(MidiService& service) : rtti::RTTIObject(), mService(&service)
    {

    }
    
    
    MidiInputPort::~MidiInputPort()
    {
        mService->unregisterInputPort(*this);
    }
    
    
    bool MidiInputPort::init(utility::ErrorState& errorState)
    {
        try {
            mPortNumber = mService->getInputPortNumber(mPortName);
            if (mPortNumber < 0)
            {
                errorState.fail("Midi input port not found: " + mPortName);
                return false;
            }
            midiIn.openPort(mPortNumber);
            midiIn.setCallback(&midiCallback, this);
            mService->registerInputPort(*this);
            return true;
        }
        catch(RtMidiError& error) {
            errorState.fail(error.getMessage());
            return false;
        }
    }    
    
}

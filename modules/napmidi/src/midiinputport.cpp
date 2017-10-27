#include "midiinputport.h"

RTTI_BEGIN_CLASS(nap::MidiInputPort)
    RTTI_PROPERTY("Port", &nap::MidiInputPort::mPortNumber, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("EnableDebugOutput", &nap::MidiInputPort::mDebugOutput, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap {
    
    void midiCallback(double deltatime, std::vector<unsigned char> *message, void *userData)
    {
        unsigned int nBytes = message->size();
        for ( unsigned int i=0; i<nBytes; i++ )
            std::cout << "Byte " << i << " = " << (int)message->at(i) << ", ";
        if ( nBytes > 0 )
            std::cout << "stamp = " << deltatime << std::endl;
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

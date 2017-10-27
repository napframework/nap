#include "midiservice.h"

#include <nap/logger.h>

#include "midiinputport.h"

RTTI_DEFINE(nap::MidiService)

namespace nap {
    
    bool MidiService::init(nap::utility::ErrorState& errorState)
    {
        try {
            mMidiIn = std::make_unique<RtMidiIn>();
            mMidiOut = std::make_unique<RtMidiOut>();
            printPorts();
            return true;
        }
        catch (RtMidiError& error)
        {
            errorState.fail(error.getMessage());
            return false;
        }
    }
    
    
    void MidiService::registerObjectCreators(rtti::Factory& factory)
    {
        factory.addObjectCreator(std::make_unique<MidiInputPortObjectCreator>(*this));
    }

    
    int MidiService::getInputPortCount()
    {
        return mMidiIn->getPortCount();
    }
    
    
    std::string MidiService::getInputPortName(int portNumber)
    {
        return mMidiIn->getPortName(portNumber);
    }
    
    
    int MidiService::getOutputPortCount()
    {
        return mMidiOut->getPortCount();
    }
    
    
    std::string MidiService::getOutputPortName(int portNumber)
    {
        return mMidiOut->getPortName(portNumber);
    }
    
    
    void MidiService::printPorts()
    {
        Logger::info("Available midi input ports:");
        for (auto i = 0; i < getInputPortCount(); ++i)
            nap::Logger::info("%i: %s", i, getInputPortName(i).c_str());

        Logger::info("Available midi output ports:");
        for (auto i = 0; i < getOutputPortCount(); ++i)
            nap::Logger::info("%i: %s", i, getOutputPortName(i).c_str());
    }

    
}

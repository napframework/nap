#include "midiinputport.h"

#include "midievent.h"

#include <nap/logger.h>

RTTI_BEGIN_CLASS(nap::MidiInputPort)
    RTTI_PROPERTY("Ports", &nap::MidiInputPort::mPortNames, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("EnableDebugOutput", &nap::MidiInputPort::mDebugOutput, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
    
    void midiCallback(double deltatime, std::vector<unsigned char> *message, void *userData)
    {
        auto inputPort = static_cast<MidiInputPort*>(userData);
        auto portNames = inputPort->getPortNames();
        auto event = std::make_unique<MidiEvent>(*message, inputPort->mID);
        if (inputPort->mDebugOutput)
            nap::Logger::info("midi input on " + portNames + ": " + event->toString());
        inputPort->receiveEvent(std::move(event));
    }
    
    
    MidiInputPort::MidiInputPort(MidiService& service) : Resource(), mService(&service)
    {

    }
    
    
    MidiInputPort::~MidiInputPort()
    {
        mService->unregisterInputPort(*this);
    }
    
    
    bool MidiInputPort::init(utility::ErrorState& errorState)
    {
        try {
            if (mPortNames.empty())
            {
                for (auto portNumber = 0; portNumber < mService->getInputPortCount(); ++portNumber)
                {
                    mPortNumbers.emplace_back(portNumber);
                    mPortNames.emplace_back(mService->getInputPortName(portNumber));
                }
            }
            else {
                for (auto& portName : mPortNames)
                {
                    auto portNumber = mService->getInputPortNumber(portName);
                    if (portNumber < 0)
                    {
                        errorState.fail("Midi input port not found: " + portName);
                        return false;
                    }
                    mPortNumbers.emplace_back(portNumber);
                }
            }
                
            for (auto& portName : mPortNames)
            {
                auto portNumber = mService->getInputPortNumber(portName);
                if (portNumber < 0)
                {
                    errorState.fail("Midi input port not found: " + portName);
                    return false;
                }
                mPortNumbers.emplace_back(portNumber);
                auto midiIn = std::make_unique<RtMidiIn>();
                midiIn->openPort(portNumber);
                midiIn->setCallback(&midiCallback, this);
                midiIns.emplace_back(std::move(midiIn));
            }
            
            mService->registerInputPort(*this);
            return true;
        }
        catch(RtMidiError& error) {
            errorState.fail(error.getMessage());
            return false;
        }
    }
    
    
    std::string MidiInputPort::getPortNames() const
    {
        std::string result;
        for (auto& portName : mPortNames)
        {
            result.append(portName);
            if (portName != mPortNames.back())
                result.append(", ");
        }
        return result;
    }

    
}

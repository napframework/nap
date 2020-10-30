/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "midioutputport.h"
#include "midievent.h"

// External Includes
#include <nap/logger.h>
#include <RtMidi.h>

RTTI_BEGIN_CLASS(nap::MidiOutputPort)
	RTTI_PROPERTY("AllowFailure",	&nap::MidiOutputPort::mAllowFailure,	nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Port",			&nap::MidiOutputPort::mPortName,		nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

namespace nap
{
    
    
    MidiOutputPort::MidiOutputPort(MidiService& service) : mService(&service)
    {
		mMidiOut = std::make_unique<RtMidiOut>();
    }


	MidiOutputPort::~MidiOutputPort()
	{
		mMidiOut.reset(nullptr);
	}


	bool MidiOutputPort::start(utility::ErrorState& errorState)
    {
		// Find and try to open port
		mPortNumber = mService->getOutputPortNumber(mPortName);
		if (mPortNumber >= 0)
		{
			try
			{
				mMidiOut->openPort(mPortNumber);
			}
			catch (RtMidiError& error)
			{
				if (!mAllowFailure)
				{
					errorState.fail("%s: %s", mID.c_str(), error.getMessage().c_str());
					return false;
				}
				nap::Logger::error("%s: %s", mID.c_str(), error.getMessage().c_str());
			}
		}
		else
		{
			std::string error_msg = utility::stringFormat("%s: Midi output port not found: %s", mID.c_str(), mPortName.c_str());
			if (!mAllowFailure)
			{
				errorState.fail(error_msg);
				return false;
			}
			nap::Logger::error(error_msg);
		}
		
		return true;
    }
    

	void MidiOutputPort::stop()
	{
		if (mMidiOut->isPortOpen())
		{
			mMidiOut->closePort();
		}
	}
    

	nap::MidiService& MidiOutputPort::getService()
	{
		return *mService;
	}


	void MidiOutputPort::sendEvent(const MidiEvent& event)
    {
		// Skip when port is not open
		if (!mMidiOut->isPortOpen())
			return;

        mOutputData = event.getData();
        try 
		{
            mMidiOut->sendMessage(&mOutputData);
        }
        catch(RtMidiError& error) 
		{
            nap::Logger::error("%s: %s", mID.c_str(), error.getMessage().c_str());
        }
    }

    
	int MidiOutputPort::getPortNumber() const
	{
		return mPortNumber;
	}

}

#pragma once

// Local Includes
#include "oscevent.h"

// External Includes
#include <osc/OscPacketListener.h>
#include <queue>
#include <mutex>

namespace nap
{
	class OSCReceiver;

	/**
	 *	Listens to incoming packages and translates those in to OSC events
	 */
	class NAPAPI OSCPacketListener : public osc::OscPacketListener
	{
		friend class OSCReceiver;
	public:
		OSCPacketListener(OSCReceiver& receiver);

	protected:
		virtual void ProcessMessage(const osc::ReceivedMessage& message, const IpEndpointName& remoteEndpoint) override;

	private:
		OSCReceiver& mReceiver;				// Receiver that holds the message queue
	};
}

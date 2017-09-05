#pragma once

#include <osc/OscPacketListener.h>

namespace nap
{
	/**
	 *	Listens to incoming packages and translates those in to OSC events
	 */
	class OSCPacketListener : public osc::OscPacketListener
	{
	protected:
		virtual void ProcessMessage(const osc::ReceivedMessage& m, const IpEndpointName& remoteEndpoint) override;
	};
}

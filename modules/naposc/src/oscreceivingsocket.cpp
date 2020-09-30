#include "oscreceivingsocket.h"

namespace nap
{

	OSCReceivingSocket::OSCReceivingSocket(const IpEndpointName& localEndpoint, bool allowReuse)
	{
		SetAllowReuse(allowReuse);
		Bind(localEndpoint);
	}


	OSCReceivingSocket::OSCReceivingSocket(int port, bool allowReuse)
	{
		SetAllowReuse(allowReuse);
		Bind(IpEndpointName(IpEndpointName::ANY_ADDRESS, port));
	}


	OSCReceivingSocket::~OSCReceivingSocket()
	{
		mMultiplexer.DetachSocketListener(this, mListener);
	}


	void OSCReceivingSocket::setListener(osc::OscPacketListener* listener)
	{
		mListener = listener; mMultiplexer.AttachSocketListener(this, mListener);
	}


	void OSCReceivingSocket::run()
	{
		mMultiplexer.RunUntilSigInt();
	}


	void OSCReceivingSocket::stop()
	{
		mMultiplexer.AsynchronousBreak();
	}

}
#include "oscreceivingsocket.h"

namespace nap
{

	OSCReceivingSocket::OSCReceivingSocket(const IpEndpointName& localEndpoint)
	{
		Bind(localEndpoint);
	}


	OSCReceivingSocket::OSCReceivingSocket(int port)
	{
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


	void OSCReceivingSocket::asynchronousBreak()
	{
		mMultiplexer.AsynchronousBreak();
	}

}
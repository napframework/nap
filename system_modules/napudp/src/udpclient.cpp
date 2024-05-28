/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "udpclient.h"
#include "udppacket.h"
#include "udpthread.h"

// External includes
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>
#include <asio/io_service.hpp>
#include <asio/system_error.hpp>
#include <nap/logger.h>
#include <thread>

RTTI_BEGIN_CLASS(nap::UDPClient, "Sends UDP Packets to an endpoint")
	RTTI_PROPERTY("Broadcast",					&nap::UDPClient::mBroadcast,					nap::rtti::EPropertyMetaData::Default,	"Transmit to all connected hosts")
	RTTI_PROPERTY("Endpoint",                   &nap::UDPClient::mEndpoint,                     nap::rtti::EPropertyMetaData::Default,	"The ip address to bind to")
	RTTI_PROPERTY("Port",						&nap::UDPClient::mPort,							nap::rtti::EPropertyMetaData::Default,	"The port to bind to")
	RTTI_PROPERTY("MaxQueueSize",				&nap::UDPClient::mMaxPacketQueueSize,			nap::rtti::EPropertyMetaData::Default,	"Maximum packet queue size")
	RTTI_PROPERTY("StopOnMaxQueueSizeExceeded", &nap::UDPClient::mStopOnMaxQueueSizeExceeded,	nap::rtti::EPropertyMetaData::Default,	"Do not add packet when max queue size is exceeded")
RTTI_END_CLASS

using asio::ip::address;
using asio::ip::udp;

namespace nap
{
    //////////////////////////////////////////////////////////////////////////
    // UDPClientASIO
    //////////////////////////////////////////////////////////////////////////

    class UDPClient::Impl
    {
    public:
        Impl(asio::io_context& context) : mIOContext(context){}

        // ASIO
        asio::io_context& 			mIOContext;
        asio::ip::udp::endpoint 	mRemoteEndpoint;
        asio::ip::udp::socket       mSocket{ mIOContext };
    };

	//////////////////////////////////////////////////////////////////////////
	// UDPClient
	//////////////////////////////////////////////////////////////////////////

    UDPClient::UDPClient() : UDPAdapter()
    {}


    UDPClient::~UDPClient()
    {}


	bool UDPClient::onStart(utility::ErrorState& errorState)
	{
        // create asio implementation
        mImpl = std::make_unique<UDPClient::Impl>(getIOContext());

        // when asio error occurs, init_success indicates whether initialization should fail or succeed
        bool init_success = false;

		// try to open socket
		asio::error_code asio_error_code;
        mImpl->mSocket.open(udp::v4(), asio_error_code);
        if(handleAsioError(asio_error_code, errorState, init_success))
            return init_success;

        // enable/disable broadcast
        mImpl->mSocket.set_option(asio::socket_base::broadcast(mBroadcast), asio_error_code);
        if(handleAsioError(asio_error_code, errorState, init_success))
            return init_success;
        
        // resolve ip address from endpoint
        asio::ip::tcp::resolver resolver(getIOContext());
        asio::ip::tcp::resolver::query query(mEndpoint, "80");
        asio::ip::tcp::resolver::iterator iter = resolver.resolve(query, asio_error_code);
        if(handleAsioError(asio_error_code, errorState, init_success))
            return init_success;

        asio::ip::tcp::endpoint endpoint = iter->endpoint();
        auto address = address::from_string(endpoint.address().to_string(), asio_error_code);
        if(handleAsioError(asio_error_code, errorState, init_success))
            return init_success;

        mImpl->mRemoteEndpoint = udp::endpoint(address, mPort);

		return true;
	}


	void UDPClient::onStop()
	{
		asio::error_code err;
        mImpl->mSocket.close(err);
		if (err)
		{
			nap::Logger::error(*this, "error closing socket : %s", err.message().c_str());
		}

        // explicitly delete socket 
        mImpl = nullptr;
	}


	void UDPClient::send(const UDPPacket& packet)
	{
		if(!mStopOnMaxQueueSizeExceeded)
		{
			mQueue.enqueue(packet);
		}
		else
		{
			if(mQueue.size_approx() < mMaxPacketQueueSize)
			{
				mQueue.enqueue(packet);
			}
			else
			{
				nap::Logger::error(*this, "max queue size exceeded, dropping packet");
			}
		}
	}


	void UDPClient::send(UDPPacket&& packet)
	{
		if(!mStopOnMaxQueueSizeExceeded)
		{
			mQueue.enqueue(std::move(packet));
		}
		else
		{
			if(mQueue.size_approx() < mMaxPacketQueueSize)
			{
				mQueue.enqueue(std::move(packet));
			}
			else
			{
				nap::Logger::error(*this, "max queue size exceeded, dropping packet");
			}
		}
	}


	void UDPClient::onProcess()
	{
		// let the socket send queued packets
		UDPPacket packet_to_send;
		while(mQueue.try_dequeue(packet_to_send))
		{
			asio::error_code err;
            mImpl->mSocket.send_to(asio::buffer(&packet_to_send.data()[0], packet_to_send.size()), mImpl->mRemoteEndpoint, 0, err);

			if(err)
			{
				nap::Logger::error(*this, "error sending packet : %s", err.message().c_str());
			}
		}
	}
}

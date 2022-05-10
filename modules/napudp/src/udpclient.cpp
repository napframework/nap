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

using asio::ip::address;
using asio::ip::udp;

RTTI_BEGIN_CLASS(nap::UDPClient)
	RTTI_PROPERTY("Endpoint",					&nap::UDPClient::mRemoteIp,						nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Port",						&nap::UDPClient::mPort,							nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("MaxQueueSize",				&nap::UDPClient::mMaxPacketQueueSize,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("StopOnMaxQueueSizeExceeded", &nap::UDPClient::mStopOnMaxQueueSizeExceeded,	nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// UDPClient
	//////////////////////////////////////////////////////////////////////////

	bool UDPClient::init(utility::ErrorState& errorState)
	{
        // when asio error occurs, init_success indicates whether initialization should fail or succeed
        bool init_success = false;

		// try to open socket
		asio::error_code asio_error_code;
		mSocket.open(udp::v4(), asio_error_code);
        if(handleAsioError(asio_error_code, errorState, init_success))
            return init_success;

        // create address from string
        auto address = address::from_string(mRemoteIp, asio_error_code);
        if(handleAsioError(asio_error_code, errorState, init_success))
            return init_success;

        mRemoteEndpoint = udp::endpoint(address, mPort);

		// init UDPAdapter, registering the client to an UDPThread
		if (!UDPAdapter::init(errorState))
			return false;

		return true;
	}


	void UDPClient::onDestroy()
	{
		UDPAdapter::onDestroy();

		asio::error_code err;
		mSocket.close(err);
		if (err)
		{
			nap::Logger::error(*this, "error closing socket : %s", err.message().c_str());
		}
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


	void UDPClient::process()
	{
		// let the socket send queued packets
		UDPPacket packet_to_send;
		while(mQueue.try_dequeue(packet_to_send))
		{
			asio::error_code err;
			mSocket.send_to(asio::buffer(&packet_to_send.data()[0], packet_to_send.size()), mRemoteEndpoint, 0, err);

			if(err)
			{
				nap::Logger::error(*this, "error sending packet : %s", err.message().c_str());
			}
		}
	}
}

/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "udpclient.h"
#include "udppacket.h"

// External includes
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>
#include <asio/io_service.hpp>
#include <asio/system_error.hpp>
#include <nap/logger.h>

#include <thread>

using asio::ip::address;
using asio::ip::udp;

RTTI_BEGIN_CLASS(nap::UdpClient)
		RTTI_PROPERTY("Endpoint", &nap::UdpClient::mRemoteIp, nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("Port", &nap::UdpClient::mPort, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
	bool UdpClient::start(utility::ErrorState& errorState)
	{
		mRun = true;

		// try to open socket
		asio::error_code asio_error_code;
		mSocket.open(udp::v4(), asio_error_code);

		if( asio_error_code )
		{
			errorState.fail(asio_error_code.message());

			if(mThrowOnInitError)
			{
				mRun = false;
				return false;
			}
			else
				nap::Logger::warn(*this, asio_error_code.message());
		}else
		{
			mRemoteEndpoint = udp::endpoint(address::from_string(mRemoteIp), mPort);

			// fire up thread
			mSendThread = std::thread(std::bind(&UdpClient::sendThread, this));
		}

		return true;
	}


	void UdpClient::stop()
	{
		mRun = false;
		mSocket.close();
		mSendThread.join();
	}


	void UdpClient::send(const UdpPacket& packet)
	{
		std::lock_guard<std::mutex> scoped_lock(mMutex);
		mQueue.emplace_back(packet);
	}


	void UdpClient::sendThread()
	{
		while(mRun)
		{
			// consume any queued packets
			std::vector<UdpPacket> packets_to_send;
			{
				std::lock_guard<std::mutex> scoped_lock(mMutex);
				packets_to_send.swap(mQueue);
			}

			// let the socket send the packets
			for(const auto& packet : packets_to_send)
			{
				asio::error_code err;
				mSocket.send_to(asio::buffer(&packet.data()[0], packet.size()), mRemoteEndpoint, 0, err);

				if(err)
				{
					nap::Logger::warn(*this, "error sending packet : %s", err.message().c_str());
				}
			}
		}
	}
}
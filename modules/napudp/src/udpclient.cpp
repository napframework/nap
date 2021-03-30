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
	RTTI_PROPERTY("MaxQueueSize", &nap::UdpClient::mMaxPacketQueueSize, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("StopOnMaxQueueSizeExceeded", &nap::UdpClient::mStopOnMaxQueueSizeExceeded, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
	bool UdpClient::start(utility::ErrorState& errorState)
	{
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
			{
				nap::Logger::warn(*this, asio_error_code.message());
			}
		}else
		{
			mRemoteEndpoint = udp::endpoint(address::from_string(mRemoteIp), mPort);

			// fire up thread
			mRun.store(true);
			mSendThread = std::thread([this] { sendThread(); });
		}

		return true;
	}


	void UdpClient::stop()
	{
		if( mRun.load() )
		{
			mRun.store(false);
			mSendThread.join();

			asio::error_code err;
			mSocket.close(err);
			if(err)
				nap::Logger::warn(*this, "error closing socket : %s", err.message().c_str());
		}
	}


	void UdpClient::onDestroy()
	{
		stop();
	}


	void UdpClient::send(const UdpPacket& packet)
	{
		mQueue.enqueue(packet);
	}


	void UdpClient::sendThread()
	{
		while(mRun.load())
		{
			// let the socket send the packets
			UdpPacket packet_to_send;
			while(mQueue.try_dequeue(packet_to_send))
			{
				asio::error_code err;
				mSocket.send_to(asio::buffer(&packet_to_send.data()[0], packet_to_send.size()), mRemoteEndpoint, 0, err);

				if(err)
				{
					nap::Logger::warn(*this, "error sending packet : %s", err.message().c_str());
				}

				// do we need to stop if we have a maximum number of packets we can have in the queue?
				if( mStopOnMaxQueueSizeExceeded )
				{
					// yes, check the packet queue
					if( mQueue.size_approx() > mMaxPacketQueueSize )
					{
						// too large, close socket because of error
						std::string error = "Max queue size exceeded, closing socket";
						nap::Logger::error(*this, error);
						mSocket.close(err);

						if(err)
							nap::Logger::warn(*this, "error closing socket : %s", err.message().c_str());

						// store error information
						mHasError.store(true);
						mError = error;

						// exit threaded function
						return;
					}
				}
			}
		}
	}
}
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "udpserver.h"
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

RTTI_BEGIN_CLASS(nap::UDPServer)
	RTTI_PROPERTY("AllowFailure", &nap::UDPServer::mAllowFailure, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Endpoint", &nap::UDPServer::mIPRemoteEndpoint, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Port", &nap::UDPServer::mPort, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("BufferSize", &nap::UDPServer::mBufferSize, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// UDPServer
	//////////////////////////////////////////////////////////////////////////

	bool UDPServer::init(utility::ErrorState& errorState)
	{
		if(!UDPAdapter::init(errorState))
			return false;

		mBuffer.resize(mBufferSize);

		// try to open socket
		asio::error_code asio_error_code;
		mSocket.open(udp::v4(), asio_error_code);
		if(asio_error_code)
		{
			if(!mAllowFailure)
			{
				errorState.fail(asio_error_code.message());
				return false;
			}
			else
			{
				nap::Logger::error(*this, asio_error_code.message());
			}
		}
		else
		{
			// try to bind socket
			nap::Logger::info(*this, "Listening at port %i", mPort);
			mSocket.bind(udp::endpoint(address::from_string(mIPRemoteEndpoint), mPort), asio_error_code);
			if( asio_error_code )
			{
				if (!mAllowFailure)
				{
					errorState.fail(asio_error_code.message());
					return false;
				}
				else
				{
					nap::Logger::error(*this, asio_error_code.message());
				}
			}
		}
		return true;
	}


	void UDPServer::onDestroy()
	{
		UDPAdapter::onDestroy();
		mSocket.close();
	}


	void UDPServer::process()
	{
		asio::error_code asio_error;
		if(mSocket.available(asio_error) > 0)
		{
			mSocket.receive_from(asio::buffer(mBuffer), mRemoteEndpoint, 0,asio_error);

			// construct udp packet, clears current buffer
			std::vector<nap::uint8> buffer;
			buffer.resize(mBufferSize);
			buffer.swap(mBuffer);

			// forward buffer to any listeners
			UDPPacket packet(std::move(buffer));
			packetReceived.trigger(packet);
		}

		if(asio_error)
		{
			nap::Logger::warn(*this, asio_error.message());
		}
	}
}

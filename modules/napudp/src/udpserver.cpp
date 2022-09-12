/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "udpserver.h"
#include "udppacket.h"
#include "udpthread.h"

// ASIO Includes
#include <asio/ip/udp.hpp>
#include <asio/io_service.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>

// External includes
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>
#include <asio/io_service.hpp>
#include <asio/system_error.hpp>
#include <nap/logger.h>

#include <thread>

RTTI_BEGIN_CLASS(nap::UDPServer)
	RTTI_PROPERTY("Port",			&nap::UDPServer::mPort,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("IP Address",		&nap::UDPServer::mIPAddress,	nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

using namespace asio::ip;

namespace nap
{
    //////////////////////////////////////////////////////////////////////////
    // UDPServerASIO
    //////////////////////////////////////////////////////////////////////////

    class UDPServerASIO
    {
    public:
        // ASIO
        asio::io_context 			mIOService;
        asio::ip::udp::endpoint 	mRemoteEndpoint;
        asio::ip::udp::socket       mSocket{ mIOService };
    };

	//////////////////////////////////////////////////////////////////////////
	// UDPServer
	//////////////////////////////////////////////////////////////////////////

    UDPServer::UDPServer() : UDPAdapter()
    {}


    UDPServer::~UDPServer()
    {}


	bool UDPServer::init(utility::ErrorState& errorState)
	{
        // create asio implementation
        mASIO = std::make_unique<UDPServerASIO>();

        // when asio error occurs, init_success indicates whether initialization should fail or succeed
        bool init_success = false;

        // try to open socket
		asio::error_code asio_error_code;
        mASIO->mSocket.open(udp::v4(), asio_error_code);
        if(handleAsioError(asio_error_code, errorState, init_success))
            return init_success;

		// try to create ip address
		// when address property is left empty, bind to any local address
		asio::ip::address address;
		if (mIPAddress.empty())
		{
			address = asio::ip::address_v4::any();
		}
		else
		{
			address = asio::ip::make_address(mIPAddress, asio_error_code);
			if (handleAsioError(asio_error_code, errorState, init_success))
				return init_success;
		}

        // try to bind socket
        nap::Logger::info(*this, "Listening at port %i", mPort);
        mASIO->mSocket.bind(udp::endpoint(address, mPort), asio_error_code);
        if(handleAsioError(asio_error_code, errorState, init_success))
            return init_success;

		// init UDPAdapter, registering the server to an UDPThread
		if (!UDPAdapter::init(errorState))
			return false;

		return true;
	}


	void UDPServer::onDestroy()
	{
		UDPAdapter::onDestroy();

        asio::error_code asio_error_code;
        mASIO->mSocket.close(asio_error_code);

        if(asio_error_code)
        {
            nap::Logger::error(*this, asio_error_code.message());
        }
	}


	void UDPServer::process()
	{
		asio::error_code asio_error;
		size_t available_bytes = mASIO->mSocket.available(asio_error);
		if(available_bytes > 0)
		{
			// fill buffer
			std::vector<uint8> buffer;
			buffer.resize(available_bytes);
            mASIO->mSocket.receive(asio::buffer(buffer), 0, asio_error);

			if (!asio_error)
			{
				// make UDPPacket and forward packet to any listeners
				UDPPacket packet(std::move(buffer));
				packetReceived.trigger(packet);
			}
		}

		if(asio_error)
		{
			nap::Logger::error(*this, asio_error.message());
		}
	}
}

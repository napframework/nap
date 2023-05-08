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
	RTTI_PROPERTY("Port",			        &nap::UDPServer::mPort,			                nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("IP Address",		        &nap::UDPServer::mIPAddress,	                nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Multicast Groups",		&nap::UDPServer::mMulticastGroups,	            nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

using namespace asio::ip;

namespace nap
{
    //////////////////////////////////////////////////////////////////////////
    // UDPServerASIO
    //////////////////////////////////////////////////////////////////////////

    class UDPServer::Impl
    {
    public:
        Impl(asio::io_context& service) : mIOContext(service){}

        // ASIO
        asio::io_context& 			mIOContext;
        asio::ip::udp::endpoint 	mRemoteEndpoint;
        asio::ip::udp::socket       mSocket{ mIOContext };
    };

	//////////////////////////////////////////////////////////////////////////
	// UDPServer
	//////////////////////////////////////////////////////////////////////////

    UDPServer::UDPServer() : UDPAdapter()
    {}


    UDPServer::~UDPServer()
    {}


	bool UDPServer::onStart(utility::ErrorState& errorState)
	{
        // create asio implementation
        mImpl = std::make_unique<UDPServer::Impl>(getIOContext());

        // when asio error occurs, init_success indicates whether initialization should fail or succeed
        bool init_success = false;

        // try to open socket
		asio::error_code asio_error_code;
        mImpl->mSocket.open(udp::v4(), asio_error_code);
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
        mImpl->mSocket.bind(udp::endpoint(address, mPort), asio_error_code);
        if(handleAsioError(asio_error_code, errorState, init_success))
            return init_success;

        // join multicast groups
        for(const auto& multicast_group : mMulticastGroups)
        {
            auto multicast_address = make_address(multicast_group, asio_error_code);
            if (handleAsioError(asio_error_code, errorState, init_success))
                return init_success;

            mImpl->mSocket.set_option(multicast::join_group(multicast_address), asio_error_code);
            if (handleAsioError(asio_error_code, errorState, init_success))
                return init_success;
        }

		// init UDPAdapter, registering the server to an UDPThread
		if (!UDPAdapter::init(errorState))
			return false;

		return true;
	}


	void UDPServer::onStop()
	{
		UDPAdapter::onDestroy();

        asio::error_code asio_error_code;
        mImpl->mSocket.close(asio_error_code);

        if(asio_error_code)
        {
            nap::Logger::error(*this, asio_error_code.message());
        }
	}


	void UDPServer::onProcess()
	{
		asio::error_code asio_error;
		size_t available_bytes = mImpl->mSocket.available(asio_error);
		if(available_bytes > 0)
		{
			// fill buffer
			std::vector<uint8> buffer;
			buffer.resize(available_bytes);
            mImpl->mSocket.receive(asio::buffer(buffer), 0, asio_error);

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

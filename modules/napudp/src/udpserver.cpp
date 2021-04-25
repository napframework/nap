/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "udpserver.h"
#include "udppacket.h"
#include "udpserverlistener.h"

// External includes
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>
#include <asio/io_service.hpp>
#include <asio/system_error.hpp>
#include <nap/logger.h>

#include <thread>

using asio::ip::address;
using asio::ip::udp;

RTTI_BEGIN_CLASS(nap::UdpServer)
	RTTI_PROPERTY("Endpoint", &nap::UdpServer::mIPRemoteEndpoint, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Port", &nap::UdpServer::mPort, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("BufferSize", &nap::UdpServer::mBufferSize, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ThrowOnFailure", &nap::UdpServer::mThrowOnInitError, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
	bool UdpServer::init(utility::ErrorState& errorState)
	{
		mRun.store(true);
		mBuffer.resize(mBufferSize);

		// try to open socket
		asio::error_code asio_error_code;
		mSocket.open(udp::v4(), asio_error_code);

		if( asio_error_code )
		{
			errorState.fail(asio_error_code.message());
			mRun.store(false);

			if(mThrowOnInitError)
			{
				return false;
			}
			else
			{
				nap::Logger::warn(*this, asio_error_code.message());
			}
		}else
		{
			// try to bind socket
			nap::Logger::info(*this, "Listening at port %i", mPort);
			mSocket.bind(udp::endpoint(address::from_string(mIPRemoteEndpoint), mPort), asio_error_code);
			if( asio_error_code )
			{
				errorState.fail(asio_error_code.message());

				mRun.store(false);
				if(mThrowOnInitError)
					return false;
				else
					nap::Logger::warn(*this, asio_error_code.message());
			}
		}

		return true;
	}

	void UdpServer::onDestroy()
	{
		if(mRun.load())
		{
			mRun.store(false);
			mSocket.close();
		}
	}

	void UdpServer::process()
	{
		if(mRun.load())
		{
			// excecute all tasks
			std::function<void()> queued_task;
			while(mTaskQueue.try_dequeue(queued_task))
			{
				queued_task();
			}

			//
			size_t bytes_received = mSocket.receive_from(asio::buffer(mBuffer), mRemoteEndpoint);

			if(bytes_received > 0)
			{
				// construct udp packet, clears current buffer
				std::vector<nap::uint8> buffer;
				buffer.resize(mBufferSize);
				buffer.swap(mBuffer);

				// forward buffer to any listeners
				UdpPacket packet(std::move(buffer));
				for(auto* listener : mListeners)
				{
					listener->onUdpPacket(packet);
				}
			}
		}
	}


	void UdpServer::registerListener(UdpServerListener * listener)
	{
		enqueueTask([this, listener]()
		{
			auto it = std::find_if(mListeners.begin(), mListeners.end(), [listener](auto& a) { return a == listener; });

			assert(it == mListeners.end()); // listener already registered

			if (it == mListeners.end())
			{
				mListeners.emplace_back(listener);
			}
		});
	}


	void UdpServer::removeListener(UdpServerListener * listener)
	{
		enqueueTask([this, listener]()
		{
			auto it = std::find_if(mListeners.begin(), mListeners.end(), [listener](auto& a)
			{
				return a == listener;
			});

			assert(it != mListeners.end()); // listener not registered

			if(it != mListeners.end())
			{
			  	mListeners.erase(it);
			}
		});
	}


	void UdpServer::enqueueTask(std::function<void()> task)
	{
		mTaskQueue.enqueue(task);
	}
}
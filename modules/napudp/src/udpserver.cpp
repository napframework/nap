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
	bool UdpServer::start(utility::ErrorState& errorState)
	{
		mRun = true;
		mBuffer.resize(mBufferSize);

		// try to open socket
		asio::error_code asio_error_code;
		mSocket.open(udp::v4(), asio_error_code);

		if( asio_error_code )
		{
			errorState.fail(asio_error_code.message());

			if(mThrowOnInitError)
				return false;
			else
				nap::Logger::warn(*this, asio_error_code.message());
		}else
		{
			// try to bind socket
			nap::Logger::info(*this, "Listening at port %i", mPort);
			mSocket.bind(udp::endpoint(address::from_string(mIPRemoteEndpoint), mPort), asio_error_code);
			if( asio_error_code )
			{
				errorState.fail(asio_error_code.message());
				if(mThrowOnInitError)
					return false;
				else
					nap::Logger::warn(*this, asio_error_code.message());
			}else
			{
				// fire up thread
				mReceiverThread = std::thread(std::bind(&UdpServer::receiveThread, this));
			}
		}

		return true;
	}

	void UdpServer::stop()
	{
		mRun = false;
		mSocket.close();
		mReceiverThread.join();
	}

	void UdpServer::receiveThread()
	{
		// bind callback to receive udp
		mSocket.async_receive_from(asio::buffer(mBuffer),
								   mRemoteEndpoint,
								   std::bind(&UdpServer::handleReceive, this, std::placeholders::_1, std::placeholders::_2));

		while(mRun)
		{
			// first, excecute any tasks scheduled
			// consume all tasks queued
			std::vector<std::function<void()>> task_queue;
			{
				std::lock_guard<std::mutex> scoped_lock(mMutex);
				task_queue.swap(mTaskQueue);
			}

			// excecute all tasks
			for(auto& task : task_queue)
			{
				task();
			}

			// run handlers ready to run
			mIOService.poll();
		}

		nap::Logger::info(*this, "Stopped");
	}


	void UdpServer::handleReceive(const asio::error_code& error, size_t bytesTransferred)
	{
		// log any errors
		if (error)
		{
			nap::Logger::info(*this,"Receive failed: %s", error.message().c_str());
			return;
		}

		// construct udp packet, clears current buffer
		std::vector<char> buffer;
		buffer.resize(mBufferSize);
		buffer.swap(mBuffer);

		// forward buffer to any listeners
		UdpPacket packet(std::move(buffer));
		for(auto* listener : mListeners)
		{
			listener->onUdpPacket(packet);
		}

		// rebind handler to socket if the server is still running
		if (mRun)
		{
			mSocket.async_receive_from(asio::buffer(mBuffer),
									   mRemoteEndpoint,
									   std::bind(&UdpServer::handleReceive, this, std::placeholders::_1, std::placeholders::_2));
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
		std::lock_guard<std::mutex> scoped_lock(mMutex);
		mTaskQueue.emplace_back(task);
	}
}
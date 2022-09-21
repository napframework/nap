/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// local includes
#include "udpthread.h"
#include "udpadapter.h"
#include "udpservice.h"

// ASIO includes
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>
#include <asio/io_service.hpp>
#include <asio/system_error.hpp>

// NAP includes
#include <nap/logger.h>

using asio::ip::address;
using asio::ip::udp;

RTTI_BEGIN_ENUM(nap::EUDPThreadUpdateMethod)
	RTTI_ENUM_VALUE(nap::EUDPThreadUpdateMethod::MAIN_THREAD, 		"Main Thread"),
	RTTI_ENUM_VALUE(nap::EUDPThreadUpdateMethod::SPAWN_OWN_THREAD, 	"Spawn Own Thread"),
	RTTI_ENUM_VALUE(nap::EUDPThreadUpdateMethod::MANUAL, 			"Manual")
RTTI_END_ENUM

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::UDPThread)
	RTTI_PROPERTY("Update Method", 	&nap::UDPThread::mUpdateMethod, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// UDPThread
	//////////////////////////////////////////////////////////////////////////


	UDPThread::UDPThread(UDPService & service) : mService(service)
	{
		mManualProcessFunc = [this]()
		{
			nap::Logger::warn(*this, "calling manual process function when thread update method is not manual!");
		};
	}


    UDPThread::~UDPThread(){}


	bool UDPThread::start(utility::ErrorState& errorState)
	{
		switch (mUpdateMethod)
		{
		case EUDPThreadUpdateMethod::SPAWN_OWN_THREAD:
			mThread = std::thread([this] { thread(); });
			break;
		case EUDPThreadUpdateMethod::MAIN_THREAD:
			mService.registerUdpThread(this);
			break;
		case EUDPThreadUpdateMethod::MANUAL:
			mManualProcessFunc = [this]() { process(); };
			break;
		default:
			errorState.fail("Unknown UDP thread update method");
			return false;
		}

		mRun.store(true);

		return true;
	}


	void UDPThread::stop()
	{
		if(mRun.load())
		{
            mRun.store(false);

			switch (mUpdateMethod)
			{
			case EUDPThreadUpdateMethod::SPAWN_OWN_THREAD:
				mThread.join();
				break;
			case EUDPThreadUpdateMethod::MAIN_THREAD:
				mService.removeUdpThread(this);
				break;
			default:
				break;
			}
		}
	}


	void UDPThread::thread()
	{
		while (mRun.load())
		{
			process();
		}
	}


	void UDPThread::process()
	{
		std::lock_guard lock(mMutex);

		for(auto& adapter : mAdapters)
		{
			adapter->process();
		}
	}


	void UDPThread::manualProcess()
	{
		mManualProcessFunc();
	}


	void UDPThread::removeAdapter(UDPAdapter * adapter)
	{
		std::lock_guard lock(mMutex);

		auto found_it = std::find_if(mAdapters.begin(), mAdapters.end(), [&](const auto& it)
			{
				return it == adapter;
			});
		assert(found_it != mAdapters.end());
		mAdapters.erase(found_it);
	}


	void UDPThread::registerAdapter(UDPAdapter * adapter)
	{
		std::lock_guard lock(mMutex);

		mAdapters.emplace_back(adapter);
	}
}

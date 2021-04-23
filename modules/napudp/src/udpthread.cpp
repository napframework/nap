/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "udpthread.h"
#include "udpdevice.h"

using asio::ip::address;
using asio::ip::udp;

RTTI_BEGIN_CLASS(nap::UdpThread)
	RTTI_PROPERTY("Devices", &nap::UdpThread::mDevices, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
	bool UdpThread::start(utility::ErrorState& errorState)
	{
		mRun.store(true);
		mThread = std::thread([this] { process(); });

		return true;
	}


	void UdpThread::stop()
	{
		if( mRun.load() )
		{
			mRun.store(false);
			mThread.join();
		}
	}


	void UdpThread::process()
	{
		while(mRun.load())
		{
			for(auto& device : mDevices)
			{
				device->process();
			}
		}
	}
}
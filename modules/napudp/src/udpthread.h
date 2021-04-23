/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External includes
#include <nap/resourceptr.h>
#include <nap/resource.h>
#include <nap/device.h>
#include <thread>
#include <mutex>

// NAP includes
#include <nap/numeric.h>
#include <concurrentqueue.h>

// ASIO includes
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>
#include <asio/io_service.hpp>
#include <asio/system_error.hpp>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// forward declares
	class UdpDevice;

	class NAPAPI UdpThread : public Device
	{
		RTTI_ENABLE(Device)
	public:
		virtual bool start(utility::ErrorState& errorState) override;

		virtual void stop() override;
	public:
		std::vector<ResourcePtr<UdpDevice>> mDevices;
	private:
		void process();

		std::thread mThread;
		std::atomic_bool mRun = { false };
	};
}

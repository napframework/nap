/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "udpservice.h"
#include "udpthread.h"

// External includes
#include <memory>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::UDPService)
RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// UDPService
	//////////////////////////////////////////////////////////////////////////

	UDPService::UDPService(ServiceConfiguration* configuration) :
		Service(configuration)
	{
	}


	bool UDPService::init(utility::ErrorState& error)
	{
		return true;
	}


	void UDPService::shutdown()
	{
	}


	void UDPService::registerObjectCreators(rtti::Factory& factory)
	{
		factory.addObjectCreator(std::make_unique<UDPThreadPoolObjectCreator>(*this));
	}


	void UDPService::update(double deltaTime)
	{
		for(auto* thread : mThreads)
		{
			thread->process();
		}
	}


	void UDPService::removeUdpThread(UDPThread* threadPool)
	{
		auto found_it = std::find_if(mThreads.begin(), mThreads.end(), [&](const auto& it)
		{
		  return it == threadPool;
		});
		assert(found_it != mThreads.end());
		mThreads.erase(found_it);
	}


	void UDPService::registerUdpThread(UDPThread* threadPool)
	{
		mThreads.emplace_back(threadPool);
	}
}
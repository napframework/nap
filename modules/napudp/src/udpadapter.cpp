/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "udpadapter.h"
#include "udpthread.h"

#include <nap/logger.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::UDPAdapter)
	RTTI_PROPERTY("Thread", &nap::UDPAdapter::mThread, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("AllowFailure", &nap::UDPAdapter::mAllowFailure, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// UDPAdapter
	//////////////////////////////////////////////////////////////////////////

    UDPAdapter::UDPAdapter(){}


    UDPAdapter::~UDPAdapter(){}


	bool UDPAdapter::init(utility::ErrorState& errorState)
	{
		if(!errorState.check(mThread !=nullptr, "Thread cannot be nullptr"))
			return false;

		return true;
	}


	void UDPAdapter::onDestroy()
	{
	}


    bool UDPAdapter::start(utility::ErrorState& errorState)
    {
        if(!onStart(errorState))
            return false;

        mThread->registerAdapter(this);

        return true;
    }


    void UDPAdapter::stop()
    {
        onStop();

        mThread->removeAdapter(this);
    };


    bool UDPAdapter::handleAsioError(const std::error_code& errorCode, utility::ErrorState& errorState, bool& success)
    {
        if(errorCode)
        {
            if(!mAllowFailure)
            {
                success = false;
                errorState.fail(errorCode.message());

                return true;
            }else
            {
                success = true;
                nap::Logger::error(*this, errorCode.message());

                return true;
            }
        }

        return false;
    }


    asio::io_context& UDPAdapter::getIOContext()
    {
        return mThread->getIOContext();
    }
}

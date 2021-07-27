/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "udpadapter.h"
#include "udpthread.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::UDPAdapter)
	RTTI_PROPERTY("Thread", &nap::UDPAdapter::mThread, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// UDPAdapter
	//////////////////////////////////////////////////////////////////////////

	bool UDPAdapter::init(utility::ErrorState& errorState)
	{
		if(!errorState.check(mThread !=nullptr, "Thread cannot be nullptr"))
			return false;

		mThread->registerAdapter(this);
		return true;
	}


	void UDPAdapter::onDestroy()
	{
		mThread->removeAdapter(this);
	}
}

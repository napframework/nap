/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "udpadapter.h"
#include "udpthread.h"

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// UDPAdapter
	//////////////////////////////////////////////////////////////////////////

	bool UDPAdapter::init(utility::ErrorState& errorState)
	{
		if(!errorState.check(mThread !=nullptr, "UDPThread cannot be nullptr"))
			return false;

		mThread->registerAdapter(this);

		return true;
	}


	void UDPAdapter::onDestroy()
	{
		mThread->removeAdapter(this);
	}
}
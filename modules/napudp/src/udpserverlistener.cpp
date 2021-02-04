/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "udpserverlistener.h"

#include <nap/logger.h>

namespace nap
{
	bool UdpServerListener::init(utility::ErrorState& errorState)
	{
		if(!errorState.check(mServer != nullptr, "Server resource ptr is null"))
			return false;

		mServer->registerListener(this);

		return true;
	}


	void UdpServerListener::onDestroy()
	{
		mServer->removeListener(this);
	}
}
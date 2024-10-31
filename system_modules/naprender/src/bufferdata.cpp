/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "bufferdata.h"

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// BufferData
	//////////////////////////////////////////////////////////////////////////

	void BufferData::release()
	{
		mBuffer = VK_NULL_HANDLE;
		mAllocation = VK_NULL_HANDLE;
	}
}

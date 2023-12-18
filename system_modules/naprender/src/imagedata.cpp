/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "imagedata.h"

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// ImageData
	//////////////////////////////////////////////////////////////////////////

	void ImageData::release()
	{
		for (auto& view : mSubViews)
			view = VK_NULL_HANDLE;

		mView = VK_NULL_HANDLE;
		mImage = VK_NULL_HANDLE;
		mAllocation = VK_NULL_HANDLE;
	}
}

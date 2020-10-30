/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "image.h"
#include "nap/core.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Image)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap
{
	Image::Image(Core& core) :
		Texture2D(core)
	{
	}


	void Image::update()
	{
		assert(!mBitmap.empty());
		update(mBitmap.getData(), mBitmap.mSurfaceDescriptor);
	}


	void Image::asyncGetData()
	{
		asyncGetData(mBitmap);
	}
}

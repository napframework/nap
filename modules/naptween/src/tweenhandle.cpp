/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "tweenhandle.h"
#include "tweenservice.h"

namespace nap
{
	TweenHandleBase::TweenHandleBase(TweenService& tweenService)
		: mService(tweenService)
	{

	}

	TweenHandleBase::~TweenHandleBase()
	{
		mService.removeTween(mTweenBase);
	}
}
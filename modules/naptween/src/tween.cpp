//
// Created by CodeFlow on 19/10/2020.
//

#include "tween.h"
#include "tweenservice.h"

namespace nap
{
	TweenHandleBase::TweenHandleBase(TweenService& tweenService)
		: mService(tweenService){}

	TweenHandleBase::~TweenHandleBase()
	{
		mService.removeTween(mTweenBase);
	}

	TweenBase::TweenBase()
	{
	}

	TweenBase::~TweenBase()
	{
	}
}
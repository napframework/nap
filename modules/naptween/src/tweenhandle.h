/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// internal includes
#include "tween.h"

// external includes
#include <mathutils.h>
#include <nap/signalslot.h>

namespace nap
{
	class TweenService;

	class NAPAPI TweenHandleBase
	{
	public:
		TweenHandleBase(TweenService& tweenService);

		virtual ~TweenHandleBase();
	protected:
		TweenService& 	mService;
		TweenBase*		mTweenBase;
	};

	template<typename T>
	class TweenHandle : public TweenHandleBase
	{
	public:
		TweenHandle(TweenService& tweenService, Tween<T>* tween);
	public:
		Tween<T>& getTween(){ return *mTween; }
	private:
		Tween<T>* mTween;
	};

	//////////////////////////////////////////////////////////////////////////
	// shortcuts
	//////////////////////////////////////////////////////////////////////////
	using TweenHandleFloat 	= TweenHandle<float>;
	using TweenHandleDouble = TweenHandle<double>;
	using TweenHandleVec2 	= TweenHandle<glm::vec2>;
	using TweenHandleVec3 	= TweenHandle<glm::vec3>;

	//////////////////////////////////////////////////////////////////////////
	// explicit MSVC template specialization exports
	//////////////////////////////////////////////////////////////////////////
	template class NAPAPI TweenHandle<float>;
	template class NAPAPI TweenHandle<double>;
	template class NAPAPI TweenHandle<glm::vec2>;
	template class NAPAPI TweenHandle<glm::vec3>;

	//////////////////////////////////////////////////////////////////////////
	// template definition
	//////////////////////////////////////////////////////////////////////////
	template<typename T>
	TweenHandle<T>::TweenHandle(TweenService& tweenService, Tween<T>* tween)
		: TweenHandleBase(tweenService), mTween(tween)
	{
		mTweenBase = tween;
	}
}


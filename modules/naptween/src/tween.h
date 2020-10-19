/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// internal includes
#include "easing.h"

// external includes
#include <mathutils.h>
#include <nap/signalslot.h>

namespace nap
{
	class TweenService;

	class TweenBase
	{
	public:
		TweenBase();

		virtual ~TweenBase();

		virtual void update(double deltaTime) = 0;
	private:
	};

	template<typename T>
	class Tween : public TweenBase
	{
	public:
		Tween(T start, T end, float duration);

		void update(double deltaTime) override;

		void setEase(Easing easing);
	public:
		Signal<T> UpdateSignal;
		Signal<T> CompleteSignal;
	private:
		std::unique_ptr<EaseBase<T>> mEase = nullptr;

		float 	mTime = 0.0f;
		T 		mStart;
		T		mEnd;
		float 	mDuration;
		bool 	mComplete = false;
	};

	class TweenHandleBase
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

		void setEase(Easing easing){ mTween->setEase(easing); }
	public:
		Signal<T>& UpdateSignal;
		Signal<T>& CompleteSignal;
	private:
		Tween<T>* mTween;
	};

	//////////////////////////////////////////////////////////////////////////
	// shortcuts
	//////////////////////////////////////////////////////////////////////////
	using TweenFloat 	= Tween<float>;
	using TweenDouble 	= Tween<double>;
	using TweenVec2 	= Tween<glm::vec2>;
	using TweenVec3 	= Tween<glm::vec3>;

	using TweenHandleFloat 	= TweenHandle<float>;
	using TweenHandleVec2 	= TweenHandle<glm::vec2>;

	//////////////////////////////////////////////////////////////////////////
	// explicit MSVC template specialization exports
	//////////////////////////////////////////////////////////////////////////
	template class NAPAPI Tween<float>;
	template class NAPAPI Tween<double>;
	template class NAPAPI Tween<glm::vec2>;
	template class NAPAPI Tween<glm::vec3>;

	template class NAPAPI TweenHandle<float>;
	template class NAPAPI TweenHandle<glm::vec2>;

	//////////////////////////////////////////////////////////////////////////
	// template definition
	//////////////////////////////////////////////////////////////////////////
	template<typename T>
	Tween<T>::Tween(T start, T end, float duration)
		: TweenBase(), mStart(start), mEnd(end), mDuration(duration)
	{
		setEase(Easing::LINEAR);
	}

	template<typename T>
	void Tween<T>::update(double deltaTime)
	{
		if(!mComplete)
		{
			mTime += deltaTime;

			if( mTime > mDuration )
			{
				mTime = mDuration;
				mComplete = true;

				T value = mEase->evaluate(mStart, mEnd, mTime / mDuration);
				CompleteSignal.trigger(value);
			}else
			{
				T value = mEase->evaluate(mStart, mEnd, mTime / mDuration);
				UpdateSignal.trigger(value);
			}
		}
	}

	template<typename T>
	void Tween<T>::setEase(Easing easing)
	{
		switch (easing)
		{
		case Easing::LINEAR:
			mEase = std::make_unique<EaseLinear<T>>();
			break;
		case Easing::OUT_CUBIC:
			break;
		}
	}


	template<typename T>
	TweenHandle<T>::TweenHandle(TweenService& tweenService, Tween<T>* tween)
		: TweenHandleBase(tweenService), mTween(tween), UpdateSignal(tween->UpdateSignal), CompleteSignal(tween->CompleteSignal)
	{
		mTweenBase = tween;
	}
}


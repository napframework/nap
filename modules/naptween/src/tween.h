/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// internal includes
#include "tweeneasing.h"
#include "tweenmode.h"

// external includes
#include <mathutils.h>
#include <nap/signalslot.h>

namespace nap
{
	class TweenService;

	class TweenBase
	{
		friend class TweenHandleBase;
	public:
		TweenBase();

		virtual ~TweenBase();

		virtual void update(double deltaTime) = 0;
	public:
		Signal<> KilledSignal;
	protected:
		bool 	mKilled 	= false;
		bool 	mComplete 	= false;
	};

	template<typename T>
	class Tween : public TweenBase
	{
	public:
		Tween(T start, T end, float duration);

		void update(double deltaTime) override;

		void setEase(TweenEasing easing);

		void setMode(TweenMode mode);

		const TweenMode getMode() const { return mMode; }

		const TweenEasing getEase() const{ return mEasing; }

		const float getTime() const { return mTime; }

		const float getDuration() const { return mDuration; }

		const T getStartValue() const { return mStart; }

		const T getEndValue() const { return mEnd; }
	public:
		Signal<const T&> UpdateSignal;
		Signal<const T&> CompleteSignal;
	private:
		std::unique_ptr<TweenEaseBase<T>> mEase = nullptr;

		std::function<void(double)> mUpdateFunc;

		float 		mTime 		= 0.0f;
		T 			mStart;
		T			mEnd;
		float 		mDuration;
		TweenMode 	mMode 		= TweenMode::NORMAL;
		TweenEasing mEasing 	= TweenEasing::LINEAR;
	};

	//////////////////////////////////////////////////////////////////////////
	// shortcuts
	//////////////////////////////////////////////////////////////////////////
	using TweenFloat 	= Tween<float>;
	using TweenDouble 	= Tween<double>;
	using TweenVec2 	= Tween<glm::vec2>;
	using TweenVec3 	= Tween<glm::vec3>;

	//////////////////////////////////////////////////////////////////////////
	// explicit MSVC template specialization exports
	//////////////////////////////////////////////////////////////////////////
	template class NAPAPI Tween<float>;
	template class NAPAPI Tween<double>;
	template class NAPAPI Tween<glm::vec2>;
	template class NAPAPI Tween<glm::vec3>;

	//////////////////////////////////////////////////////////////////////////
	// template definition
	//////////////////////////////////////////////////////////////////////////
	template<typename T>
	Tween<T>::Tween(T start, T end, float duration)
		: TweenBase(), mStart(start), mEnd(end), mDuration(duration)
	{
		setEase(TweenEasing::LINEAR);
		setMode(TweenMode::NORMAL);
	}

	template<typename T>
	void Tween<T>::update(double deltaTime)
	{
		if(mKilled)
			return;

		mUpdateFunc(deltaTime);
	}

	template<typename T>
	void Tween<T>::setMode(TweenMode mode)
	{
		mMode = mode;

		switch (mode)
		{
		case NORMAL:
		{
			mUpdateFunc = [this](double deltaTime)
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
			};
		}
			break;
		case PING_PONG:
		{
			float direction = 1.0f;
			mUpdateFunc = [this, direction](double deltaTime) mutable
			{
				mTime += deltaTime * direction;

				if( mTime > mDuration )
				{
					mTime = mDuration - ( mTime - mDuration );
					direction = -1.0f;

					T value = mEase->evaluate(mStart, mEnd, mTime / mDuration);
					UpdateSignal.trigger(value);
				}else if( mTime < 0.0f )
				{
					mTime = -mTime;
					direction = 1.0f;

					T value = mEase->evaluate(mStart, mEnd, mTime / mDuration);
					UpdateSignal.trigger(value);
				}else
				{
					T value = mEase->evaluate(mStart, mEnd, mTime / mDuration);
					UpdateSignal.trigger(value);
				}
			};
		}
			break;
		case LOOP:
		{
			mUpdateFunc = [this](double deltaTime)
			{
				if(!mComplete)
				{
					mTime += deltaTime;

					if( mTime > mDuration )
					{
						mTime = mDuration - mTime;

						T value = mEase->evaluate(mStart, mEnd, mTime / mDuration);
						UpdateSignal.trigger(value);
					}else
					{
						T value = mEase->evaluate(mStart, mEnd, mTime / mDuration);
						UpdateSignal.trigger(value);
					}
				}
			};
		}
			break;
		case REVERSE:
		{
			mUpdateFunc = [this](double deltaTime)
			{
				if(!mComplete)
				{
					mTime += deltaTime;

					if( mTime > mDuration )
					{
					  	mTime = mDuration;

					  	T value = mEase->evaluate(mStart, mEnd, 1.0f - ( mTime / mDuration ));
					  	CompleteSignal.trigger(value);
					}else
					{
					  	T value = mEase->evaluate(mStart, mEnd, 1.0f - ( mTime / mDuration ));
					  	UpdateSignal.trigger(value);
					}
				}
			};
		}
			break;
		}
	}

	template<typename T>
	void Tween<T>::setEase(TweenEasing easing)
	{
		mEasing = easing;

		switch (easing)
		{
		case TweenEasing::LINEAR:
			mEase = std::make_unique<TweenEaseLinear<T>>();
			break;
		case TweenEasing::CUBIC_INOUT:
			mEase = std::make_unique<TweenEaseInOutCubic<T>>();
			break;
		case TweenEasing::CUBIC_OUT:
			mEase = std::make_unique<TweenEaseOutCubic<T>>();
			break;
		case TweenEasing::CUBIC_IN:
			mEase = std::make_unique<TweenEaseInCubic<T>>();
			break;
		case TweenEasing::BACK_OUT:
			mEase = std::make_unique<TweenEaseOutBack<T>>();
			break;
		case TweenEasing::BACK_INOUT:
			mEase = std::make_unique<TweenEaseInOutBack<T>>();
			break;
		case TweenEasing::BACK_IN:
			mEase = std::make_unique<TweenEaseInBack<T>>();
			break;
		case TweenEasing::BOUNCE_OUT:
			mEase = std::make_unique<TweenEaseOutBounce<T>>();
			break;
		case TweenEasing::BOUNCE_INOUT:
			mEase = std::make_unique<TweenEaseInOutBounce<T>>();
			break;
		case TweenEasing::BOUNCE_IN:
			mEase = std::make_unique<TweenEaseInBounce<T>>();
			break;
		case TweenEasing::CIRC_OUT:
			mEase = std::make_unique<TweenEaseOutCirc<T>>();
			break;
		case TweenEasing::CIRC_INOUT:
			mEase = std::make_unique<TweenEaseInOutCirc<T>>();
			break;
		case TweenEasing::CIRC_IN:
			mEase = std::make_unique<TweenEaseInCirc<T>>();
			break;
		case TweenEasing::ELASTIC_OUT:
			mEase = std::make_unique<TweenEaseOutElastic<T>>();
			break;
		case TweenEasing::ELASTIC_INOUT:
			mEase = std::make_unique<TweenEaseInOutElastic<T>>();
			break;
		case TweenEasing::ELASTIC_IN:
			mEase = std::make_unique<TweenEaseInElastic<T>>();
			break;
		case TweenEasing::EXPO_OUT:
			mEase = std::make_unique<TweenEaseOutExpo<T>>();
			break;
		case TweenEasing::EXPO_INOUT:
			mEase = std::make_unique<TweenEaseInOutExpo<T>>();
			break;
		case TweenEasing::EXPO_IN:
			mEase = std::make_unique<TweenEaseInExpo<T>>();
			break;
		case TweenEasing::QUAD_OUT:
			mEase = std::make_unique<TweenEaseOutQuad<T>>();
			break;
		case TweenEasing::QUAD_INOUT:
			mEase = std::make_unique<TweenEaseInOutQuad<T>>();
			break;
		case TweenEasing::QUAD_IN:
			mEase = std::make_unique<TweenEaseInQuad<T>>();
			break;
		case TweenEasing::QUART_OUT:
			mEase = std::make_unique<TweenEaseOutQuart<T>>();
			break;
		case TweenEasing::QUART_INOUT:
			mEase = std::make_unique<TweenEaseInOutQuart<T>>();
			break;
		case TweenEasing::QUART_IN:
			mEase = std::make_unique<TweenEaseInQuart<T>>();
			break;
		case TweenEasing::QUINT_OUT:
			mEase = std::make_unique<TweenEaseOutQuint<T>>();
			break;
		case TweenEasing::QUINT_INOUT:
			mEase = std::make_unique<TweenEaseInOutQuint<T>>();
			break;
		case TweenEasing::QUINT_IN:
			mEase = std::make_unique<TweenEaseInQuint<T>>();
			break;
		case TweenEasing::SINE_OUT:
			mEase = std::make_unique<TweenEaseOutSine<T>>();
			break;
		case TweenEasing::SINE_INOUT:
			mEase = std::make_unique<TweenEaseInOutSine<T>>();
			break;
		case TweenEasing::SINE_IN:
			mEase = std::make_unique<TweenEaseInSine<T>>();
			break;
		}
	}
}


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
#include <nap/logger.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	// forward declares
	class TweenService;

	/**
	 * Base class of every tween
	 */
	class NAPAPI TweenBase
	{
		// tween service can access properties of the tween
		friend class TweenService;
	public:
		/**
		 * Constructor
		 */
		TweenBase() = default;

		/**
		 * Default deconstructor
		 */
		virtual ~TweenBase() = default;

		/**
		 * Update function called by tween service
		 * @param deltaTime
		 */
		virtual void update(double deltaTime) = 0;
	public:
		// signals

		/**
		 * Killed signal will be dispatched when the tween is removed by the service but isn't completed yet
		 */
		Signal<> KilledSignal;
	protected:
		// killed boolean
		bool 	mKilled 	= false;

		// complete boolean
		bool 	mComplete 	= false;
	};

	/**
	 * A Tween is responsible for interpolating between two values over the period of a certain time using an easing method ( see : https://github.com/jesusgollonet/ofpennereasing )
	 * A Tween can be created by the user in which case the user is responsible for updating and managing the tween.
	 * A Tween can also be created by the TweenService
	 * When the Tween is created by the TweenService, the service retains the unique pointers to the Tween and gives back a tween handle to the user
	 * When the Tween is created by the TweenService, the tween will receive update calls from the service, which is the main thread
	 * this update call happens before the update call on components and application
	 * You can tween any type that supports arithmetic operators
	 * Please note that when the tween is created by the TweenService, it can ONLY be accessible outside the TweenService by using the TweenHandle
	 * @tparam T the type of value that you would like to tween
	 */
	template<typename T>
	class Tween : public TweenBase
	{
	public:
		/**
		 * Constructor taking the initial start & end value of the tween, plus duration
		 * @param start start value of the tween
		 * @param end end value of the tween
		 * @param duration duration of the tween
		 */
		Tween(T start, T end, float duration);

		/**
		 * update function called by the TweenService
		 * @param deltaTime
		 */
		void update(double deltaTime) override;

		/**
		 * set easing method used for tweening
		 * @param easing the easing method
		 */
		void setEase(ETweenEaseType easing);

		/**
		 * sets the tween mode for this tween, see ETweenMode enum
		 * @param mode
		 */
		void setMode(ETweenMode mode);

		/**
		 * set the duration for this tween
		 * also changes mTime of this tween, to ensure smooth tweening
		 * asserts when duration < 0.0f
		 */
		void setDuration(float duration);

		/**
		 * restart the tween
		 */
		void restart();

		/**
		 * @return current tween mode
		 */
		ETweenMode getMode() const { return mMode; }

		/**
		 * @return current ease type
		 */
		ETweenEaseType getEase() const{ return mEasing; }

		/**
		 * @return current time in time
		 */
		float getTime() const { return mTime; }

		/**
		 * @return duration of tween
		 */
		float getDuration() const { return mDuration; }

		/**
		 * @return current tweened value
		 */
		const T& getCurrentValue() const { return mCurrentValue; }

		/**
		 * @return start value
		 */
		const T& getStartValue() const { return mStart; }

		/**
		 * @return end value
		 */
		const T& getEndValue() const { return mEnd; }
	public:
		// Signals

		/**
		 * Update signal dispatched on value update
		 * Occurs on main thread
		 */
		Signal<const T&> UpdateSignal;

		/**
		 * Complete signal dispatched when tween is finished
		 * Always dispatched on main thread
		 */
		Signal<const T&> CompleteSignal;
	private:

		/**
		 * unique ptr to current easing method
		 */
		std::unique_ptr<TweenEaseBase<T>> mEase = nullptr;

		/**
		 * holds update function, update function is dependent on current tween mode
		 */
		std::function<void(double)> mUpdateFunc;

		// current time
		float 			mTime 			= 0.0f;

		// start value
		T 				mStart;

		// end value
		T				mEnd;

		// current value
		T 				mCurrentValue;

		// duration
		float 			mDuration;

		// tween mode
		ETweenMode 		mMode 			= ETweenMode::NORMAL;

		// ease type
		ETweenEaseType 	mEasing 		= ETweenEaseType::LINEAR;
	};


	//////////////////////////////////////////////////////////////////////////
	// Type Definitions
	//////////////////////////////////////////////////////////////////////////
	using TweenFloat 	= Tween<float>;
	using TweenDouble 	= Tween<double>;
	using TweenVec2 	= Tween<glm::vec2>;
	using TweenVec3 	= Tween<glm::vec3>;


	//////////////////////////////////////////////////////////////////////////
	// Template Definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename T>
	Tween<T>::Tween(T start, T end, float duration)
		: TweenBase(), mStart(start), mEnd(end), mCurrentValue(start), mDuration(duration)
	{
		setEase(ETweenEaseType::LINEAR);
		setMode(ETweenMode::NORMAL);

		// when this tween is killed, update function doesn't do anything anymore
		KilledSignal.connect([this]{
			mUpdateFunc = [this](double deltaTime){};
		});
	}

	template<typename T>
	void Tween<T>::update(double deltaTime)
	{
		mUpdateFunc(deltaTime);
	}

	template<typename T>
	void Tween<T>::setDuration(float duration)
	{
		assert(duration >= 0.0f); // invalid duration

		// when duration is bigger then 0, scale time accordingly to ensure smooth transition
		if( duration > 0.0f )
		{
			float current_progress = mTime / mDuration;
			mDuration = duration;
			mTime = mDuration * current_progress;
		}
		else
		{
			// when duration is 0, it doesn't matter since we will hit complete in next update
			mDuration = duration;
			mTime = 0.0f;
		}
	}

	template<typename T>
	void Tween<T>::setMode(ETweenMode mode)
	{
		mMode = mode;
		switch (mode)
		{
			default:
			{
				nap::Logger::warn("Unknown tween mode, choosing NORMAL mode");
			}
			case NORMAL:
			{
				mUpdateFunc = [this](double deltaTime)
				{
				  if(!mComplete)
				  {
					  mTime += deltaTime;

					  if( mTime >= mDuration )
					  {
						  mTime = mDuration;
						  mComplete = true;

						  mCurrentValue = mEase->evaluate(mStart, mEnd, mTime / mDuration);

						  UpdateSignal.trigger(mCurrentValue);
						  CompleteSignal.trigger(mCurrentValue);
					  }else
					  {
						  mCurrentValue = mEase->evaluate(mStart, mEnd, mTime / mDuration);
						  UpdateSignal.trigger(mCurrentValue);
					  }
				  }
				};
				break;
			}
			case PING_PONG:
			{
				float direction = 1.0f;
				mUpdateFunc = [this, direction](double deltaTime) mutable
				{
					mTime += deltaTime * direction;

					if(mTime >= mDuration)
					{
						mTime = mDuration - ( mTime - mDuration );
						direction = -1.0f;

						mCurrentValue = mEase->evaluate(mStart, mEnd, mTime / mDuration);
						UpdateSignal.trigger(mCurrentValue);
					}
					else if(mTime <= 0.0f)
					{
						mTime = -mTime;
						direction = 1.0f;

						mCurrentValue = mEase->evaluate(mStart, mEnd, mTime / mDuration);
						UpdateSignal.trigger(mCurrentValue);
					}
					else
					{
						mCurrentValue = mEase->evaluate(mStart, mEnd, mTime / mDuration);
						UpdateSignal.trigger(mCurrentValue);
					}
				};
				break;
			}
			case LOOP:
			{
				mUpdateFunc = [this](double deltaTime)
				{
					if(!mComplete)
					{
						mTime += deltaTime;

						if(mTime >= mDuration)
						{
							mTime = mDuration - mTime;

							mCurrentValue = mEase->evaluate(mStart, mEnd, mTime / mDuration);
							UpdateSignal.trigger(mCurrentValue);
						}
						else
						{
							mCurrentValue = mEase->evaluate(mStart, mEnd, mTime / mDuration);
							UpdateSignal.trigger(mCurrentValue);
						}
					}
				};
				break;
			}
			case REVERSE:
			{
				mUpdateFunc = [this](double deltaTime)
				{
					if(!mComplete)
					{
						mTime += deltaTime;
						if(mTime >= mDuration)
						{
							mComplete = true;
						  	mTime = mDuration;

							mCurrentValue = mEase->evaluate(mStart, mEnd, 1.0f - ( mTime / mDuration ));

							UpdateSignal.trigger(mCurrentValue);
						  	CompleteSignal.trigger(mCurrentValue);
						}
						else
						{
							mCurrentValue = mEase->evaluate(mStart, mEnd, 1.0f - ( mTime / mDuration ));
						  	UpdateSignal.trigger(mCurrentValue);
						}
					}
				};
				break;
			}
		}
	}

	template<typename T>
	void Tween<T>::restart()
	{
		mTime	 		= 0.0f;
		mComplete 		= false;
		mKilled 		= false;
		mCurrentValue 	= mStart;
	}


	template<typename T>
	void Tween<T>::setEase(ETweenEaseType easing)
	{
		static std::unordered_map<ETweenEaseType, std::function<std::unique_ptr<TweenEaseBase<T>>()>> ease_constructors
		{
			{ETweenEaseType::LINEAR, 		[](){ return std::make_unique<TweenEaseLinear<T>>(); 		}},
			{ETweenEaseType::CUBIC_INOUT, 	[](){ return std::make_unique<TweenEaseOutCubic<T>>(); 		}},
			{ETweenEaseType::CUBIC_OUT, 	[](){ return std::make_unique<TweenEaseOutCubic<T>>(); 		}},
			{ETweenEaseType::CUBIC_IN, 		[](){ return std::make_unique<TweenEaseInCubic<T>>(); 		}},
			{ETweenEaseType::BACK_OUT, 		[](){ return std::make_unique<TweenEaseOutBack<T>>(); 		}},
			{ETweenEaseType::BACK_INOUT, 	[](){ return std::make_unique<TweenEaseInOutBack<T>>(); 	}},
			{ETweenEaseType::BACK_IN, 		[](){ return std::make_unique<TweenEaseInBack<T>>(); 		}},
			{ETweenEaseType::BOUNCE_OUT, 	[](){ return std::make_unique<TweenEaseOutBounce<T>>(); 	}},
			{ETweenEaseType::BOUNCE_INOUT, 	[](){ return std::make_unique<TweenEaseInOutBounce<T>>(); 	}},
			{ETweenEaseType::BOUNCE_IN, 	[](){ return std::make_unique<TweenEaseInBounce<T>>(); 		}},
			{ETweenEaseType::CIRC_OUT,	 	[](){ return std::make_unique<TweenEaseOutCirc<T>>(); 		}},
			{ETweenEaseType::CIRC_INOUT, 	[](){ return std::make_unique<TweenEaseInOutCirc<T>>(); 	}},
			{ETweenEaseType::CIRC_IN, 		[](){ return std::make_unique<TweenEaseInCirc<T>>(); 		}},
			{ETweenEaseType::ELASTIC_OUT, 	[](){ return std::make_unique<TweenEaseOutElastic<T>>(); 	}},
			{ETweenEaseType::ELASTIC_INOUT, [](){ return std::make_unique<TweenEaseInOutElastic<T>>(); 	}},
			{ETweenEaseType::ELASTIC_IN, 	[](){ return std::make_unique<TweenEaseInElastic<T>>(); 	}},
			{ETweenEaseType::EXPO_OUT, 		[](){ return std::make_unique<TweenEaseOutExpo<T>>(); 		}},
			{ETweenEaseType::EXPO_INOUT, 	[](){ return std::make_unique<TweenEaseInOutExpo<T>>(); 	}},
			{ETweenEaseType::EXPO_IN, 		[](){ return std::make_unique<TweenEaseInExpo<T>>(); 		}},
			{ETweenEaseType::QUAD_OUT, 		[](){ return std::make_unique<TweenEaseOutQuad<T>>(); 		}},
			{ETweenEaseType::QUAD_INOUT, 	[](){ return std::make_unique<TweenEaseInOutQuad<T>>(); 	}},
			{ETweenEaseType::QUAD_IN, 		[](){ return std::make_unique<TweenEaseInQuad<T>>(); 		}},
			{ETweenEaseType::QUART_OUT, 	[](){ return std::make_unique<TweenEaseOutQuart<T>>(); 		}},
			{ETweenEaseType::QUART_INOUT, 	[](){ return std::make_unique<TweenEaseInOutQuart<T>>(); 	}},
			{ETweenEaseType::QUART_IN, 		[](){ return std::make_unique<TweenEaseInQuart<T>>(); 		}},
			{ETweenEaseType::QUINT_OUT, 	[](){ return std::make_unique<TweenEaseOutQuint<T>>(); 		}},
			{ETweenEaseType::QUINT_INOUT, 	[](){ return std::make_unique<TweenEaseInOutQuint<T>>(); 	}},
			{ETweenEaseType::QUINT_IN, 		[](){ return std::make_unique<TweenEaseInQuint<T>>(); 		}},
			{ETweenEaseType::SINE_OUT, 		[](){ return std::make_unique<TweenEaseOutSine<T>>(); 		}},
			{ETweenEaseType::SINE_INOUT, 	[](){ return std::make_unique<TweenEaseInOutSine<T>>(); 	}},
			{ETweenEaseType::SINE_IN, 		[](){ return std::make_unique<TweenEaseInSine<T>>(); 		}}
		};

		mEasing = easing;

		auto constructor_it = ease_constructors.find(easing);
		assert(constructor_it != ease_constructors.end()); // entry not found
		mEase = constructor_it->second();
	}
}


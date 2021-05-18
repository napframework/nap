/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// external includes
#include <mathutils.h>
#include <math.h>
#include <easing.h>

namespace nap
{
	/**
	 * Every possible tween ease type (serializable)
	 */
	enum ETweenEaseType : int
	{
		LINEAR 			= 0,
		CUBIC_IN	  	= 1,
		CUBIC_INOUT	  	= 2,
		CUBIC_OUT	  	= 3,
		BACK_IN		  	= 4,
		BACK_INOUT	  	= 5,
		BACK_OUT 		= 6,
		BOUNCE_IN 		= 7,
		BOUNCE_INOUT 	= 8,
		BOUNCE_OUT 		= 9,
		CIRC_IN 		= 10,
		CIRC_INOUT 		= 11,
		CIRC_OUT 		= 12,
		ELASTIC_IN 		= 13,
		ELASTIC_INOUT 	= 14,
		ELASTIC_OUT 	= 15,
		EXPO_IN 		= 16,
		EXPO_INOUT 		= 17,
		EXPO_OUT 		= 18,
		QUAD_IN 		= 19,
		QUAD_INOUT 		= 20,
		QUAD_OUT 		= 21,
		QUART_IN 		= 22,
		QUART_INOUT 	= 23,
		QUART_OUT 		= 24,
		QUINT_IN 		= 25,
		QUINT_INOUT 	= 26,
		QUINT_OUT 		= 27,
		SINE_IN 		= 28,
		SINE_INOUT		= 29,
		SINE_OUT 		= 30
	};

	/**
	 * Base class for evaluation
	 */
	template<typename T>
	class TweenEaseBase
	{
	public:
		/**
		 * Constructor
		 */
		TweenEaseBase() = default;

		/**
		 * Deconstructor
		 */
		virtual ~TweenEaseBase() = default;

		/**
		 * Evaluates using an easing method between start and end value
		 * Override this method to implement your own evaluation method
		 * @param start the start value
		 * @param end the end value
		 * @param progress progress between start & end ( float between 0 and 1 )
		 * @return the value computed by easing method
		 */
		virtual T evaluate(T& start, T& end, float progress) = 0;
	};

	template<typename T>
	class TweenEaseLinear : public TweenEaseBase<T>
	{
	public:
		T evaluate(T& start, T& end, float progress) override;
	};

	template<typename T>
	class TweenEaseInCubic : public TweenEaseBase<T>
	{
	public:
		T evaluate(T& start, T& end, float progress) override;
	};

	template<typename T>
	class TweenEaseInOutCubic : public TweenEaseBase<T>
	{
	public:
		T evaluate(T& start, T& end, float progress) override;
	};

	template<typename T>
	class TweenEaseOutCubic : public TweenEaseBase<T>
	{
	public:
		T evaluate(T& start, T& end, float progress) override;
	};

	template<typename T>
	class TweenEaseInBack : public TweenEaseBase<T>
	{
	public:
		T evaluate(T& start, T& end, float progress) override;
	};

	template<typename T>
	class TweenEaseInOutBack : public TweenEaseBase<T>
	{
	public:
		T evaluate(T& start, T& end, float progress) override;
	};

	template<typename T>
	class TweenEaseOutBack : public TweenEaseBase<T>
	{
	public:
		T evaluate(T& start, T& end, float progress) override;
	};

	template<typename T>
	class TweenEaseInBounce : public TweenEaseBase<T>
	{
	public:
		T evaluate(T& start, T& end, float progress) override;
	};

	template<typename T>
	class TweenEaseInOutBounce : public TweenEaseBase<T>
	{
	public:
		T evaluate(T& start, T& end, float progress) override;
	};

	template<typename T>
	class TweenEaseOutBounce : public TweenEaseBase<T>
	{
	public:
		T evaluate(T& start, T& end, float progress) override;
	};

	template<typename T>
	class TweenEaseInCirc : public TweenEaseBase<T>
	{
	public:
		T evaluate(T& start, T& end, float progress) override;
	};

	template<typename T>
	class TweenEaseInOutCirc : public TweenEaseBase<T>
	{
	public:
		T evaluate(T& start, T& end, float progress) override;
	};

	template<typename T>
	class TweenEaseOutCirc : public TweenEaseBase<T>
	{
	public:
		T evaluate(T& start, T& end, float progress) override;
	};

	template<typename T>
	class TweenEaseInElastic : public TweenEaseBase<T>
	{
	public:
		T evaluate(T& start, T& end, float progress) override;
	};

	template<typename T>
	class TweenEaseInOutElastic : public TweenEaseBase<T>
	{
	public:
		T evaluate(T& start, T& end, float progress) override;
	};

	template<typename T>
	class TweenEaseOutElastic : public TweenEaseBase<T>
	{
	public:
		T evaluate(T& start, T& end, float progress) override;
	};

	template<typename T>
	class TweenEaseInExpo : public TweenEaseBase<T>
	{
	public:
		T evaluate(T& start, T& end, float progress) override;
	};

	template<typename T>
	class TweenEaseInOutExpo : public TweenEaseBase<T>
	{
	public:
		T evaluate(T& start, T& end, float progress) override;
	};

	template<typename T>
	class TweenEaseOutExpo : public TweenEaseBase<T>
	{
	public:
		T evaluate(T& start, T& end, float progress) override;
	};

	template<typename T>
	class TweenEaseInQuad : public TweenEaseBase<T>
	{
	public:
		T evaluate(T& start, T& end, float progress) override;
	};

	template<typename T>
	class TweenEaseInOutQuad : public TweenEaseBase<T>
	{
	public:
		T evaluate(T& start, T& end, float progress) override;
	};

	template<typename T>
	class TweenEaseOutQuad : public TweenEaseBase<T>
	{
	public:
		T evaluate(T& start, T& end, float progress) override;
	};

	template<typename T>
	class TweenEaseInQuart : public TweenEaseBase<T>
	{
	public:
		T evaluate(T& start, T& end, float progress) override;
	};

	template<typename T>
	class TweenEaseInOutQuart : public TweenEaseBase<T>
	{
	public:
		T evaluate(T& start, T& end, float progress) override;
	};

	template<typename T>
	class TweenEaseOutQuart : public TweenEaseBase<T>
	{
	public:
		T evaluate(T& start, T& end, float progress) override;
	};

	template<typename T>
	class TweenEaseInQuint : public TweenEaseBase<T>
	{
	public:
		T evaluate(T& start, T& end, float progress) override;
	};

	template<typename T>
	class TweenEaseInOutQuint : public TweenEaseBase<T>
	{
	public:
		T evaluate(T& start, T& end, float progress) override;
	};

	template<typename T>
	class TweenEaseOutQuint : public TweenEaseBase<T>
	{
	public:
		T evaluate(T& start, T& end, float progress) override;
	};

	template<typename T>
	class TweenEaseInSine : public TweenEaseBase<T>
	{
	public:
		T evaluate(T& start, T& end, float progress) override;
	};

	template<typename T>
	class TweenEaseInOutSine : public TweenEaseBase<T>
	{
	public:
		T evaluate(T& start, T& end, float progress) override;
	};

	template<typename T>
	class TweenEaseOutSine : public TweenEaseBase<T>
	{
	public:
		T evaluate(T& start, T& end, float progress) override;
	};

	//////////////////////////////////////////////////////////////////////////
	// template definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename T>
	T TweenEaseLinear<T>::evaluate(T& start, T& end, float progress)
	{
		return math::Linear::easeNone<float>(progress, 0.0f, 1.0f, 1.0f) * ( end - start ) + start;
	}

	template<typename T>
	T TweenEaseInCubic<T>::evaluate(T& start, T& end, float progress)
	{
		return math::Cubic::easeIn<float>(progress, 0.0f, 1.0f, 1.0f) * ( end - start ) + start;
	}

	template<typename T>
	T TweenEaseInOutCubic<T>::evaluate(T& start, T& end, float progress)
	{
		return math::Cubic::easeInOut<float>(progress, 0.0f, 1.0f, 1.0f) * ( end - start ) + start;
	}

	template<typename T>
	T TweenEaseOutCubic<T>::evaluate(T& start, T& end, float progress)
	{
		return math::Cubic::easeOut<float>(progress, 0.0f, 1.0f, 1.0f) * ( end - start ) + start;
	}

	template<typename T>
	T TweenEaseInBack<T>::evaluate(T& start, T& end, float progress)
	{
		return math::Back::easeIn<float>(progress, 0.0f, 1.0f, 1.0f) * ( end - start ) + start;
	}

	template<typename T>
	T TweenEaseInOutBack<T>::evaluate(T& start, T& end, float progress)
	{
		return math::Back::easeInOut<float>(progress, 0.0f, 1.0f, 1.0f) * ( end - start ) + start;
	}

	template<typename T>
	T TweenEaseOutBack<T>::evaluate(T& start, T& end, float progress)
	{
		return math::Back::easeOut<float>(progress, 0.0f, 1.0f, 1.0f) * ( end - start ) + start;
	}

	template<typename T>
	T TweenEaseInBounce<T>::evaluate(T& start, T& end, float progress)
	{
		return math::Bounce::easeIn<float>(progress, 0.0f, 1.0f, 1.0f) * ( end - start ) + start;
	}

	template<typename T>
	T TweenEaseInOutBounce<T>::evaluate(T& start, T& end, float progress)
	{
		return math::Bounce::easeInOut<float>(progress, 0.0f, 1.0f, 1.0f) * ( end - start ) + start;
	}

	template<typename T>
	T TweenEaseOutBounce<T>::evaluate(T& start, T& end, float progress)
	{
		return math::Bounce::easeOut<float>(progress, 0.0f, 1.0f, 1.0f) * ( end - start ) + start;
	}

	template<typename T>
	T TweenEaseInCirc<T>::evaluate(T& start, T& end, float progress)
	{
		return math::Circ::easeIn<float>(progress, 0.0f, 1.0f, 1.0f) * ( end - start ) + start;
	}

	template<typename T>
	T TweenEaseInOutCirc<T>::evaluate(T& start, T& end, float progress)
	{
		return math::Circ::easeInOut<float>(progress, 0.0f, 1.0f, 1.0f) * ( end - start ) + start;
	}

	template<typename T>
	T TweenEaseOutCirc<T>::evaluate(T& start, T& end, float progress)
	{
		return math::Circ::easeOut<float>(progress, 0.0f, 1.0f, 1.0f) * ( end - start ) + start;
	}

	template<typename T>
	T TweenEaseInElastic<T>::evaluate(T& start, T& end, float progress)
	{
		return math::Elastic::easeIn<float>(progress, 0.0f, 1.0f, 1.0f) * ( end - start ) + start;
	}

	template<typename T>
	T TweenEaseInOutElastic<T>::evaluate(T& start, T& end, float progress)
	{
		return math::Elastic::easeInOut<float>(progress, 0.0f, 1.0f, 1.0f) * ( end - start ) + start;
	}

	template<typename T>
	T TweenEaseOutElastic<T>::evaluate(T& start, T& end, float progress)
	{
		return math::Elastic::easeOut<float>(progress, 0.0f, 1.0f, 1.0f) * ( end - start ) + start;
	}

	template<typename T>
	T TweenEaseInExpo<T>::evaluate(T& start, T& end, float progress)
	{
		return math::Expo::easeIn<float>(progress, 0.0f, 1.0f, 1.0f) * ( end - start ) + start;
	}

	template<typename T>
	T TweenEaseInOutExpo<T>::evaluate(T& start, T& end, float progress)
	{
		return math::Expo::easeInOut<float>(progress, 0.0f, 1.0f, 1.0f) * ( end - start ) + start;
	}

	template<typename T>
	T TweenEaseOutExpo<T>::evaluate(T& start, T& end, float progress)
	{
		return math::Expo::easeOut<float>(progress, 0.0f, 1.0f, 1.0f) * ( end - start ) + start;
	}

	template<typename T>
	T TweenEaseInQuad<T>::evaluate(T& start, T& end, float progress)
	{
		return math::Quad::easeIn<float>(progress, 0.0f, 1.0f, 1.0f) * ( end - start ) + start;
	}

	template<typename T>
	T TweenEaseInOutQuad<T>::evaluate(T& start, T& end, float progress)
	{
		return math::Quad::easeInOut<float>(progress, 0.0f, 1.0f, 1.0f) * ( end - start ) + start;
	}

	template<typename T>
	T TweenEaseOutQuad<T>::evaluate(T& start, T& end, float progress)
	{
		return math::Quad::easeOut<float>(progress, 0.0f, 1.0f, 1.0f) * ( end - start ) + start;
	}

	template<typename T>
	T TweenEaseInQuart<T>::evaluate(T& start, T& end, float progress)
	{
		return math::Quart::easeIn<float>(progress, 0.0f, 1.0f, 1.0f) * ( end - start ) + start;
	}

	template<typename T>
	T TweenEaseInOutQuart<T>::evaluate(T& start, T& end, float progress)
	{
		return math::Quart::easeInOut<float>(progress, 0.0f, 1.0f, 1.0f) * ( end - start ) + start;
	}

	template<typename T>
	T TweenEaseOutQuart<T>::evaluate(T& start, T& end, float progress)
	{
		return math::Quart::easeOut<float>(progress, 0.0f, 1.0f, 1.0f) * ( end - start ) + start;
	}

	template<typename T>
	T TweenEaseInQuint<T>::evaluate(T& start, T& end, float progress)
	{
		return math::Quint::easeIn<float>(progress, 0.0f, 1.0f, 1.0f) * ( end - start ) + start;
	}

	template<typename T>
	T TweenEaseInOutQuint<T>::evaluate(T& start, T& end, float progress)
	{
		return math::Quint::easeInOut<float>(progress, 0.0f, 1.0f, 1.0f) * ( end - start ) + start;
	}

	template<typename T>
	T TweenEaseOutQuint<T>::evaluate(T& start, T& end, float progress)
	{
		return math::Quint::easeOut<float>(progress, 0.0f, 1.0f, 1.0f) * ( end - start ) + start;
	}

	template<typename T>
	T TweenEaseInSine<T>::evaluate(T& start, T& end, float progress)
	{
		return math::Sine::easeIn<float>(progress, 0.0f, 1.0f, 1.0f) * ( end - start ) + start;
	}

	template<typename T>
	T TweenEaseInOutSine<T>::evaluate(T& start, T& end, float progress)
	{
		return math::Sine::easeInOut<float>(progress, 0.0f, 1.0f, 1.0f) * ( end - start ) + start;
	}

	template<typename T>
	T TweenEaseOutSine<T>::evaluate(T& start, T& end, float progress)
	{
		return math::Sine::easeOut<float>(progress, 0.0f, 1.0f, 1.0f) * ( end - start ) + start;
	}
}
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <cmath>

/**
 * From https://github.com/jesusgollonet/ofpennereasing
 */

namespace nap
{
	namespace math
	{
		namespace Back
		{
			template <typename T>
			T easeIn(T t, T b, T c, T d);

			template <typename T>
			T easeOut(T t, T b, T c, T d);

			template <typename T>
			T easeInOut(T t, T b, T c, T d);
		}

		namespace Bounce
		{
			template <typename T>
			T easeIn(T t, T b, T c, T d);

			template <typename T>
			T easeOut(T t, T b, T c, T d);

			template <typename T>
			T easeInOut(T t, T b, T c, T d);
		}

		namespace Circ
		{

			template <typename T>
			T easeIn(T t, T b, T c, T d);

			template <typename T>
			T easeOut(T t, T b, T c, T d);

			template <typename T>
			T easeInOut(T t, T b, T c, T d);
		}

		namespace Cubic
		{

			template <typename T>
			T easeIn(T t, T b, T c, T d);

			template <typename T>
			T easeOut(T t, T b, T c, T d);

			template <typename T>
			T easeInOut(T t, T b, T c, T d);
		}

		namespace Elastic
		{
			template <typename T>
			T easeIn(T t, T b, T c, T d);

			template <typename T>
			T easeOut(T t, T b, T c, T d);

			template <typename T>
			T easeInOut(T t, T b, T c, T d);
		}

		namespace Expo
		{

			template <typename T>
			T easeIn(T t, T b, T c, T d);

			template <typename T>
			T easeOut(T t, T b, T c, T d);

			template <typename T>
			T easeInOut(T t, T b, T c, T d);
		}

		namespace Linear
		{

			template <typename T>
			T easeNone(T t, T b, T c, T d);

			template <typename T>
			T easeIn(T t, T b, T c, T d);

			template <typename T>
			T easeOut(T t, T b, T c, T d);

			template <typename T>
			T easeInOut(T t, T b, T c, T d);
		}

		namespace Quad
		{
			template <typename T>
			T easeIn(T t, T b, T c, T d);

			template <typename T>
			T easeOut(T t, T b, T c, T d);

			template <typename T>
			T easeInOut(T t, T b, T c, T d);
		}

		namespace Quart
		{
			template <typename T>
			T easeIn(T t, T b, T c, T d);

			template <typename T>
			T easeOut(T t, T b, T c, T d);

			template <typename T>
			T easeInOut(T t, T b, T c, T d);
		}

		namespace Quint
		{
			template <typename T>
			T easeIn(T t, T b, T c, T d);

			template <typename T>
			T easeOut(T t, T b, T c, T d);

			template <typename T>
			T easeInOut(T t, T b, T c, T d);
		}

		namespace Sine
		{

			template <typename T>
			T easeIn(T t, T b, T c, T d);

			template <typename T>
			T easeOut(T t, T b, T c, T d);

			template <typename T>
			T easeInOut(T t, T b, T c, T d);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Definitions
//////////////////////////////////////////////////////////////////////////

template <typename T>
T nap::math::Back::easeIn(T t, T b, T c, T d)
{
	T s = 1.70158f;
	T postFix = t /= d;
	return c * (postFix)*t * ((s + 1) * t - s) + b;
}


template <typename T>
T nap::math::Back::easeOut(T t, T b, T c, T d)
{
	T s = 1.70158f;
	return c * ((t = t / d - 1) * t * ((s + 1) * t + s) + 1) + b;
}

template <typename T>
T nap::math::Back::easeInOut(T t, T b, T c, T d)
{
	T s = 1.70158f;
	if ((t /= d / 2) < 1)
		return c / 2 * (t * t * (((s *= (1.525f)) + 1) * t - s)) + b;
	T postFix = t -= 2;
	return c / 2 * ((postFix)*t * (((s *= (1.525f)) + 1) * t + s) + 2) + b;
}

template <typename T>
T nap::math::Bounce::easeIn(T t, T b, T c, T d)
{
	return c - easeOut(d - t, 0, c, d) + b;
}

template <typename T>
T nap::math::Bounce::easeOut(T t, T b, T c, T d)
{
	if ((t /= d) < (1 / 2.75f)) {
		return c * (7.5625f * t * t) + b;
	}
	else if (t < (2 / 2.75f)) {
		T postFix = t -= (1.5f / 2.75f);
		return c * (7.5625f * (postFix)*t + .75f) + b;
	}
	else if (t < (2.5 / 2.75)) {
		T postFix = t -= (2.25f / 2.75f);
		return c * (7.5625f * (postFix)*t + .9375f) + b;
	}
	else {
		T postFix = t -= (2.625f / 2.75f);
		return c * (7.5625f * (postFix)*t + .984375f) + b;
	}
}

template <typename T>
T nap::math::Bounce::easeInOut(T t, T b, T c, T d)
{
	if (t < d / 2)
		return easeIn(t * 2, 0, c, d) * .5f + b;
	else
		return easeOut(t * 2 - d, 0, c, d) * .5f + c * .5f + b;
}

template <typename T>
T nap::math::Circ::easeIn(T t, T b, T c, T d)
{
	return -c * (sqrt(1 - (t /= d) * t) - 1) + b;
}

template <typename T>
T nap::math::Circ::easeOut(T t, T b, T c, T d)
{
	return c * sqrt(1 - (t = t / d - 1) * t) + b;
}

template <typename T>
T nap::math::Circ::easeInOut(T t, T b, T c, T d)
{
	if ((t /= d / 2) < 1)
		return -c / 2 * (sqrt(1 - t * t) - 1) + b;
	return c / 2 * (sqrt(1 - t * (t -= 2)) + 1) + b;
}

template <typename T>
T nap::math::Cubic::easeIn(T t, T b, T c, T d)
{
	return c * (t /= d) * t * t + b;
}

template <typename T>
T nap::math::Cubic::easeOut(T t, T b, T c, T d)
{
	return c * ((t = t / d - 1) * t * t + 1) + b;
}

template <typename T>
T nap::math::Cubic::easeInOut(T t, T b, T c, T d)
{
	if ((t /= d / 2) < 1)
		return c / 2 * t * t * t + b;
	return c / 2 * ((t -= 2) * t * t + 2) + b;
}

template <typename T>
T nap::math::Elastic::easeIn(T t, T b, T c, T d)
{
	if (t == 0)
		return b;
	if ((t /= d) == 1)
		return b + c;
	T p = d * .3f;
	T a = c;
	T s = p / 4;
	T postFix = a * pow(2, 10 * (t -= 1)); // this is a fix, again, with post-increment operators
	return -(postFix * sin((t * d - s) * (2 * M_PI) / p)) + b;
}

template <typename T>
T nap::math::Elastic::easeOut(T t, T b, T c, T d)
{
	if (t == 0)
		return b;
	if ((t /= d) == 1)
		return b + c;
	T p = d * .3f;
	T a = c;
	T s = p / 4;
	return (a * pow(2, -10 * t) * sin((t * d - s) * (2 * M_PI) / p) + c + b);
}

template <typename T>
T nap::math::Elastic::easeInOut(T t, T b, T c, T d)
{
	if (t == 0)
		return b;
	if ((t /= d / 2) == 2)
		return b + c;
	T p = d * (.3f * 1.5f);
	T a = c;
	T s = p / 4;

	if (t < 1) {
		T postFix = a * pow(2, 10 * (t -= 1)); // postIncrement is evil
		return -.5f * (postFix * sin((t * d - s) * (2 * M_PI) / p)) + b;
	}
	T postFix = a * pow(2, -10 * (t -= 1)); // postIncrement is evil
	return postFix * sin((t * d - s) * (2 * M_PI) / p) * .5f + c + b;
}

template <typename T>
T nap::math::Expo::easeIn(T t, T b, T c, T d)
{
	return (t == 0) ? b : c * pow(2, 10 * (t / d - 1)) + b;
}

template <typename T>
T nap::math::Expo::easeOut(T t, T b, T c, T d)
{
	return (t == d) ? b + c : c * (-pow(2, -10 * t / d) + 1) + b;
}

template <typename T>
T nap::math::Expo::easeInOut(T t, T b, T c, T d)
{
	if (t == 0)
		return b;
	if (t == d)
		return b + c;
	if ((t /= d / 2) < 1)
		return c / 2 * pow(2, 10 * (t - 1)) + b;
	return c / 2 * (-pow(2, -10 * --t) + 2) + b;
}

template <typename T>
T nap::math::Linear::easeNone(T t, T b, T c, T d)
{
	return c * t / d + b;
}

template <typename T>
T nap::math::Linear::easeIn(T t, T b, T c, T d)
{
	return c * t / d + b;
}

template <typename T>
T nap::math::Linear::easeOut(T t, T b, T c, T d)
{
	return c * t / d + b;
}

template <typename T>
T nap::math::Linear::easeInOut(T t, T b, T c, T d)
{
	return c * t / d + b;
}

template <typename T>
T nap::math::Quad::easeIn(T t, T b, T c, T d)
{
	return c * (t /= d) * t + b;
}

template <typename T>
T nap::math::Quad::easeOut(T t, T b, T c, T d)
{
	return -c * (t /= d) * (t - 2) + b;
}

template <typename T>
T nap::math::Quad::easeInOut(T t, T b, T c, T d)
{
	if ((t /= d / 2) < 1)
		return ((c / 2) * (t * t)) + b;
	return -c / 2 * (((t - 2) * (--t)) - 1) + b;
	/*
	originally return -c/2 * (((t-2)*(--t)) - 1) + b;

	I've had to swap (--t)*(t-2) due to diffence in behaviour in
	pre-increment operators between java and c++, after hours
	of joy
	*/
}

template <typename T>
T nap::math::Quart::easeIn(T t, T b, T c, T d)
{
	return c * (t /= d) * t * t * t + b;
}

template <typename T>
T nap::math::Quart::easeOut(T t, T b, T c, T d)
{
	return -c * ((t = t / d - 1) * t * t * t - 1) + b;
}

template <typename T>
T nap::math::Quart::easeInOut(T t, T b, T c, T d)
{
	if ((t /= d / 2) < 1)
		return c / 2 * t * t * t * t + b;
	return -c / 2 * ((t -= 2) * t * t * t - 2) + b;
}

template <typename T>
T nap::math::Quint::easeIn(T t, T b, T c, T d)
{
	return c * (t /= d) * t * t * t * t + b;
}

template <typename T>
T nap::math::Quint::easeOut(T t, T b, T c, T d)
{
	return c * ((t = t / d - 1) * t * t * t * t + 1) + b;
}

template <typename T>
T nap::math::Quint::easeInOut(T t, T b, T c, T d)
{
	if ((t /= d / 2) < 1)
		return c / 2 * t * t * t * t * t + b;
	return c / 2 * ((t -= 2) * t * t * t * t + 2) + b;
}

template <typename T>
T nap::math::Sine::easeIn(T t, T b, T c, T d)
{
	return -c * cos(t / d * (M_PI / 2)) + c + b;
}

template <typename T>
T nap::math::Sine::easeOut(T t, T b, T c, T d)
{
	return c * sin(t / d * (M_PI / 2)) + b;
}

template <typename T>
T nap::math::Sine::easeInOut(T t, T b, T c, T d)
{
	return -c / 2 * (cos(M_PI * t / d) - 1) + b;
}

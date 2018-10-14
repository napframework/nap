#pragma once

#include <QtCore/QtGlobal>

namespace napqt
{


	/**
	 * Evaluate a clamped bezier curve at time t
	 */
	template<typename T, typename U>
	T bezier(const T (& pts)[4], U t)
	{
		U u = 1 - t;
		U a = u * u * u;
		U b = 3 * u * u * t;
		U c = 3 * u * t * t;
		U d = t * t * t;
		return pts[0] * a + pts[1] * b + pts[2] * c + pts[3] * d;
	}

	/**
	 * Do a binary search on a curve for the t value at position x
	 * ie. Convert a time value in cartesian space into a parametric t value.
	 * @tparam T 2D point, its components should be of type U
	 * @tparam U floating point
	 * @param pts The curve points to evaluate
	 * @param x Time input value in cartesian space
	 * @param threshold The desired precision of the result
	 * @param maxIterations Max number of tries to get to the desired precision
	 * @return
	 */
	template<typename T, typename U>
	U tForX(const T (& pts)[4], U x, U threshold = 0.001, int maxIterations = 100)
	{
		U depth = 0.5;
		U t = 0.5;
		for (int i = 0; i < maxIterations; i++)
		{
			U dx = x - bezier(pts, t).x();
			U adx = qAbs(dx);
			if (adx <= threshold)
				break;

			depth *= 0.5;
			t += (dx > 0) ? depth : -depth;
		}
		return t;
	}

	/**
	 * Evaluate a curve segment
	 * @tparam T 2D point, its components should be of type U
	 * @tparam U floating point
	 * @param pts The curve points to evaluate
	 * @param x Time input value in cartesian space
	 * @return Vertical component of the evaluated curve segment at time x
	 */
	template<typename T, typename U>
	U evalCurveSegment(const T (& pts)[4], U x)
	{
		U t = tForX(pts, x);
		return bezier(pts, t).y();
	}
}
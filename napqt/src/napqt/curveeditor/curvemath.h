#pragma once

#include <QtGlobal>
#include <QtDebug>

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
	 * @tparam P 2D point, its components should be of type U
	 * @tparam F floating point
	 * @param pts The curve points to evaluate
	 * @param x Time input value in cartesian space
	 * @param threshold The desired precision of the result
	 * @param maxIterations Max number of tries to get to the desired precision
	 * @return
	 */
	template<typename P, typename F>
	F tForX(const P (& pts)[4], F x, F threshold = 0.0001, int maxIterations = 100)
	{
		F depth = 0.5;
		F t = 0.5;
		for (int i = 0; i < maxIterations; i++)
		{
			auto dx = x - bezier(pts, t).x();
			auto adx = qAbs(dx);
			if (adx <= threshold)
				break;

			depth *= 0.5;
			t += (dx > 0) ? depth : -depth;
		}
		return t;
	}

	/**
	 * Evaluate a curve segment
	 * @tparam P 2D point, its components should be of type U
	 * @tparam F floating point
	 * @param pts The curve points to evaluate
	 * @param x Time input value in cartesian space
	 * @return Vertical component of the evaluated curve segment at time x
	 */
	template<typename P, typename F>
	F evalCurveSegmentBezier(const P (& pts)[4], F x)
	{
		F t = tForX(pts, x);
		return bezier(pts, t).y();
	}

	template<typename P, typename F>
	F evalCurveSegmentLinear(const P (& pts)[4], F x)
	{
		const auto& a = pts[0];
		const auto& b = pts[3];
		F t = (x - a.x()) / (b.x() - a.x());
		return lerp(a.y(), b.y(), t);
	}

	template<typename P, typename F>
	F evalCurveSegmentStepped(const P (& pts)[4], F x)
	{
		return pts[0].y();
	}

}
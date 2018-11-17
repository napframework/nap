
namespace nap
{
	namespace math
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
				auto dx = x - bezier(pts, t).mTime;
				auto adx = qAbs(dx);
				if (adx <= threshold)
					break;

				depth *= 0.5;
				t += (dx > 0) ? depth : -depth;
			}
			return t;
		}

		/**
		 * Linearly interpolate between a and b at time t.
		 */
		template <typename T>
		inline T lerp(const T& a, const T& b, const T& t)
		{
			return a + t * (b - a);
		}

		/**
		 * Evaluate a curve segment using cubic bezier interpolation
		 *
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
			return bezier(pts, t).mValue;
		}

		/**
		 * Evaluate a curve segment using linear interpolation.
		 *
		 * @tparam P 2D point, its components should be of type U
		 * @tparam F floating point
		 * @param pts The curve points to evaluate
		 * @param x Time input value in cartesian space
		 * @return Vertical component of the evaluated curve segment at time x
		 */
		template<typename P, typename F>
		F evalCurveSegmentLinear(const P& a, const P& b, F x)
		{
			F t = (x - a.mTime) / (b.mTime - a.mTime);
			return lerp(a.mValue, b.mValue, t);
		}

	}
}


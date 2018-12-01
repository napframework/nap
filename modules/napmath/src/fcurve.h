#pragma once

#include <utility/dllexport.h>
#include <rtti/rtti.h>
#include <rtti/object.h>
#include <glm/glm.hpp>
#include <nap/resource.h>

namespace nap
{
	namespace math
	{
		/**
		 * Possible interpolations per curve segment
		 */
		enum class NAPAPI FCurveInterp : int
		{
			Bezier = 0,
			Linear,
			Stepped,
		};

		/**
		 * A time/value pair to be used in function curves.
		 * This is not a regular 2D vector because the components may have different types.
		 * @tparam T type of the time parameter
		 * @tparam V type of the value parameter
		 */
		template<typename T, typename V>
		struct NAPAPI FComplex
		{
			FComplex() = default;
			FComplex(const T& t, const V& value) : mTime(t), mValue(value) {}

			T mTime; // x-axis value
			V mValue; // y-axis value


			FComplex operator+(const FComplex& other) const
			{
				return {mTime + other.mTime, mValue + other.mValue};
			}

			FComplex operator-(const FComplex& other) const
			{
				return {mTime - other.mTime, mValue - other.mValue};
			}

			FComplex operator*(const T& other) const
			{
				return {mTime * other, mValue * other};
			}
		};

		/**
		 * Represents a single handle on an FCurve
		 * @tparam T type of the time parameter
		 * @tparam V type of the value parameter
		 */
		template<typename T, typename V>
		struct NAPAPI FCurvePoint
		{
			FCurvePoint() = default;
			FCurvePoint(const FComplex<T, V>& pos, const FComplex<T, V>& inTan, const FComplex<T, V>& outTan)
				: mPos(pos), mInTan(inTan), mOutTan(outTan) {}

			FComplex<T, V> mPos;
			FComplex<T, V> mInTan;
			FComplex<T, V> mOutTan;
			FCurveInterp mInterp = FCurveInterp::Bezier;

			// Non-essential to functionality, but necessary for editing
			bool mTangentsAligned = true;
		};


		/**
		 * A 1-D function curve that can be used to map one value to another,
		 * for example to animate something over time.
		 *
		 * @tparam T type of the time parameter
		 * @tparam V type of the value parameter
		 */
		template<typename T, typename V>
		class NAPAPI FunctionCurve : public Resource
		{
		RTTI_ENABLE(Resource)

		public:
			/**
			 * Default constructor, creates curve with a 0-1 ramp
			 */
			FunctionCurve();

			/**
			 * Evaluate this curve at time t
			 * @param t 
			 * @return the evaluated value of the curve at time t
			 */
			V evaluate(const T& t)
			{
				if (mPoints.empty())
					return V();

				// Ensure points are sorted before evaluation
				if (!mPointsSorted)
				{
					sortPoints();
					mPointsSorted = true;
				}

				const FCurvePoint<T, V>& firstPoint = mSortedPoints[0];
				if (t < firstPoint.mPos.mTime)
					return firstPoint.mPos.mValue;

				const FCurvePoint<T, V>& lastPoint = mSortedPoints[mSortedPoints.size() - 1];
				if (t >= lastPoint.mPos.mTime)
					return lastPoint.mPos.mValue;

				int idx = pointIndexAtTime(t);
				const FCurvePoint<T, V>& curr = mSortedPoints[idx];
				const FCurvePoint<T, V>& next = mSortedPoints[idx + 1];

				auto a = curr.mPos;
				auto b = a + curr.mOutTan;
				auto d = next.mPos;
				auto c = d + next.mInTan;

				limitOverhangPoints(a, b, c, d);

				switch (curr.mInterp)
				{
					case FCurveInterp::Bezier:
						return evalCurveSegmentBezier({a, b, c, d}, t);
					case FCurveInterp::Linear:
						return evalCurveSegmentLinear({a, b, c, d}, t);
					case FCurveInterp::Stepped:
						return evalCurveSegmentStepped({a, b, c, d}, t);
					default:
						assert(false);
				}

				return V();
			}


			/**
			 * Mark curve as dirty and ensure points are sorted on next evaluation
			 */
			void invalidate() { mPointsSorted = false; }

			/**
			 * The points that define this shape of this curve
			 */
			std::vector<FCurvePoint<T, V>> mPoints;

		private:
            /**
             * Evaluate a clamped bezier curve at time t
             */
            FComplex<T, V>  bezier(const FComplex<T, V> (& pts)[4], T t)
            {
                T u = 1 - t;
                T a = u * u * u;
                T b = 3 * u * u * t;
                T c = 3 * u * t * t;
                T d = t * t * t;
                return pts[0] * a + pts[1] * b + pts[2] * c + pts[3] * d;
            }

            /**
             * Do a binary search on a curve for the t value at position x
             * ie. Convert a time value in cartesian space into a parametric t value.
             * @tparam P 2D point, its components should be of type U
             * @tparam T floating point
             * @param pts The curve points to evaluate
             * @param x Time input value in cartesian space
             * @param threshold The desired precision of the result
             * @param maxIterations Max number of tries to get to the desired precision
             * @return
             */
            T tForX(const FComplex<T, V> (& pts)[4], T x, T threshold = 0.0001, int maxIterations = 100)
            {
                T depth = 0.5;
                T t = 0.5;
                for (int i = 0; i < maxIterations; i++)
                {
                    auto dx = x - bezier(pts, t).mTime;
                    auto adx = std::abs(dx);
                    if (adx <= threshold)
                        break;

                    depth *= 0.5;
                    t += (dx > 0) ? depth : -depth;
                }
                return t;
            }

            inline V lerp(const V& a, const V& b, const T& t)
            {
                return a + t * (b - a);
            }

            /**
             * Evaluate a curve segment
             * @tparam P 2D point, its components should be of type U
             * @tparam T floating point
             * @param pts The curve points to evaluate
             * @param x Time input value in cartesian space
             * @return Vertical component of the evaluated curve segment at time x
             */
            V evalCurveSegmentBezier(const nap::math::FComplex<T, V> (& pts)[4], T x)
            {
                T t = tForX(pts, x);
                return bezier(pts, t).mValue;
            }

            V evalCurveSegmentLinear(const FComplex<T, V> (& pts)[4], T x)
            {
                const auto& a = pts[0];
                const auto& b = pts[3];
                T t = (x - a.mTime) / (b.mTime - a.mTime);
                return lerp(a.mValue, b.mValue, t);
            }

            V evalCurveSegmentStepped(const FComplex<T, V>(& pts)[4], T x)
            {
                return pts[0].mValue;
            }

            /**
			 * Ensure points are sorted before evaluating the curve
			 */
			void sortPoints()
			{
				mSortedPoints.clear();
				for (int i = 0; i < mPoints.size(); i++)
					mSortedPoints.emplace_back(mPoints[i]);

				std::sort(mSortedPoints.begin(), mSortedPoints.end(),
						  [](const FCurvePoint<T, V>& lhs, const FCurvePoint<T, V>& rhs)
						  {
							  return lhs.mPos.mTime < rhs.mPos.mTime;
						  });
			}

			/**
			 * Grab the two points on either side of time
			 * @param t
			 * @param curr
			 * @param next
			 */
			int pointIndexAtTime(const T& t) const
			{
				for (size_t i = 0, len = mSortedPoints.size(); i < len; i++)
				{
					if (t < mSortedPoints[i].mPos.mTime)
					{
						if (i ==0)
							return 0;
						return static_cast<int>(i - 1);
					}
				}
				assert(false);
				return -1;
			}

			/**
			 * In case of Bezier interpolation, keep this curve segment from having multiple solutions
			 * @param x0
			 * @param x1
			 * @param x2
			 * @param x3
			 */
			void limitOverhang(const T& x0, T& x1, T& x2, const T& x3)
			{
				x1 = std::min(x1, x3);
				x2 = std::max(x2, x0);
			}

			/**
			 * In case of Bezier interpolation, keep this curve segment from having multiple solutions
			 * @param pa
			 * @param pb
			 * @param pc
			 * @param pd
			 */
			void limitOverhangPoints(const FComplex<T, V>& pa, FComplex<T, V>& pb, FComplex<T, V>& pc, const FComplex<T, V>& pd)
			{
				auto a = pa.mTime;
				auto b = pb.mTime;
				auto c = pc.mTime;
				auto d = pd.mTime;
				auto bb = b;
				auto cc = c;

				limitOverhang(a, b, c, d);

				// calculate ratios for both tangents
				auto rb = (b - a) / (bb - a);
				auto rc = (c - d) / (cc - d);

				// apply corrected times
				pb.mTime = b;
				pc.mTime = c;

				// we limited time, keep the value on the tangent line for c1 continuity
				pb.mValue = ((pb.mValue - pa.mValue) * rb) + pa.mValue;
				pc.mValue = ((pc.mValue - pd.mValue) * rc) + pd.mValue;
			}

			mutable std::vector<FCurvePoint<T, V>> mSortedPoints;
			bool mPointsSorted = false; // keep track point sort state for proper curve eval

		};


		//////////////////////////////////////////////////////////////////////////
		// Alias some type specializations
		//////////////////////////////////////////////////////////////////////////

		using FloatFComplex = FComplex<float, float>;
		using Vec2FComplex = FComplex<float, glm::vec2>;
		using Vec3FComplex = FComplex<float, glm::vec3>;
		using Vec4FComplex = FComplex<float, glm::vec4>;

		using FloatFCurve = FunctionCurve<float, float>;
		using Vec2FCurve  = FunctionCurve<float, glm::vec2>;
		using Vec3FCurve  = FunctionCurve<float, glm::vec3>;
		using Vec4FCurve  = FunctionCurve<float, glm::vec4>;

		using FloatFCurvePoint = FCurvePoint<float, float>;
		using Vec2FCurvePoint = FCurvePoint<float, glm::vec2>;
		using Vec3FCurvePoint = FCurvePoint<float, glm::vec3>;
		using Vec4FCurvePoint = FCurvePoint<float, glm::vec4>;


        //////////////////////////////////////////////////////////////////////////
        // MSVC needs explicit template specialization exports
        //////////////////////////////////////////////////////////////////////////
		template class NAPAPI FunctionCurve<float, float>;
        template class NAPAPI FunctionCurve<float, glm::vec2>;
//        template class NAPAPI FunctionCurve<float, glm::vec3>;
//        template class NAPAPI FunctionCurve<float, glm::vec4>;
	}
}

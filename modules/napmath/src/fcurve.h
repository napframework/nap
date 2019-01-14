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
		enum class ECurveInterp : int
		{
			Bezier = 0,				///< Bezier style interpolation
			Linear,					///< Linear style interpolation
			Stepped					///< Stepped interpolated (hold previous value until next curve point
		};


		/**
		 * A time/value pair to be used in function curves.
		 * This is not a regular 2D vector because the components may have different types.
		 * @param T type of the time parameter
		 * @param V type of the value parameter
		 */
		template<typename T, typename V>
		struct FComplex
		{
			/**
			 * Cfreate a time/value pair with time at 0 and default values for V
			 */
			FComplex() = default;

			/**
			 * Create a time/value pair
			 * @param t the time
			 * @param value the value
			 */
			FComplex(const T& t, const V& value) : mTime(t), mValue(value) {}

			T mTime;	///<  Time mapped to the x axis
			V mValue;	///<  Value mapped to the y axis

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
		 * @param T type of the time parameter
		 * @param V type of the value parameter
		 */
		template<typename T, typename V>
		struct FCurvePoint
		{
			/**
			 * Create a point at time 0 and zeroed values. Tangents have reasonable default values.
			 */
			FCurvePoint() = default;

			/**
			 *
			 * @param pos Position of the handle
			 * @param inTan Left tangent position, relative to point position
			 * @param outTan Right tangent position, relative to point position
			 */
			FCurvePoint(const FComplex<T, V>& pos, const FComplex<T, V>& inTan, const FComplex<T, V>& outTan)
				: mPos(pos), mInTan(inTan), mOutTan(outTan) {}

			FComplex<T, V> mPos;		///< Position of the handle
			FComplex<T, V> mInTan;		///< Left tangent position, relative to mPos
			FComplex<T, V> mOutTan;		///< Right tangent position, relative to mPos
			ECurveInterp mInterp = ECurveInterp::Bezier;

			bool mTangentsAligned = true;	///< Non-essential to functionality, but necessary for editing
		};


		//////////////////////////////////////////////////////////////////////////
		// 1-D Curve Resource
		//////////////////////////////////////////////////////////////////////////

		/**
		 * A 1-D curve that can be used to map one value to another.
		 * This resource can be used to animate a value over time.
		 * The times and values of this curve are unbounded.
		 * @param T type of the time parameter
		 * @param V type of the value parameter
		 */
		template<typename T, typename V>
		class FCurve : public Resource
		{
		RTTI_ENABLE(Resource)
		public:
			/**
			 * Default constructor
			 */
			FCurve();

			/**
			 * Destructor
			 */
			~FCurve() override { mSortedPoints.clear(); }


			/**
			 * Evaluate this curve at time t
			 * @param t point in time to get the interpolated curve value for
			 * @return the evaluated value of the curve at time t
			 */
			V evaluate(const T& t);

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
            FComplex<T, V>  bezier(const FComplex<T, V> (& pts)[4], T t);

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
            T tForX(const FComplex<T, V> (& pts)[4], T x, T threshold = 0.0001, int maxIterations = 100);

			/**
			 * linear interpolate value a to b using a value of t (0-1)
			 *
			 * @param a the first input value
			 * @param b the second input value
			 * @param t the interpolation value (0-1)
			 */
            V lerp(const V& a, const V& b, const T& t);

            /**
             * Evaluate a curve segment using cubic bezier interpolation
             *
             * @tparam P 2D point, its components should be of type U
             * @tparam T floating point
             * @param pts The curve points to evaluate
             * @param x Time input value in cartesian space
             * @return Vertical component of the evaluated curve segment at time x
             */
            V evalCurveSegmentBezier(const nap::math::FComplex<T, V> (& pts)[4], T x);

			/**
			 * Evaluate a curve segment using linear interpolation
			 *
			 * @tparam P 2D point, its components should be of type U
			 * @tparam T floating point
			 * @param pts The curve points to evaluate
			 * @param x Time input value in cartesian space
			 * @return Vertical component of the evaluated curve segment at time x
			 */
            V evalCurveSegmentLinear(const FComplex<T, V> (& pts)[4], T x);

			/**
			 * Evaluate a curve segment using stepped interpolation
			 *
			 * @tparam P 2D point, its components should be of type U
			 * @tparam T floating point
			 * @param pts The curve points to evaluate
			 * @param x Time input value in cartesian space
			 * @return Vertical component of the evaluated curve segment at time x
			 */
            V evalCurveSegmentStepped(const FComplex<T, V>(& pts)[4], T x);

            /**
			 * Ensure points are sorted before evaluating the curve
			 */
			void sortPoints();

			/**
			 * Grab the index of the point before or on time t
			 *
			 * @param t the time of the point index
			 */
			int pointIndexAtTime(const T& t) const;

			/**
			 * In case of Bezier interpolation, keep this curve segment from having multiple solutions,
			 *
			 * @param x0 time 0 of curve segment
			 * @param x1 time 1 of curve segment
			 * @param x2 time 2 of curve segment
			 * @param x3 time 3 of curve segment
			 */
			void limitOverhang(const T& x0, T& x1, T& x2, const T& x3);

			/**
			 * In case of Bezier interpolation, keep this curve segment from having multiple solutions
			 *
			 * @param pa point 0 of curve segment
			 * @param pb point 1 of curve segment
			 * @param pc point 2 of curve segment
			 * @param pd point 3 of curve segment
			 */
			void limitOverhangPoints(const FComplex<T, V>& pa, FComplex<T, V>& pb, FComplex<T, V>& pc, const FComplex<T, V>& pd);

			mutable std::vector<FCurvePoint<T, V>*> mSortedPoints;				///< sorted pointers to the original (unsorted) points in mPoints
			bool mPointsSorted = false;											///< keep track point sort state for proper curve eval
		};


		//////////////////////////////////////////////////////////////////////////
		// Type definitions for all supported curve types
		//////////////////////////////////////////////////////////////////////////

		using FloatFComplex		= FComplex<float, float>;
		using Vec2FComplex		= FComplex<float, glm::vec2>;
		using Vec3FComplex		= FComplex<float, glm::vec3>;
		using Vec4FComplex		= FComplex<float, glm::vec4>;

		using FloatFCurve		= FCurve<float, float>;
		using Vec2FCurve		= FCurve<float, glm::vec2>;
		using Vec3FCurve		= FCurve<float, glm::vec3>;
		using Vec4FCurve		= FCurve<float, glm::vec4>;

		using FloatFCurvePoint	= FCurvePoint<float, float>;
		using Vec2FCurvePoint	= FCurvePoint<float, glm::vec2>;
		using Vec3FCurvePoint	= FCurvePoint<float, glm::vec3>;
		using Vec4FCurvePoint	= FCurvePoint<float, glm::vec4>;


		//////////////////////////////////////////////////////////////////////////
		// explicit MSVC template specialization exports
		//////////////////////////////////////////////////////////////////////////
		template class NAPAPI FCurve<float, float>;
		template class NAPAPI FCurve<float, glm::vec2>;
		// template class NAPAPI FCurve<float, glm::vec3>;
		// template class NAPAPI FCurve<float, glm::vec4>;


		//////////////////////////////////////////////////////////////////////////
		// Template Definitions
		//////////////////////////////////////////////////////////////////////////

		template<typename T, typename V>
		V nap::math::FCurve<T, V>::evaluate(const T& t)
		{
			if (mPoints.empty())
				return V();

			// Ensure points are sorted before evaluation
			if (!mPointsSorted)
			{
				sortPoints();
				mPointsSorted = true;
			}

			const FCurvePoint<T, V>* firstPoint = mSortedPoints[0];
			if (t < firstPoint->mPos.mTime)
				return firstPoint->mPos.mValue;

			const FCurvePoint<T, V>* lastPoint = mSortedPoints[mSortedPoints.size() - 1];
			if (t >= lastPoint->mPos.mTime)
				return lastPoint->mPos.mValue;

			int idx = pointIndexAtTime(t);
			const FCurvePoint<T, V>* curr = mSortedPoints[idx];
			const FCurvePoint<T, V>* next = mSortedPoints[idx + 1];

			auto a = curr->mPos;
			auto b = a + curr->mOutTan;
			auto d = next->mPos;
			auto c = d + next->mInTan;

			limitOverhangPoints(a, b, c, d);
			switch (curr->mInterp)
			{
			case ECurveInterp::Bezier:
				return evalCurveSegmentBezier({ a, b, c, d }, t);
			case ECurveInterp::Linear:
				return evalCurveSegmentLinear({ a, b, c, d }, t);
			case ECurveInterp::Stepped:
				return evalCurveSegmentStepped({ a, b, c, d }, t);
			default:
				assert(false);
			}

			return V();
		}

		template<typename T, typename V>
		nap::math::FComplex<T, V> nap::math::FCurve<T, V>::bezier(const FComplex<T, V>(&pts)[4], T t)
		{
			T u = 1 - t;
			T a = u * u * u;
			T b = 3 * u * u * t;
			T c = 3 * u * t * t;
			T d = t * t * t;
			return pts[0] * a + pts[1] * b + pts[2] * c + pts[3] * d;
		}


		template<typename T, typename V>
		T nap::math::FCurve<T, V>::tForX(const FComplex<T, V>(&pts)[4], T x, T threshold /*= 0.0001*/, int maxIterations /*= 100*/)
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

		template<typename T, typename V>
		V nap::math::FCurve<T, V>::lerp(const V& a, const V& b, const T& t)
		{
			return a + t * (b - a);
		}

		template<typename T, typename V>
		V nap::math::FCurve<T, V>::evalCurveSegmentBezier(const nap::math::FComplex<T, V>(&pts)[4], T x)
		{
			T t = tForX(pts, x);
			return bezier(pts, t).mValue;
		}

		template<typename T, typename V>
		V nap::math::FCurve<T, V>::evalCurveSegmentLinear(const FComplex<T, V>(&pts)[4], T x)
		{
			const auto& a = pts[0];
			const auto& b = pts[3];
			T t = (x - a.mTime) / (b.mTime - a.mTime);
			return lerp(a.mValue, b.mValue, t);
		}


		template<typename T, typename V>
		V nap::math::FCurve<T, V>::evalCurveSegmentStepped(const FComplex<T, V>(&pts)[4], T x)
		{
			return pts[0].mValue;
		}

		template<typename T, typename V>
		void nap::math::FCurve<T, V>::sortPoints()
		{
			mSortedPoints.clear();
			for (int i = 0; i < mPoints.size(); i++)
				mSortedPoints.emplace_back(&mPoints[i]);

			std::sort(mSortedPoints.begin(), mSortedPoints.end(),
				[](const FCurvePoint<T, V>* lhs, const FCurvePoint<T, V>* rhs)
			{
				return lhs->mPos.mTime < rhs->mPos.mTime;
			});
		}

		template<typename T, typename V>
		int nap::math::FCurve<T, V>::pointIndexAtTime(const T& t) const
		{
			for (size_t i = 0, len = mSortedPoints.size(); i < len; i++)
			{
				if (t < mSortedPoints[i]->mPos.mTime)
				{
					if (i == 0)
						return 0;
					return static_cast<int>(i - 1);
				}
			}
			assert(false);
			return -1;
		}

		template<typename T, typename V>
		void nap::math::FCurve<T, V>::limitOverhang(const T& x0, T& x1, T& x2, const T& x3)
		{
			x1 = std::min(x1, x3);
			x2 = std::max(x2, x0);
		}

		template<typename T, typename V>
		void nap::math::FCurve<T, V>::limitOverhangPoints(const FComplex<T, V>& pa, FComplex<T, V>& pb, FComplex<T, V>& pc, const FComplex<T, V>& pd)
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
	}
}

#pragma once

#include <utility/dllexport.h>
#include <rtti/rtti.h>
#include <rtti/object.h>
#include <glm/glm.hpp>
#include <nap/resource.h>

#include "curvefunctions.h"

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
		 * @tparam U type of the value parameter
		 */
		template<typename T, typename U>
		struct NAPAPI FComplex
		{
			FComplex() = default;
			FComplex(const T& time, const U& value) : mTime(time), mValue(value) {}

			T mTime; // x-axis value
			U mValue; // y-axis value


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
		 * @tparam U type of the value parameter
		 */
		template<typename T, typename U>
		struct NAPAPI FCurvePoint
		{
			using Fc = FComplex<T, U>;

			FCurvePoint() = default;
			FCurvePoint(const Fc& pos, const Fc& inTan, const Fc& outTan)
				: mPos(pos), mInTan(inTan), mOutTan(outTan) {}

			Fc mPos;
			Fc mInTan;
			Fc mOutTan;
			FCurveInterp mInterp = FCurveInterp::Bezier;

			// Non-essential to functionality, but necessary for editing
			bool mTangentsAligned = true;
		};

		/**
		 * A 1-D function curve that can be used to map one value to another,
		 * for example to animate something over time.
		 *
		 * @tparam T type of the time parameter
		 * @tparam U type of the value parameter
		 */
		template<typename T, typename U>
		class NAPAPI FCurve : public Resource
		{
		RTTI_ENABLE(Resource)
			using Pt = FCurvePoint<T, U>;
			using Fc = FComplex<T, U>;

		public:
			/**
			 * Default constructor, creates curve with a 0-1 ramp
			 */
			FCurve();

			/**
			 * Evaluate this curve at time t
			 * @param t 
			 * @return the evaluated value of the curve at time t
			 */
			T evaluate(const U& t);

			/**
			 * The points that define this curve's shape
			 */
			std::vector<FCurvePoint<T, U>> mPoints;

		private:
			/**
			 * Ensure points are sorted before evaluating the curve
			 */
			void sortPoints();

			/**
			 * Grab the two points on either side of time
			 * @param time
			 * @param curr
			 * @param next
			 */
			int pointIndexAtTime(const U& time) const;

			/**
			 * In case of Bezier interpolation, keep this curve segment from having multiple solutions
			 * @param x0
			 * @param x1
			 * @param x2
			 * @param x3
			 */
			void limitOverhang(const U& x0, U& x1, U& x2, const U& x3);

			/**
			 * In case of Bezier interpolation, keep this curve segment from having multiple solutions
			 * @param pa
			 * @param pb
			 * @param pc
			 * @param pd
			 */
			void limitOverhangPoints(const Fc& pa, Fc& pb, Fc& pc, const Fc& pd);

			mutable std::vector<FCurvePoint<T, U>> mSortedPoints;
		};


		//////////////////////////////////////////////////////////////////////////
		// Template definitions
		//////////////////////////////////////////////////////////////////////////

		template<typename T, typename U>
		void FCurve<T, U>::limitOverhangPoints(const FCurve::Fc& pa, FCurve::Fc& pb, FCurve::Fc& pc, const FCurve::Fc& pd)
		{
			auto a = pa.mTime;
			auto b = pb.mTime;
			auto c = pc.mTime;
			auto d = pd.mTime;
			auto bb = b;
			auto cc = c;

			limitOverhang(a, b, c, d);

			// Calculate ratios for both tangents
			auto rb = (b - a) / (bb - a);
			auto rc = (c - d) / (cc - d);

			// Apply corrected times
			pb.mTime = b;
			pc.mTime = c;

			// we limited time, keep the value on the tangent line for c1 continuity
			pb.mValue = ((pb.mValue - pa.mValue) * rb) + pa.mValue;
			pc.mValue = ((pc.mValue - pd.mValue) * rc) + pd.mValue;
		}

		template<typename T, typename U>
		void FCurve<T, U>::limitOverhang(const U& x0, U& x1, U& x2, const U& x3)
		{
			x1 = std::min(x1, x3);
			x2 = std::max(x2, x0);
		}

		template<typename T, typename U>
		int FCurve<T, U>::pointIndexAtTime(const U& time) const
		{
			for (size_t i = 0, len = mSortedPoints.size(); i < len; i++)
				if (time > mSortedPoints[i].mPos.mTime)
					return static_cast<int>(i);
			assert(false);
			return -1;
		}

		template<typename T, typename U>
		void FCurve<T, U>::sortPoints()
		{
			mSortedPoints.clear();
			for (int i = 0; i < mPoints.size(); i++)
				mSortedPoints.emplace_back(mPoints[i]);

			std::sort(mSortedPoints.begin(), mSortedPoints.end(),
					  [](const FCurvePoint<T, U>& lhs, const FCurvePoint<T, U>& rhs)
					  {
						  return lhs.mPos.mTime < rhs.mPos.mTime;
					  });
		}

		template<typename T, typename U>
		T FCurve<T, U>::evaluate(const U& time)
		{
			if (mPoints.empty())
				return T();

			sortPoints(); // TODO: Optimize or move this call somewhere else

			const Pt& firstPoint = mSortedPoints[0];
			if (time < firstPoint.mPos.mTime)
				return firstPoint.mPos.mValue;

			const Pt& lastPoint = mSortedPoints[mSortedPoints.size() - 1];
			if (time >= lastPoint.mPos.mTime)
				return lastPoint.mPos.mValue;

			int idx = pointIndexAtTime(time);
			const Pt& curr = mSortedPoints[idx];
			const Pt& next = mSortedPoints[idx + 1];

			auto a = curr.mPos;
			auto b = a + curr.mOutTan;
			auto d = next.mPos;
			auto c = d + next.mInTan;

			limitOverhangPoints(a, b, c, d);

			switch (curr.mInterp)
			{
				case FCurveInterp::Bezier:
					return evalCurveSegmentBezier({a, b, c, d}, time);
				case FCurveInterp::Linear:
					return evalCurveSegmentLinear({a, b, c, d}, time);
				case FCurveInterp::Stepped:
					return evalCurveSegmentStepped({a, b, c, d}, time);
				default:
					assert(false);
			}

			return 0;
		}

		//////////////////////////////////////////////////////////////////////////
		// Alias some type specializations
		//////////////////////////////////////////////////////////////////////////

		using FloatFComplex = FComplex<float, float>;
		using Vec2FComplex = FComplex<float, glm::vec2>;
		using Vec3FComplex = FComplex<float, glm::vec3>;
		using Vec4FComplex = FComplex<float, glm::vec4>;

		using FloatFCurve = FCurve<float, float>;
		using Vec2FCurve  = FCurve<float, glm::vec2>;
		using Vec3FCurve  = FCurve<float, glm::vec3>;
		using Vec4FCurve  = FCurve<float, glm::vec4>;

		using FloatFCurvePoint = FCurvePoint<float, float>;
		using Vec2FCurvePoint = FCurvePoint<float, glm::vec2>;
		using Vec3FCurvePoint = FCurvePoint<float, glm::vec3>;
		using Vec4FCurvePoint = FCurvePoint<float, glm::vec4>;

	}
}

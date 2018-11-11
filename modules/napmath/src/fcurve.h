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
		enum class NAPAPI FCurveInterp : int {
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

			T mTime; // x-axis value
			U mValue; // y-axis value
		};

		/**
		 * Represents a single handle on an FCurve
		 * @tparam T type of the time parameter
		 * @tparam U type of the value parameter
		 */
		template<typename T, typename U>
		struct NAPAPI FCurvePoint
		{
			FCurvePoint() = default;

			FComplex<T, U> mPos;
			FComplex<T, U> mInTan;
			FComplex<T, U> mOutTan;
			FCurveInterp mInterp = FCurveInterp::Bezier;

			// Non-essential to functionality, but necessary for editing
			bool mTangentsAligned = true;
		};

		/**
		 * Compare functor to be used to keep curve points sorted for faster evaluation
		 * @tparam T type of the time parameter
		 * @tparam U type of the value parameter
		 */
		template<typename T, typename U>
		struct PointCompare
		{
			bool operator()(const FCurvePoint<T, U>& lhs, const FCurvePoint<T, U>& rhs)
			{
				return lhs.mPos.mTime < rhs.mPos.mTime;
			}
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
			FCurve() = default;
			
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
			void pointsAtTime(const U& time, Pt*& curr, Pt*& next) const;

			/**
			 * In case of Bezier interpolation, keep this curve segment from having multiple solutions
			 * @param x0
			 * @param x1
			 * @param x2
			 * @param x3
			 */
			void limitOverhang(U& x0, U& x1, U& x2, U& x3);

			/**
			 * In case of Bezier interpolation, keep this curve segment from having multiple solutions
			 * @param pa
			 * @param pb
			 * @param pc
			 * @param pd
			 */
			void limitOverhangPoints(Fc& pa, Fc& pb, Fc& pc, Fc& pd);

			std::set<FCurvePoint<T, U>, PointCompare<T, U>> mSortedPoints;
		};


		//////////////////////////////////////////////////////////////////////////
		// Template definitions
		//////////////////////////////////////////////////////////////////////////

		template<typename T, typename U>
		void FCurve<T, U>::limitOverhangPoints(FCurve::Fc& pa, FCurve::Fc& pb, FCurve::Fc& pc, FCurve::Fc& pd)
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
		void FCurve<T, U>::limitOverhang(U& x0, U& x1, U& x2, U& x3)
		{
			x1 = min(x1, x3);
			x2 = qMax(x2, x0);
		}

		template<typename T, typename U>
		void FCurve<T, U>::pointsAtTime(const U& time, FCurve::Pt*& curr, FCurve::Pt*& next) const
		{
			Pt* lastPt = mSortedPoints.cbegin();
			for (auto pt : mSortedPoints)
			{
				if (pt.mPos.mTime > time)
				{
					curr = lastPt;
					next = pt;
					return;
				}
				lastPt = pt;
			}
			assert(false);
		}

		template<typename T, typename U>
		void FCurve<T, U>::sortPoints()
		{
			mSortedPoints.clear();
			mSortedPoints.emplace(mPoints);
		}

		template<typename T, typename U>
		T FCurve<T, U>::evaluate(const U& time)
		{
			sortPoints(); // TODO: Optimize or move this call somewhere else

			Pt* firstPoint = mSortedPoints[0];
			if (time < firstPoint->mPos.mTime)
				return firstPoint->mPos.mValue;

			Pt* lastPoint = mSortedPoints[mSortedPoints.size() - 1];
			if (time >= lastPoint->mPos.mTime)
				return lastPoint->mPos.mValue;

			Pt* curr = nullptr;
			Pt* next = nullptr;
			pointsAtTime(time, curr, next);
			assert(curr);
			assert(next);
			auto a = curr->mPos;
			auto b = a + curr->mOutTan;
			auto d = next->mPos;
			auto c = d + next->mInTan;

			limitOverhangPoints(a, b, c, d);

			switch (curr->mInterp)
			{
				case FCurveInterp::Bezier:
					evalCurveSegmentBezier({a, b, c, d}, time);
					break;
				case FCurveInterp::Linear:
					evalCurveSegmentLinear({a, b, c, d}, time);
					break;
				case FCurveInterp::Stepped:
					evalCurveSegmentStepped({a, b, c, d}, time);
					break;
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

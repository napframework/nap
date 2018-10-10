#pragma once

#include "timeline.h"
#include <glm/glm.hpp>

namespace nap
{
	using CurvePoint = glm::vec2;

	/**
	 * Reference implementation of a timeline event animating a propert from one value to another
	 */
	class LerpEvent : public TimelineEvent
	{
	public:
		void start() override;
		void end() override;
		void update(double deltaTime) override;

		double mStartValue = 0;
		double mEndValue = 1;

		rtti::ObjectPtrBase mTargetObject; // The object containing the property to be animated
		rtti::Property mTargetProperty; // The property to be animated
	private:
		void setTargetValue(double targetValue);
		double mLocalTime;
	};

	/**
	 * A resource representing a function curve, defined by a set of key points.
	 * NOTE: If the points need to change at run-time, this curve needs instance replication
	 */
	class CurveResource
	{
	public:
		enum Interpolation { Linear, Quartic, Cubic };

		/**
		 * Retrieve the value of the curve at time t.
		 *
		 * @param t The time at which to retrieve the curve value. (non-normalized)
		 * @return The evaluated value of the curve.
		 */
		double evaluate(double t);

		/**
		 * How to interpolate between the points
		 */
		Interpolation mInterpolation;

		/**
		 *
		 */
		std::vector<CurvePoint> mPoints;
	private:

	};

	/**
	 * An event that animates a property according to an animation curve. Needs instance replication
	 */
	class CurveEvent : public TimelineEvent
	{
	public:
		void start() override;
		void end() override;
		void update(double deltaTime) override;

		CurveResource mCurveResource;
		rtti::ObjectPtrBase mTargetObject; // The object containing the property to be animated
		rtti::Property mTargetProperty; // The property to be animated
	private:
		void setTargetValue(double targetValue);
		double mLocalTime;
	};


}
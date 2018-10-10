#include <mathutils.h>
#include "timelinedemo.h"


void nap::LerpEvent::start()
{
	mLocalTime = 0;
}

void nap::LerpEvent::end()
{
	setTargetValue(mEndValue);
}

void nap::LerpEvent::update(double deltaTime)
{
	mLocalTime += deltaTime;
	double t = mLocalTime / (endTime() - startTime());
	double targetValue = nap::math::lerp(mStartValue, mEndValue, t);
	setTargetValue(targetValue);
}

void nap::LerpEvent::setTargetValue(double targetValue)
{
	// TODO: Apply current value to target property
}

double nap::CurveResource::evaluate(double t)
{
	// TODO: Evaluate curve here
	return 0;
}

void nap::CurveEvent::start()
{
	mLocalTime = 0;
}

void nap::CurveEvent::end()
{
	double curveValue = mCurveResource.evaluate(endTime());
	setTargetValue(curveValue);
}

void nap::CurveEvent::update(double deltaTime)
{
	mLocalTime += deltaTime;
	double curveValue = mCurveResource.evaluate(mLocalTime);
	setTargetValue(curveValue);
}

void nap::CurveEvent::setTargetValue(double targetValue)
{
	// TODO: Apply current value to target property
}

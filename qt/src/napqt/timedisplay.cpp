/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "timedisplay.h"
#include <QtMath>
#include <QVector>
#include <QMap>

using namespace nap::qt;

static const qreal MILLISECOND = 0.001;
static const qreal SECOND = 1.0;
static const qreal MINUTE = SECOND * 60;
static const qreal HOUR = MINUTE * 60;
static const qreal DAY = HOUR * 24;
static const qreal WEEK = DAY * 7;
static const qreal YEAR = WEEK * 52;

static const qreal FRAME_12 = 1.0 / 12.0;
static const qreal FRAME_24 = 1.0 / 24.0;
static const qreal FRAME_25 = 1.0 / 25.0;
static const qreal FRAME_30 = 1.0 / 30.0;
static const qreal FRAME_60 = 1.0 / 12.0;

static const QMap<int, QVector<qreal>> INTERVALS_FRAME = {
		{12, {
					 FRAME_12,
					 FRAME_12 * 2,
					 FRAME_12 * 6
			 }},
		{24, {
					 FRAME_24,
					 FRAME_24 * 2,
					 FRAME_24 * 6,
					 FRAME_24 * 12,
			 }},
		{25, {
					 FRAME_25,
					 FRAME_25 * 5,
			 }},
		{30, {
					 FRAME_30,
					 FRAME_30 * 2,
					 FRAME_30 * 5,
					 FRAME_30 * 15,
			 }},
		{60, {
					 FRAME_60,
					 FRAME_60 * 2,
					 FRAME_60 * 5,
					 FRAME_60 * 15,
					 FRAME_60 * 30,
			 }},
};

static const QVector<qreal> INTERVALS_SMPTE = {
		SECOND,
		SECOND * 2,
		SECOND * 5,
		SECOND * 10,
		SECOND * 20,
		SECOND * 30,
		MINUTE,
		MINUTE * 2,
		MINUTE * 5,
		MINUTE * 10,
		MINUTE * 15,
		MINUTE * 30,
		HOUR,
		HOUR * 2,
		HOUR * 5,
		HOUR * 10,
		HOUR * 20,
		HOUR * 50,
		HOUR * 100,
};

static const QVector<qreal> INTERVALS_TIME = {
		MILLISECOND,
		MILLISECOND * 2,
		MILLISECOND * 5,
		MILLISECOND * 10,
		MILLISECOND * 20,
		MILLISECOND * 50,
		MILLISECOND * 100,
		MILLISECOND * 200,
		MILLISECOND * 500,
		SECOND,
		SECOND * 2,
		SECOND * 5,
		SECOND * 10,
		SECOND * 30,
		MINUTE,
		MINUTE * 2,
		MINUTE * 5,
		MINUTE * 10,
		MINUTE * 15,
		MINUTE * 20,
		MINUTE * 30,
		HOUR,
		HOUR * 2,
		HOUR * 3,
		HOUR * 6,
		HOUR * 12,
		DAY,
		WEEK,
		WEEK * 4,
		YEAR,
};

static const QMap<qreal, QString> TIME_SUFFIXES = {
		{MILLISECOND, "ms"},
		{SECOND,      "s"},
		{MINUTE,      "m"},
		{HOUR,        "h"},
		{DAY,         "d"},
		{WEEK,        "w"},
		{YEAR,        "yr"},
};

bool calcInterval(const qreal windowSize, const qreal viewSize, const qreal minStepSize,
				  const QVector<qreal>& intervals, qreal& outValue)
{
	for (const auto& ival : intervals)
	{
		qreal steps = windowSize / ival;
		qreal stepSize = viewSize / steps;
		if (stepSize > minStepSize)
		{
			outValue = ival;
			return true;
		}
	}
	return false;
}


void IntervalDisplay::setHatchSpacing(qreal minor, qreal major)
{
	mMinorHatchSpacing = minor;
	mMajorHatchSpacing = major;
}

qreal SMPTEIntervalDisplay::calcStepInterval(qreal windowSize,
													  qreal viewSize,
													  qreal minStepSize) const
{
	qreal ival;
	if (INTERVALS_FRAME.contains(mFramerate))
	{
		if (calcInterval(windowSize, viewSize, minStepSize, INTERVALS_FRAME[mFramerate], ival))
			return ival;
	}

	if (calcInterval(windowSize, viewSize, minStepSize, INTERVALS_SMPTE, ival))
		return ival;

	return -1;
}

const QString SMPTEIntervalDisplay::timeToString(qreal interval, qreal time) const
{
	int f = qFloor(fmod(time, 1.0) * mFramerate);
	int s = qFloor(time);
	int m = qFloor(s / 60.0);
	int h = qFloor(m / 60.0);
	m = m % 60;
	s = s % 60;

	return QString("%1:%2:%3.%4").arg(QString::asprintf("%02d", h),
									  QString::asprintf("%02d", m),
									  QString::asprintf("%02d", s),
									  QString::asprintf("%02d", f));
}

SMPTEIntervalDisplay::SMPTEIntervalDisplay(int framerate)
{
	setFramerate(framerate);
}

void SMPTEIntervalDisplay::setFramerate(int framerate)
{
	mFramerate = framerate;
}

qreal GeneralTimeIntervalDisplay::calcStepInterval(qreal windowSize,
															qreal viewSize,
															qreal minStepSize) const
{
	qreal ival;
	if (calcInterval(windowSize, viewSize, minStepSize, INTERVALS_TIME, ival))
		return ival;

	return -1;
}

const QString GeneralTimeIntervalDisplay::timeToString(qreal interval, qreal time) const
{
	if (time == 0)
		return "0";

	QMapIterator<qreal, QString> iter(TIME_SUFFIXES);
	iter.toBack();
	while (iter.hasPrevious())
	{
		iter.previous();
		if (fmod(time, iter.key()) == 0)
			return QString::number(time / iter.key()) + iter.value();

		if (interval >= iter.key())
			return QString("%1%2").arg(QString::number(time / (qreal) iter.key()), iter.value());
	}
	return QString::number(time);
}


qreal FloatIntervalDisplay::calcStepInterval(qreal windowSize, qreal viewWidth,
													  qreal minStepSize) const
{
	const qreal ln10 = log(10);

	// calculate an initial guess at step size
	qreal targetSteps = viewWidth / minStepSize;
	qreal tempStep = windowSize / targetSteps;

	// get the magnitude of the step size
	auto mag = qFloor(log(tempStep) / ln10);
	auto magPow = qPow(10, mag);

	// calculate most significant digit of the new step size
	qreal magMsd = qRound(tempStep / magPow + 0.5);

	// promote the MSD to either 1, 2, or 5
	if (magMsd > 5.0)
		magMsd = 10.0;
	else if (magMsd > 2.0)
		magMsd = 5.0;
	else if (magMsd > 1.0)
		magMsd = 2.0;

	return magMsd * magPow;
}

const QString FloatIntervalDisplay::timeToString(qreal interval, qreal time) const
{
	return QString::number(time);
}

qreal AnimationIntervalDisplay::calcStepInterval(qreal windowSize, qreal viewWidth, qreal minStepSize) const
{
	qreal ival;
	if (INTERVALS_FRAME.contains(mFramerate))
	{
		if (calcInterval(windowSize, viewWidth, minStepSize, INTERVALS_FRAME[mFramerate], ival))
			return ival;
	}

	if (calcInterval(windowSize, viewWidth, minStepSize, INTERVALS_TIME, ival))
		return ival;

	return -1;
}

const QString AnimationIntervalDisplay::timeToString(qreal interval, qreal time) const
{
	if (time == 0)
		return "0";

	QMapIterator<qreal, QString> iter(TIME_SUFFIXES);
	iter.toBack();
	while (iter.hasPrevious())
	{
		iter.previous();
		if (fmod(time, iter.key()) == 0)
			return QString::number(time / iter.key()) + iter.value();

		if (interval >= iter.key())
			return QString("%1%2").arg(QString::number(time / (qreal) iter.key()), iter.value());
	}
	return QString::number(time);
}

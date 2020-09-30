/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <QString>
#include <QList>

namespace nap
{
	namespace qt
	{
		class IntervalDisplay
		{
		public:
			virtual QString name() const = 0;

			virtual qreal calcStepInterval(qreal windowSize, qreal viewWidth, qreal minStepSize) const = 0;

			virtual const QString timeToString(qreal interval, qreal time) const = 0;

			void setHatchSpacing(qreal minor, qreal major);

			qreal minorHatchSpacing() const { return mMinorHatchSpacing; }
			qreal majorHatchSpacing() const { return mMajorHatchSpacing; }

		private:
			qreal mMinorHatchSpacing = 10;
			qreal mMajorHatchSpacing = 100;
		};

		class SMPTEIntervalDisplay : public IntervalDisplay
		{
		public:
			explicit SMPTEIntervalDisplay(int framerate = 30);

			QString name() const override { return "SMPTE"; };

			void setFramerate(int fps);

			qreal calcStepInterval(qreal windowSize, qreal viewSize, qreal minStepSize) const override;

			const QString timeToString(qreal interval, qreal time) const override;

		private:
			int mFramerate;
		};

		class GeneralTimeIntervalDisplay : public IntervalDisplay
		{
		public:
			QString name() const override { return "General"; };

			qreal calcStepInterval(qreal windowSize, qreal viewSize, qreal minStepSize) const override;

			const QString timeToString(qreal interval, qreal time) const override;
		};

		class FloatIntervalDisplay : public IntervalDisplay
		{
		public:
			QString name() const override { return "Float"; };

			qreal calcStepInterval(qreal windowSize, qreal viewWidth, qreal minStepSize) const override;

			const QString timeToString(qreal interval, qreal time) const override;
		};

		class AnimationIntervalDisplay : public IntervalDisplay
		{
		public:
			explicit AnimationIntervalDisplay(int framerate = 30) : mFramerate(framerate), IntervalDisplay() {}
			QString name() const override { return "Animation"; };
			qreal calcStepInterval(qreal windowSize, qreal viewWidth, qreal minStepSize) const override;
			const QString timeToString(qreal interval, qreal time) const override;
			const int framerate() const { return mFramerate; }
		private:
			int mFramerate;
		};

	} // namespace qt
	
} // namespace nap
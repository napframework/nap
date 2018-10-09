#pragma once

#include <QString>
#include <QList>

namespace napkin
{
	class TimeDisplay
	{
	public:
		virtual QString name() const = 0;

		virtual qreal calcStepInterval(qreal windowSize, qreal viewWidth, qreal minStepSize) const = 0;

		virtual const QString timeToString(qreal interval, qreal time) const = 0;

		void setHatchSpacing(qreal minor, qreal major);

		qreal minorHatchSpacing() const
		{ return mMinorHatchSpacing; }

		qreal majorHatchSpacing() const
		{ return mMajorHatchSpacing; }

	private:
		qreal mMinorHatchSpacing = 10;
		qreal mMajorHatchSpacing = 100;
	};

	class SMPTETimeDisplay : public TimeDisplay
	{
	public:
		SMPTETimeDisplay(int framerate = 30);

		QString name() const override { return "SMPTE"; };

		void setFramerate(int fps);

		qreal calcStepInterval(qreal windowSize, qreal viewSize, qreal minStepSize) const override;

		const QString timeToString(qreal interval, qreal time) const override;

	private:
		int mFramerate;
	};

	class GeneralTimeDisplay : public TimeDisplay
	{
	public:
		QString name() const override { return "General"; };

		qreal calcStepInterval(qreal windowSize, qreal viewSize, qreal minStepSize) const override;

		const QString timeToString(qreal interval, qreal time) const override;
	};

	class FloatTimeDisplay : public TimeDisplay
	{
	public:
		QString name() const override { return "Float"; };

		qreal calcStepInterval(qreal windowSize, qreal viewWidth, qreal minStepSize) const override;

		const QString timeToString(qreal interval, qreal time) const override;
	};

	class AnimationTimeDisplay : public TimeDisplay
	{
	public:
		AnimationTimeDisplay(int framerate=30) : mFramerate(framerate), TimeDisplay() {}
		QString name() const override { return "Animation"; };
		qreal calcStepInterval(qreal windowSize, qreal viewWidth, qreal minStepSize) const override;
		const QString timeToString(qreal interval, qreal time) const override;
		const int framerate() const { return mFramerate; }
	private:
		int mFramerate;
	};

} // namespace napkin
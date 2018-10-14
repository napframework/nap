#include "ruler.h"

#include <QStyleOptionButton>
#include <QPainter>
#include <QStylePainter>
#include <QtGui>

napqt::Ruler::Ruler(QWidget* parent) : QWidget(parent)
{
//	mTimeConfig = new SMPTEIntervalDisplay();
//	mTimeConfig = new FloatIntervalDisplay();
	mTimeConfig = new GeneralTimeIntervalDisplay();

	mFont.setFamily("Monospace");
	mFont.setStyleHint(QFont::TypeWriter);
	setHatchFont(mFont);

}


void napqt::Ruler::setRange(const napqt::Range& range)
{
	if (mRange == range)
		return;

	mRange.set(range);
	update();
}

void napqt::Ruler::paintEvent(QPaintEvent* event)
{
	QStylePainter painter;
	painter.begin(this);

	painter.setBrush(palette().background());
	painter.setPen(Qt::NoPen);
	painter.setFont(mFont);
	painter.drawRect(rect());
	{
		painter.setPen(QPen(palette().foreground().color()));

		drawHatches(painter, mMinorHatchLength, mTimeConfig->minorHatchSpacing(), false);
		drawHatches(painter, mMajorHatchLength, mTimeConfig->majorHatchSpacing(), true);
	}
	painter.end();
}


void napqt::Ruler::drawHatches(QPainter& painter, int hatchLength, qreal minStepSize, bool drawLabels) const
{
	qreal start = mRange.start();
	qreal end = mRange.end();
	int textY = mTextHeight;
	if (start > end)
	{
		start = mRange.end();
		end = mRange.start();
	}

	qreal windowSize = end - start; // How much time we're seeing in the view

	qreal viewScale = width() / windowSize;

	// Step distance in time-space
	qreal stepInterval = mTimeConfig->calcStepInterval(windowSize, width(), minStepSize);

	// Step distance in view-space (pixels)
	qreal stepSize = viewScale * stepInterval;
	// Start offset in view-space (pixels)
	qreal startOffset = -start * viewScale;
	// How much to offset (sub-step) to match scroll
	qreal localOffset = fmod(startOffset, stepSize);

	// Offset in time-space
	int timeOffset = qFloor(start / stepInterval);
	if (timeOffset < 0) // Correct for negative values
		timeOffset++;

	int y = height() - hatchLength;

	int stepCount = qCeil(windowSize / stepInterval) + 1;
	for (int i = 0; i < stepCount; i++)
	{
		// floor instead of round, matches QGraphicsView aliasing
		int x = qFloor(localOffset + (qreal) i * stepSize);

		painter.drawLine(x, y, x, height());

		if (drawLabels)
		{
			qreal time = (timeOffset + i) * stepInterval;

			const QString timestr = mTimeConfig->timeToString(stepInterval, time);
			painter.drawText(x + mLabelOffset.x(),
							 textY + mLabelOffset.y(),
							 timestr);
		}
	}
}



void napqt::Ruler::setHeight(int height)
{
	setMinimumHeight(height);
	setMaximumHeight(height);
}

void napqt::Ruler::setHatchFont(const QFont& font)
{
	mFont = font;

	QFontMetrics metrics(mFont);

	mTextHeight = metrics.height();
}

void napqt::Ruler::resizeEvent(QResizeEvent* event)
{
	update();
}
void napqt::Ruler::setDisplayFormat(napqt::IntervalDisplay* fmt)
{
	mTimeConfig = fmt;
	update();
}





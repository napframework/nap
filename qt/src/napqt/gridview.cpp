/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "gridview.h"
#include "timedisplay.h"
#include <cassert>
#include <utility>
#include <QMouseEvent>
#include <QtGui>
#include <QtDebug>

#include <napqt/qtutils.h>

using namespace nap::qt;

QPointF mul(const QPointF& a, const QPointF& b)
{
	return QPointF(a.x() * b.x(), a.y() * b.y());
}

QString secondsToSMPTE(qreal seconds, int framerate)
{
	int f = qFloor(fmod(seconds, 1.0) * framerate);
	int s = qFloor(seconds);
	int m = qFloor(s / 60.0);
	int h = qFloor(m / 60.0);
	m = m % 60;
	s = s % 60;

	return QString("%1:%2:%3:%4").arg(QString::asprintf("%02d", h),
									  QString::asprintf("%02d", m),
									  QString::asprintf("%02d", s),
									  QString::asprintf("%02d", f));
}

GridView::GridView(QWidget* parent) : QGraphicsView(parent), mRubberBand(QRubberBand::Rectangle, this)
{
	mIvalDisplayHorizontal = std::make_shared<FloatIntervalDisplay>();
	mIvalDisplayVertical = std::make_shared<FloatIntervalDisplay>();
	setTransformationAnchor(QGraphicsView::NoAnchor);
	setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
	setMouseTracking(true);
	viewport()->setMouseTracking(true);
	mRulerFont.setFamily("monospace");
	mRulerFont.setPointSize(9);

	mPanBounds.setLeft(-std::numeric_limits<qreal>::max());
	mPanBounds.setTop(-std::numeric_limits<qreal>::max());
	mPanBounds.setRight(std::numeric_limits<qreal>::max());
	mPanBounds.setBottom(std::numeric_limits<qreal>::max());

	mRubberBand.hide();
}


void GridView::mousePressEvent(QMouseEvent* event)
{
	mMousePressPos = event->pos();
	mMouseLastPos = event->pos();

	bool lmb = event->buttons() == Qt::LeftButton;
	bool mmb = event->buttons() == Qt::MiddleButton;
	bool rmb = event->buttons() == Qt::RightButton;
	bool altKey = event->modifiers() == Qt::AltModifier;

	auto clickedItem = itemAt(event->pos());
	if (lmb && !clickedItem)
		startRubberBand(event->pos());
	else
		QGraphicsView::mousePressEvent(event);
}

void GridView::mouseMoveEvent(QMouseEvent* event)
{
	QGraphicsView::mouseMoveEvent(event);
	auto mousePos = event->pos();
	mMouseDelta = mousePos - mMouseLastPos;

	bool lmb = event->buttons() == Qt::LeftButton;
	bool mmb = event->buttons() == Qt::MiddleButton;
	bool rmb = event->buttons() == Qt::RightButton;
	bool altKey = event->modifiers() & Qt::AltModifier;
	bool shiftKey = event->modifiers() & Qt::ShiftModifier;

	if (altKey && (lmb || mmb))
	{
		pan(QPointF(mMouseDelta));
		event->accept();
	}
	else if ((altKey || shiftKey) && rmb)
	{
		QPointF delta(1, 1);
		if (altKey)
			delta += QPointF(mMouseDelta.x(), 0) * 0.01;

		if (shiftKey)
			delta += QPointF(0, -mMouseDelta.y()) * 0.01;

		zoom(delta, mapToScene(mMousePressPos));
		event->accept();
	}
	else
	{
		QGraphicsView::mouseMoveEvent(event);
	}

	updateRubberBand(mousePos);

	mMouseLastPos = mousePos;
	event->ignore();
}

void GridView::mouseReleaseEvent(QMouseEvent* event)
{
	QGraphicsView::mouseReleaseEvent(event);
	hideRubberBand();
}

void GridView::keyPressEvent(QKeyEvent* event)
{
	QGraphicsView::keyPressEvent(event);
	switch (event->key())
	{
		case Qt::Key_Home:
			centerView();
			break;
		case Qt::Key_A:
			frameAll();
			break;
		case Qt::Key_F:
			frameSelected();
			break;
		default:
			break;
	}
}

void GridView::keyReleaseEvent(QKeyEvent* event)
{
	QGraphicsView::keyReleaseEvent(event);
}

void GridView::zoom(const QPointF& delta, const QPointF& pivot)
{
	auto xf = transform();

	// Translate to zoom around pivot
	xf.translate(pivot.x(), pivot.y());

	if (mZoomMode == ZoomMode::IgnoreAspectRatio)
	{
		xf.scale(delta.x(), delta.y());
	} else
	{
		qreal avg = (delta.x() + delta.y()) / 2;
		if (mZoomMode == ZoomMode::KeepAspectRatio)
		{
			xf.scale(avg, avg);
		} else if (mZoomMode == ZoomMode::Horizontal)
		{
			xf.scale(avg, 1);
		} else if (mZoomMode == ZoomMode::Vertical)
		{
			xf.scale(1, avg);
		}
	}

	// Restore after pivot zoom
	xf.translate(-pivot.x(), -pivot.y());

	constrainTransform(xf);

	applyTransform(xf);
	viewTransformed();
}

void GridView::pan(const QPointF& delta)
{
	auto xf = transform();
	auto scale = viewScale();
	qreal dx = (mPanMode == PanMode::Vertical) ? 0 : delta.x() / scale.x();
	qreal dy = (mPanMode == PanMode::Horizontal) ? 0 : delta.y() / scale.y();
	xf.translate(dx, dy);

	constrainTransform(xf);

	applyTransform(xf);
	viewTransformed();
}

void GridView::constrainTransform(QTransform& xf)
{
	auto p = getTranslation(xf);
	p.setX(qBound(-mPanBounds.right(), p.x(), -mPanBounds.left()));
	p.setY(qBound(-mPanBounds.bottom(), p.y(), -mPanBounds.top()));
	setTranslation(xf, p);
}

void GridView::constrainView()
{
	return;
	auto xf = transform();
//	constrainTransform(xf);
	applyTransform(xf);
}

void GridView::setVerticalFlipped(bool flipped)
{
	if (mVerticalFlipped != flipped)
		scale(1, -1);
	mVerticalFlipped = flipped;

}


void GridView::drawBackground(QPainter* painter, const QRectF& rect)
{

	auto viewRect = mapToScene(viewport()->rect()).boundingRect();
	QColor bgCol = palette().background().color();
	QColor holdoutCol = mDrawHoldout ? bgCol.darker(110) : bgCol;
	painter->fillRect(rect, holdoutCol);
	if (mDrawHoldout)
		painter->fillRect(mHoldoutRect, bgCol);


	QColor gridMinorCol = holdoutCol.darker(110);
	QColor gridMajorCol = holdoutCol.darker(150);

	if (mGridEnabled)
	{
		if (mDrawHLines)
			drawHatchesHorizontal(painter, viewRect, mGridMinStepSizeHMinor, gridMinorCol, false);
		if (mDrawVLines)
			drawHatchesVertical(painter, viewRect, mGridMinStepSizeVMinor, gridMinorCol, false);
		if (mDrawHLines)
			drawHatchesHorizontal(painter, viewRect, mGridMinStepSizeHMajor, gridMajorCol, mDrawLabelsH);
		if (mDrawVLines)
			drawHatchesVertical(painter, viewRect, mGridMinStepSizeVMajor, gridMajorCol, mDrawLabelsV);
	}

}

void GridView::drawHatchesHorizontal(QPainter* painter, const QRectF& rect, qreal minStepSize, const QColor& color,
									 bool labels)
{
	qreal start = rect.left();
	qreal end = rect.right();
	qreal windowSize = end - start; // How much time we're seeing in the view
	int viewWidth = viewport()->width();
	int viewHeight = viewport()->height();
	int labelOffsetX = 4;
	int labelOffsetY = 20;

	auto viewScale = getScale(transform()).width();

	qreal stepInterval = mIvalDisplayHorizontal->calcStepInterval(windowSize, viewWidth, minStepSize);

	qreal stepSize = viewScale * stepInterval;
	qreal startOffset = -start * viewScale;
	qreal localOffset = fmod(startOffset, stepSize);

	// Offset in time-space
	int timeOffset = qFloor(start / stepInterval);
	if (timeOffset < 0) // Correct for negative values
		timeOffset++;

	// Start drawing in screenspace
	painter->save();
	painter->resetTransform();

	painter->setPen(color);

	int stepCount = qCeil(windowSize / stepInterval) + 1;
	for (int i = 0; i < stepCount; i++)
	{
		// floor instead of round, matches QGraphicsView aliasing
		int x = qFloor(localOffset + (qreal) i * stepSize);

		painter->drawLine(x, 0, x, viewHeight);

		if (labels)
		{
			qreal time = (timeOffset + i) * stepInterval;

			const QString timestr = mIvalDisplayHorizontal->timeToString(stepInterval, time);
			painter->drawText(x + labelOffsetX, labelOffsetY, timestr);
		}
	}

	painter->restore();
}
void GridView::drawHatchesVertical(QPainter* painter, const QRectF& rect, qreal minStepSize, const QColor& color,
								   bool labels)
{
//	qreal minStepSize = gridMinStepSizeV;
	qreal start = rect.top();
	qreal end = rect.bottom();
	qreal windowSize = end - start; // How much time we're seeing in the view
	int viewWidth = viewport()->width();
	int viewHeight = viewport()->height();
	int labelOffsetX = 4;
	int labelOffsetY = 20;

	auto viewScale = getScale(transform()).height();

	// Step distance in time-space
	qreal stepInterval = mIvalDisplayVertical->calcStepInterval(windowSize, viewHeight, minStepSize);

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

	// Start drawing in screenspace
	painter->save();
	painter->resetTransform();

	painter->setPen(color);

	int stepCount = qCeil(windowSize / stepInterval) + 1;
	for (int i = 0; i < stepCount; i++)
	{
		// floor instead of round, matches QGraphicsView aliasing
		int y = qFloor(localOffset + (qreal) i * stepSize);
		if (mVerticalFlipped)
			y += viewHeight;

		painter->drawLine(0, y, viewWidth, y);

		if (labels)
		{
			qreal time = (timeOffset + i) * stepInterval;

			const QString timestr = mIvalDisplayHorizontal->timeToString(stepInterval, time);
			painter->drawText(labelOffsetX, y + labelOffsetY, timestr);
		}
	}

	painter->restore();
}

void GridView::wheelEvent(QWheelEvent* event)
{
	qreal delta = 1 + event->delta() * 0.001;
	zoom(QPointF(delta, delta), mapToScene(event->pos()));
}

void GridView::centerView()
{
	auto xf = transform();
	xf.reset();
	xf.translate(rect().width() / 2, rect().height() / 2);
	applyTransform(xf);
	viewTransformed();
}


void GridView::frameAll()
{
	frameView(frameItemsBounds());
}


void GridView::frameSelected()
{
	frameView(frameItemsBoundsSelected());
}


void GridView::frameView(const QRectF& frameRect)
{
	const QMargins margins = mFrameMargins;

	QRectF rec = frameRect;
	if (mVerticalFlipped)
	{
		rec = { rec.left(), rec.bottom(), rec.width(), -rec.height() };
	}
	// The rectangle in view/pixel-space we want to fit the contents in
	auto focusRectView = viewport()->rect().adjusted(margins.left(), margins.top(), -margins.right(),
													 -margins.bottom());
	auto xf = transform();
	auto origScale = getScale(xf);
	qreal sx = origScale.width();
	qreal sy = origScale.height();
	qreal tx = 0;
	qreal ty = 0;

	if (mFrameZoomMode == ZoomMode::KeepAspectRatio)
		assert(false); // TODO: implement
	if (mFrameZoomMode == ZoomMode::IgnoreAspectRatio || mFrameZoomMode == ZoomMode::Horizontal)
		sx = focusRectView.width() / rec.width();
	if (mFrameZoomMode == ZoomMode::IgnoreAspectRatio || mFrameZoomMode == ZoomMode::Vertical)
		sy = focusRectView.height() / rec.height();

	if (mFramePanMode == PanMode::Parallax || mFramePanMode == PanMode::Horizontal)
		tx = -rec.left() + (margins.left() / sx);
	if (mFramePanMode == PanMode::Parallax || mFramePanMode == PanMode::Vertical)
		ty = -rec.top() + (margins.top() / sy);

	xf.reset();
	setScale(xf, sx, sy);
	// center
	xf.translate(tx, ty);
	applyTransform(xf);

//	// Use
//	centerOn(tx, ty);
	constrainView();
	viewTransformed();
}


const QPointF GridView::viewScale() const
{
	auto xf = transform();
	return QPointF(xf.m11(), xf.m22());
}

const QPointF GridView::viewPos() const
{
	auto xf = transform();
	return QPointF(xf.m31(), xf.m32());
}

void GridView::applyTransform(const QTransform& xf)
{
	auto trans = xf;
	setTransform(trans);
}

const QRectF GridView::frameItemsBoundsSelected() const
{
	auto selection = scene()->selectedItems();
	if (selection.isEmpty())
		return frameItemsBounds();

	auto rect = selection[0]->sceneBoundingRect();

	if (selection.size() == 1)
		return rect;

	for (int i = 1, len = selection.size(); i < len; i++)
		rect = rect.united(selection[i]->sceneBoundingRect());

	return rect;
}

const QRectF GridView::frameItemsBounds() const
{
	return scene()->itemsBoundingRect();
}

void GridView::setVerticalScroll(int value)
{
	auto mViewTransform = transform();
	auto scroll = getTranslation(mViewTransform);
	scroll.setY(-value);
	setTranslation(mViewTransform, scroll);
	applyTransform(mViewTransform);
}

void GridView::setPanBounds(const QRectF& rec)
{
	mPanBounds = rec;
}

void GridView::setPanBounds(qreal left, qreal top, qreal right, qreal bottom)
{
	mPanBounds = QRectF(left, top, right, bottom);
	constrainView();
}

void GridView::startRubberBand(const QPoint& pos)
{
	mRubberBand.setGeometry(QRect(pos, QSize()));
	mRubberBand.show();
}

void GridView::updateRubberBand(const QPoint& pos)
{
	mRubberBand.setGeometry(QRect(mMousePressPos, pos).normalized());
}

void GridView::hideRubberBand()
{
	mRubberBand.hide();
}

const QRect GridView::rubberBandGeo() const
{
	return mRubberBand.geometry();
}

void GridView::setGridIntervalDisplay(std::shared_ptr<IntervalDisplay> horiz, std::shared_ptr<IntervalDisplay> vert)
{
	mIvalDisplayHorizontal = std::move(horiz);
	mIvalDisplayVertical = std::move(vert);
}

void GridView::setSelection(const QList<QGraphicsItem*>& items)
{
	clearSelection();
	addSelection(items);
}

void GridView::addSelection(const QList<QGraphicsItem*>& items)
{
	for (auto m : items)
	{
		if (!m->isSelected())
			m->setSelected(true);
	}
}

void GridView::removeSelection(const QList<QGraphicsItem*>& items)
{
	for (auto m : items)
		if (m->isSelected())
			m->setSelected(false);
}

void GridView::toggleSelection(const QList<QGraphicsItem*>& items)
{
	for (auto m : items)
		m->setSelected(!m->isSelected());
}

void GridView::clearSelection()
{
	removeSelection(scene()->selectedItems());
}

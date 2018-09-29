#include "gridview.h"
#include <QMouseEvent>
#include <QtGui/QtGui>
#include <nap/logger.h>
#include <QtDebug>
#include <generic/qtutils.h>

using namespace napkin;

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

GridView::GridView() : QGraphicsView()
{
	setTransformationAnchor(QGraphicsView::NoAnchor);
	setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
	setMouseTracking(true);
	viewport()->setMouseTracking(true);
	mRulerFont.setFamily("monospace");
	mRulerFont.setPointSize(9);

	mPanBounds.setLeft(std::numeric_limits<qreal>::min());
	mPanBounds.setTop(std::numeric_limits<qreal>::min());
	mPanBounds.setRight(std::numeric_limits<qreal>::max());
	mPanBounds.setBottom(std::numeric_limits<qreal>::max());
}


void GridView::mousePressEvent(QMouseEvent* event)
{
	mMousePressPos = event->pos();
	mMouseLastPos = event->pos();
}

void GridView::mouseMoveEvent(QMouseEvent* event)
{
	QGraphicsView::mouseMoveEvent(event);
	auto mousePos = event->pos();
	mMouseDelta = mousePos - mMouseLastPos;

	bool lmb = event->buttons() == Qt::LeftButton;
	bool mmb = event->buttons() == Qt::MiddleButton;
	bool rmb = event->buttons() == Qt::RightButton;
	bool altKey = event->modifiers() == Qt::AltModifier;

	if (altKey && (lmb || mmb))
	{
		pan(QPointF(mMouseDelta));
		event->accept();
	} else if (altKey && rmb)
	{
		zoom(QPointF(1, 1) + QPointF(mMouseDelta) * 0.01, mapToScene(mMousePressPos));
		event->accept();
	}

	mMouseLastPos = mousePos;
	event->ignore();
}

void GridView::mouseReleaseEvent(QMouseEvent* event)
{
	QGraphicsView::mouseReleaseEvent(event);
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
			frameAll(true, false);
			break;
		case Qt::Key_F:
			frameSelected(true, false);
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

	setTransform(xf);
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

	setTransform(xf);
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
	auto xf = transform();
	constrainTransform(xf);
	setTransform(xf);
}

void GridView::drawBackground(QPainter* painter, const QRectF& rect)
{
	painter->fillRect(rect, palette().background().color());
	if (!mGridEnabled)
		return;

	auto viewRect = viewport()->rect();
	auto sceneRect = mapToScene(viewRect).boundingRect();
	qreal desiredSpacing = 300;


	QString suffix;

	// Draw minor lines
	{
		qreal stepSizeX = calcGridStepTime(desiredSpacing, viewRect.width(), sceneRect.width(), 10);
		qreal stepSizeY = calcGridStep(desiredSpacing, viewRect.height(), sceneRect.height());

		int xmin = qFloor(sceneRect.left() / stepSizeX);
		int xmax = qCeil(sceneRect.right() / stepSizeX);
		int ymin = qFloor(sceneRect.top() / stepSizeY);
		int ymax = qCeil(sceneRect.bottom() / stepSizeY);


		painter->setPen(QPen(Qt::gray, 0, Qt::DotLine));
		for (int x = xmin; x < xmax; x++)
		{
			qreal tx = x * stepSizeX;
			QPointF p1(tx, sceneRect.top() + 22);
			QPointF p2(tx, sceneRect.bottom());
			painter->drawLine(p1, p2);
		}

	}

	// Draw lines
	{
		qreal stepSizeX = calcGridStepTime(desiredSpacing, viewRect.width(), sceneRect.width(), 150);
		qreal stepSizeY = calcGridStep(desiredSpacing, viewRect.height(), sceneRect.height());

		int xmin = qFloor(sceneRect.left() / stepSizeX);
		int xmax = qCeil(sceneRect.right() / stepSizeX);
		int ymin = qFloor(sceneRect.top() / stepSizeY);
		int ymax = qCeil(sceneRect.bottom() / stepSizeY);

		painter->setPen(QPen(Qt::gray, 0));
		if (stepSizeX > 0)
		{
			for (int x = xmin; x < xmax; x++)
			{
				qreal tx = x * stepSizeX;
				QPointF p1(tx, sceneRect.top());
				QPointF p2(tx, sceneRect.bottom());
				painter->drawLine(p1, p2);
			}
		}
		if (stepSizeY > 0)
		{
			for (int y = ymin; y < ymax; y++)
			{
				qreal ty = y * stepSizeY;
				QPointF p1(sceneRect.left(), ty);
				QPointF p2(sceneRect.right(), ty);
				painter->drawLine(p1, p2);
			}
		}

		// Draw Numbers
		painter->setFont(mRulerFont);

		painter->save();
		painter->resetMatrix();
		if (stepSizeX > 0)
		{
			for (int x = xmin; x < xmax; x++)
			{
				qreal tx = x * stepSizeX;
				auto pt = mapFromScene(tx, sceneRect.top()) + QPointF(5, 15);
				if (pt.x() > 20)
				{
					if (mRulerFormat == RulerFormat::Float)
						painter->drawText(pt, QString::number(tx) + suffix);
					else if (mRulerFormat == RulerFormat::SMPTE)
						painter->drawText(pt, secondsToSMPTE(tx, mFramerate));
				}
			}
		}
		if (stepSizeY > 0)
		{
			for (int y = ymin; y < ymax; y++)
			{
				qreal ty = y * stepSizeY + 5;
				auto pt = mapFromScene(sceneRect.left(), ty) + QPointF(5, 15);
				if (pt.y() > 20)
				{
					painter->drawText(pt, QString::number(ty) + suffix);
				}
			}
		}

		painter->restore();
	}


	const int framerate = 30;
	const qreal frame = 1.0 / framerate;
	const qreal second = 1;
	const qreal minute = 60;
	const qreal hour = minute * 60;
	const qreal day = hour * 24;


}

qreal GridView::calcGridStep(qreal desiredSpacing, qreal viewWidth, qreal sceneRectWidth) const
{
	qreal targetSteps = viewWidth / desiredSpacing;
	qreal estStep = sceneRectWidth / targetSteps;
	qreal magPow = qPow(10, qFloor(log(estStep) / log(10)));
	qreal magMsd = qRound(estStep / magPow * 0.5);

	if (magMsd > 5)
		magMsd = 10;
	else if (magMsd > 2)
		magMsd = 5;
	else if (magMsd > 1)
		magMsd = 2;


	return magMsd * magPow;
}

qreal GridView::calcGridStepTime(qreal desiredSpacing, qreal viewWidth, qreal sceneRectWidth,
								 qreal minStepSize) const
{
	const qreal intervals[] = {
			1.0 / mFramerate, // frame
			1.0, // second
			2.0,
			5.0,
			10.0, // 10 seconds
			30.0, // 30 seconds
			60.0, // minute
			60.0 * 60.0, // hour
			60.0 * 60.0 * 2,
			60.0 * 60.0 * 5,
			60.0 * 60.0 * 10,
			60.0 * 60.0 * 20,
			60.0 * 60.0 * 50,
			60.0 * 60.0 * 100, // hundred hours
			60.0 * 60.0 * 200,
			60.0 * 60.0 * 500,
			60.0 * 60.0 * 1000,
	};
	int len = sizeof(intervals) / sizeof(intervals[0]);

	for (int i = 0; i < len; i++)
	{
		qreal ival = intervals[i];
		qreal steps = sceneRectWidth / ival;
		qreal stepSize = viewWidth / steps;
		if (stepSize > minStepSize)
			return ival;
	}
	return -1;
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
	setTransform(xf);
	viewTransformed();
}


void GridView::frameAll(bool horizontal, bool vertical, QMargins margins)
{
	frameView(this->scene()->itemsBoundingRect(), horizontal, vertical, margins);
}

void GridView::frameSelected(bool horizontal, bool vertical, QMargins margins)
{
	frameView(selectedItemsBoundingRect(), horizontal, vertical, margins);
}


void GridView::frameView(const QRectF& rec, bool horizontal, bool vertical, QMargins margins)
{
	auto focusRectView = viewport()->rect().adjusted(margins.left(), margins.top(), -margins.right(), -margins.bottom());
	auto xf = transform();
	auto origScale = napkin::getScale(xf);
	qreal sx = origScale.width();
	qreal sy = origScale.height();
	qreal tx = 0;
	qreal ty = 0;

	if (horizontal)
	{
		sx = focusRectView.width() / rec.width();
		tx = -rec.left() + (margins.left() / sx);
	}
	if (vertical)
	{
		sy = focusRectView.height() / rec.height();
		ty = -rec.top() + (margins.top() / sy);
	}

	xf.reset();
	setScale(xf, sx, sy);
	// center
	xf.translate(tx, ty);
	setTransform(xf);

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

QRectF GridView::selectedItemsBoundingRect() const
{
	auto selection = scene()->selectedItems();
	if (selection.isEmpty())
		return scene()->itemsBoundingRect();

	auto rect = selection[0]->sceneBoundingRect();

	if (selection.size() == 1)
		return rect;

	for (int i = 1, len = selection.size(); i < len; i++)
		rect = rect.united(selection[i]->sceneBoundingRect());

	return rect;
}

void GridView::setVerticalScroll(int value)
{
	auto mViewTransform = transform();
	auto scroll = getTranslation(mViewTransform);
	scroll.setY(-value - scene()->sceneRect().top());
	setTranslation(mViewTransform, scroll);
//	viewTransformed();
}

void GridView::fitInView(const QRectF& rect, const QMargins& margins,
						 bool horizontal, bool vertical)
{
	if (!scene() or rect.isNull())
		return;

//	auto last_scene_roi = rect;
	auto flags = Qt::KeepAspectRatio;

	auto unity = transform().mapRect(QRectF(0, 0, 1, 1));

	scale(1.0 / unity.width(), 1.0 / unity.height());
	auto viewRect = viewport()->rect();
	auto sceneRect = transform().mapRect(rect);
	auto xratio = viewRect.width() / (qreal) sceneRect.width();
	auto yratio = viewRect.height() / (qreal) sceneRect.height();
	if (flags == Qt::KeepAspectRatio)
	{
		xratio = yratio = qMin(xratio, yratio);
	} else if (flags == Qt::KeepAspectRatioByExpanding)
	{
		xratio = yratio = qMax(xratio, yratio);
	}
	if (!horizontal)
		xratio = 1;
	if (!vertical)
		yratio = 1;

	scale(xratio, yratio);
	centerOn(rect.center());
}

void GridView::setGridEnabled(bool enabled)
{
	mGridEnabled = enabled;
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


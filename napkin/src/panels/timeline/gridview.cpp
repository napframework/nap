#include "gridview.h"
#include <QMouseEvent>
#include <QtGui/QtGui>
#include <iostream>

using namespace napkin;

GridView::GridView() : QGraphicsView() {
	setTransformationAnchor(QGraphicsView::NoAnchor);
}


void GridView::mousePressEvent(QMouseEvent* event) {
	QWidget::mousePressEvent(event);
	mMousePressPos = event->pos();
	mMouseLastPos = event->pos();
}

void GridView::mouseMoveEvent(QMouseEvent* event) {
	QWidget::mouseMoveEvent(event);
	auto mousePos = event->pos();
	auto delta = mousePos - mMouseLastPos;

	bool lmb = event->buttons() == Qt::LeftButton;
	bool rmb = event->buttons() == Qt::RightButton;
	bool altKey = event->modifiers() == Qt::AltModifier;

	if (lmb && altKey) {
		pan(QPointF(delta));
	} else if (altKey && rmb) {
		zoom(QPointF(1, 1) + QPointF(delta) * 0.01, mapToScene(mMousePressPos.toPoint()));
	}

	mMouseLastPos = mousePos;
}

void GridView::mouseReleaseEvent(QMouseEvent* event) {
	QWidget::mouseReleaseEvent(event);
}

void GridView::keyPressEvent(QKeyEvent* event) {
	QWidget::keyPressEvent(event);
	if (event->key() == Qt::Key_F) {
		centerView();
	}
}

void GridView::keyReleaseEvent(QKeyEvent* event) {
	QWidget::keyReleaseEvent(event);
}

void GridView::zoom(const QPointF& delta, const QPointF& pivot) {
	mViewTransform.translate(pivot.x(), pivot.y());
	mViewTransform.scale(delta.x(), delta.y());
	mViewTransform.translate(-pivot.x(), -pivot.y());
	applyViewTransform();
}

void GridView::pan(const QPointF& delta) {
	auto scale = viewScale();
	qreal dx = delta.x() / scale.x();
	qreal dy = delta.y() / scale.y();
	mViewTransform.translate(dx, dy);
	applyViewTransform();
}

void GridView::drawBackground(QPainter* painter, const QRectF& rect) {
	painter->fillRect(rect, Qt::darkGray);

	auto viewRect = viewport()->rect();
	auto sceneRect = mapToScene(viewRect).boundingRect();
	qreal desiredSpacing = 300;


//	std::cout << stepSizeX << std::endl;

	qreal stepSizeX = calcGridStep(desiredSpacing, viewRect.width(), sceneRect.width());
	qreal stepSizeY = calcGridStep(desiredSpacing, viewRect.height(), sceneRect.height());

	int xmin = qFloor(sceneRect.left() / stepSizeX);
	int xmax = qCeil(sceneRect.right() / stepSizeX);
	int ymin = qFloor(sceneRect.top() / stepSizeY);
	int ymax = qCeil(sceneRect.bottom() / stepSizeY);

	painter->setPen(QPen(Qt::gray, 0));
	for (int x = xmin; x < xmax; x++) {
		qreal tx = x * stepSizeX;
		QPointF p1(tx, sceneRect.top());
		QPointF p2(tx, sceneRect.bottom());
		painter->drawLine(p1, p2);
	}

	for (int y = ymin; y < ymax; y++) {
		qreal ty = y * stepSizeY;
		QPointF p1(sceneRect.left(), ty);
		QPointF p2(sceneRect.right(), ty);
		painter->drawLine(p1, p2);
	}

	painter->setBrush(Qt::red);
	painter->drawRect(0, 0, 100, 100);

}

qreal GridView::calcGridStep(qreal desiredSpacing, qreal viewWidth, qreal sceneRectWidth) const {
	qreal targetSteps = viewWidth / desiredSpacing;
	qreal estStep = sceneRectWidth / targetSteps;
	qreal magPow = qPow(10, qFloor(log(estStep)/log(10)));
	qreal magMsd = qRound(estStep/magPow * 0.5);

	if (magMsd > 5)
		magMsd = 10;
	else if (magMsd > 2)
		magMsd = 5;
	else if (magMsd > 1)
		magMsd = 2;

	return magMsd * magPow;
}

void GridView::wheelEvent(QWheelEvent* event) {
	qreal delta = 1 + event->delta() * 0.001;
	zoom(QPointF(delta, delta), mapToScene(event->pos()));
}

void GridView::centerView() {
	mViewTransform.reset();
	applyViewTransform();
}

void GridView::applyViewTransform() {
	setTransform(mViewTransform);
}

const QPointF GridView::viewScale() const {
	return QPointF(mViewTransform.m11(), mViewTransform.m22());
}

const QPointF GridView::viewPos() const {
	return QPointF(mViewTransform.m31(), mViewTransform.m32());
}


#include "gridview.h"
#include <QMouseEvent>
#include <QtGui/QtGui>
#include <iostream>

using namespace napkin;

void myDrawText(QPainter *painter, QPointF p, QString text){

	QRawFont rawFont = QRawFont::fromFont(painter->font());
	QVector<quint32> indexes = rawFont.glyphIndexesForString(text);

	painter->save();
	for(unsigned int i=0; i<indexes.count(); i++){
		QPainterPath path = rawFont.pathForGlyph(indexes[i]);
		painter->translate(QPointF(p.x(), p.y() + i*0));
		painter->fillPath(path,painter->pen().brush());
	}
	painter->restore();
}

GridView::GridView() : QGraphicsView() {
	setTransformationAnchor(QGraphicsView::NoAnchor);

	mRulerFont.setFamily("monospace");
	mRulerFont.setPointSize(9);
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

	auto& xf = mViewTransform;

	// Translate to zoom around pivot
	xf.translate(pivot.x(), pivot.y());

	if (mZoomMode == IgnoreAspectRatio) {
		xf.scale(delta.x(), delta.y());
	} else {
		qreal avg = (delta.x() + delta.y()) / 2;
		if (mZoomMode == KeepAspectRatio) {
			xf.scale(avg, avg);
		} else if (mZoomMode == Horizontal) {
			xf.scale(avg, 1);
		} else if (mZoomMode == Vertical) {
			xf.scale(1, avg);
		}
	}

	// Restore after pivot zoom
	xf.translate(-pivot.x(), -pivot.y());

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
	auto vScale = viewScale();
	auto vPos = viewPos();
	auto sceneRect = mapToScene(viewRect).boundingRect();
	qreal desiredSpacing = 300;


//	std::cout << stepSizeX << std::endl;

	qreal stepSizeX = calcGridStep(desiredSpacing, viewRect.width(), sceneRect.width());
	qreal stepSizeY = calcGridStep(desiredSpacing, viewRect.height(), sceneRect.height());

	int xmin = qFloor(sceneRect.left() / stepSizeX);
	int xmax = qCeil(sceneRect.right() / stepSizeX);
	int ymin = qFloor(sceneRect.top() / stepSizeY);
	int ymax = qCeil(sceneRect.bottom() / stepSizeY);

	// Draw lines
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


	// Draw Numbers
	painter->setFont(mRulerFont);

	painter->save();
	painter->resetMatrix();
	for (int x = xmin; x < xmax; x++) {
		qreal tx = x * stepSizeX;
		auto pt = mapFromScene(tx, sceneRect.top()) + QPointF(5, 15);
		if (pt.x() > 20)
			painter->drawText(pt,QString::number(tx));
	}
	for (int y = ymin; y < ymax; y++) {
		qreal ty = y * stepSizeY + 5;
		auto pt = mapFromScene(sceneRect.left(), ty) + QPointF(5, 15);
		if (pt.y() > 20)
			painter->drawText(pt, QString::number(ty));
	}
	painter->restore();

	painter->setBrush(Qt::red);
	painter->drawRect(0, 0, 100, 100);
	painter->setBrush(Qt::green);
	painter->drawRect(0, 0, 1, 1);
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
	mViewTransform.translate(rect().x() / 2, rect().y() / 2);
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


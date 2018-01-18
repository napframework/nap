#include "eventitem.h"
#include <QPainter>
#include <QGraphicsScene>
#include <QtGui/QtGui>

napkin::EventItem::EventItem(QGraphicsItem* parent, napkin::Event& event)
		: mEvent(event), QObject(), QGraphicsRectItem(parent) {
	setFlag(QGraphicsItem::ItemIsSelectable, true);
	setFlag(QGraphicsItem::ItemIsMovable, true);
	mBrush = QBrush(QColor("#89A"));
	mBrushSelected = QBrush(QColor("#9AB"));

	mPenBorder = QPen(Qt::black, 1);
	mPenBorder.setCosmetic(true);
	mPenBorderSelected = QPen(Qt::white, 1);
	mPenBorderSelected.setCosmetic(true);

	onEventChanged(event);

	event.connect(&event, &Event::changed, this, &EventItem::onEventChanged);

}

void napkin::EventItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {

	painter->setPen(isSelected() ? mPenBorderSelected : mPenBorder);
	painter->setBrush(isSelected() ? mBrushSelected : mBrush);
	painter->drawRect(rect());

	QPoint textMargin(3, 5);

	painter->save();

	// Inverse scale to 'unscale' text
	auto mtx = painter->matrix();
	qreal sx = mtx.m11();
	qreal sy = mtx.m22();
	qreal px = mtx.dx();
	qreal py = mtx.dy();
	qreal pw = rect().width() * sx;
	qreal ph = rect().height() * sy;
	QRectF viewRect(px, py, pw, ph);

	painter->scale(sx, sy);

	painter->resetTransform();
	viewRect.moveTo(mtx.dx(), mtx.dy());

	auto text = painter->fontMetrics().elidedText(mEvent.name(), Qt::ElideRight, pw);


//	painter->drawText(textMargin.x(), rect().height() - textMargin.y(), text);
	painter->drawText(viewRect, text);

	painter->restore();
}

void napkin::EventItem::onEventChanged(Event& event) {
	setRect(0, 0, event.length(), event.track().height());
}


#include "eventitem.h"
#include <QBrush>
#include <QPainter>
#include <QGraphicsScene>
#include <QtGui/QtGui>

napkin::EventItem::EventItem(QGraphicsItem* parent, napkin::Event& event)
		: mEvent(event), QGraphicsRectItem(parent) {
	setFlag(QGraphicsItem::ItemIsSelectable, true);
	setBrush(QBrush(Qt::red));
	QPen pen(Qt::black, 1);
	pen.setCosmetic(true);
	setPen(pen);
	setRect(0, 0, event.length(), event.track().height());
}

void napkin::EventItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
	QGraphicsRectItem::paint(painter, option, widget);

	QPoint textMargin(3, 5);

	// Inverse scale to 'unscale' text
	auto mtx = painter->matrix();
	qreal sx = 1.0 / mtx.m11();
	qreal sy = 1.0 / mtx.m22();
	int width = qFloor(rect().width() / sx) - textMargin.x() * 2;
	painter->save();

	auto textRect = mapFromScene(rect()).boundingRect().adjusted(2, 2, -2, -2);

	painter->scale(sx, sy);

	auto text = painter->fontMetrics().elidedText(mEvent.name(), Qt::ElideRight, width);

	painter->drawText(textMargin.x(), rect().height() - textMargin.y(), text);

	painter->restore();
}

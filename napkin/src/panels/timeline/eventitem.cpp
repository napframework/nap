#include "eventitem.h"
#include <QBrush>
#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsTextItem>

napkin::EventItem::EventItem(QGraphicsItem* parent, napkin::Event& event) : mClip(event), QGraphicsRectItem(parent), mTextItem(this) {
	setBrush(QBrush(Qt::red));
	QPen pen(Qt::black, 1);
	pen.setCosmetic(true);
	setPen(pen);
	setRect(0, 0, event.length(), event.track().height());
	mTextItem.setPlainText(event.name());
	mTextItem.setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
}

void napkin::EventItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
	QGraphicsRectItem::paint(painter, option, widget);
}

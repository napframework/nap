#include "wireitem.h"

#include <QStyleOption>
#include <QPainter>
#include <QGraphicsScene>

#include "operatoritem.h"

#include "../napkin_utilities.h"

WireItem::WireItem(PinItem& srcPin, PinItem& dstPin)
        : mSrcPin(srcPin), mDstPin(dstPin), mWire(new Wire(
        (nap::OutputPlugBase&) srcPin.plugItem()->plug(),
        (nap::InputPlugBase&) dstPin.plugItem()->plug())), QGraphicsPathItem() {
    setFlag(ItemIsFocusable, true);
    setFlag(ItemIsSelectable, true);
}

void WireItem::updatePath()
{
	QPointF mSrcPos = mSrcPin.attachPos();
	QPointF mDstPos = mDstPin.attachPos();

	QPen pen;
	if (isSelected()) {
		pen.setColor(NetworkStyle::Col_NodeConnection);
		pen.setWidth(2);
	} else {
		pen.setColor(NetworkStyle::Col_NodeConnection);
		pen.setWidth(1);
	}

	pen.setStyle(Qt::SolidLine);
	pen.setColor(mSrcPin.color());

	QPainterPath p;
    calculateWirePath(mSrcPos, mDstPos, p);
	setPath(p);
	setPen(pen);
}

QRectF WireItem::boundingRect() const { return path().boundingRect(); }

void WireItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	Q_UNUSED(option)
	Q_UNUSED(widget)

	if (isVisible()) {
		updatePath();
		painter->setPen(pen());
		painter->drawPath(path());
	}
}

#include "wirepreviewitem.h"

#include <QStyleOption>
#include <QPainter>
#include <QGraphicsScene>

#include "operatoritem.h"
#include "../napkin_utilities.h"



void WirePreview::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	Q_UNUSED(option)
	Q_UNUSED(widget)

	if (isVisible()) {
		updatePath();
		painter->setPen(pen());
		painter->drawPath(path());
	}
}

QRectF WirePreview::boundingRect() const { return path().boundingRect(); }

void WirePreview::updatePath()
{
	if (mSrcPin) mSrcPos = mSrcPin->attachPos();
	if (mDstPin) mDstPos = mDstPin->attachPos();

	QPen pen;
	pen.setColor(NetworkStyle::Col_NodeConnection);
	pen.setWidth(1);
	pen.setStyle(Qt::DashLine);

	QPainterPath p;
    calculateWirePath(mSrcPos, mDstPos, p);
	setPath(p);
	setPen(pen);
}

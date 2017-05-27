#include "pintitem.h"
#include "operatoritem.h"

#include <QGraphicsScene>
#include <QPainter>

#include "../napkin_utilities.h"

PinItem::PinItem(PlugItem* plug) : QGraphicsItem(plug), mPlug(plug)
{
	Q_ASSERT(plug);
	mRect = QRectF(0, 0, NetworkStyle::PinSize, NetworkStyle::PinSize);

	//	setPath(shapeFromPlug(plug->plug()));
	//	setPen(QPen(Qt::darkGray));
	//	setBrush(dataTypeColor(plug->plug().getDataType()));
}

QPointF PinItem::attachPos()
{

	if (mPlug->isInput())
		return QPointF(scenePos().x() - mRect.left(),
					   scenePos().y() + mRect.height() / 2);
	return QPointF(scenePos().x() + mRect.right(),
				   scenePos().y() + mRect.height() / 2);
}


QPainterPath PinItem::shapeFromPlug(const nap::Plug& plug) const
{
	QPainterPath path;
	path.moveTo(0, 0);
	path.lineTo(mRect.width(), mRect.height() / 2.0);
	path.lineTo(0, mRect.height());
	path.lineTo(0, 0);
	return path;
}
void PinItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
					QWidget* widget)
{
	QStyleOptionGraphicsItem op = *option;
//    painter->fillRect(boundingRect(), op.palette.brush(QPalette::Mid));

    op.palette.setColor(QPalette::WindowText,
                        dataTypeColor(plugItem()->plug().getDataType()));
	QApplication::style()->drawPrimitive(QStyle::PE_IndicatorArrowRight, &op,
										 painter, widget);
}

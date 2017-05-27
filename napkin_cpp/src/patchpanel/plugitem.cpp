#include "plugitem.h"
#include <QGraphicsScene>

#include "../napkin_utilities.h"

PlugItem::PlugItem(OperatorItem* operatorItem, nap::Plug& plug)
	: QGraphicsItem(operatorItem), mOperator(operatorItem), mConnectedWire(NULL), mPlug(plug)
{
	mLabel = new QGraphicsTextItem(this);
	mPin = new PinItem(this);
	setParentItem(operatorItem);

	setName(QString::fromStdString(plug.getName()));
}

QRectF PlugItem::boundingRect() const { return childrenBoundingRect(); }

void PlugItem::setName(const QString name)
{
	mLabel->setPlainText(name);
	layout();
}
void PlugItem::layout()
{
	double textoffset = -6;

	if (isInput()) {
		mPin->setPos(0, 0);
		mLabel->setPos(mPin->boundingRect().width(), textoffset);
		//		mValue->setPos(-mValue->boundingRect().width(), 0);
	} else {
		mLabel->setPos(0, textoffset);
		mPin->setPos(mLabel->boundingRect().width(), 0);
	}

	mOperator->layout();
}

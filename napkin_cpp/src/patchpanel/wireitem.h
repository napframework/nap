#pragma once

#include <QGraphicsEffect>
#include <QGraphicsItem>

#include "../wire.h"
#include "../itemtypes.h"
#include "pintitem.h"

class WireItem : public QGraphicsPathItem
{
public:
	enum { Type = UserType + WireItemType };
	int type() const { return Type; }
public:
    WireItem(PinItem& srcPin, PinItem& dstPin);

    Wire& wire() const { return *mWire; }

	PinItem& srcPin() const { return mSrcPin; }
	PinItem& dstPin() const { return mDstPin; }

	void updatePath();
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
	QRectF boundingRect() const;

protected:
    PinItem& mSrcPin;
    PinItem& mDstPin;
	Wire* mWire;
};



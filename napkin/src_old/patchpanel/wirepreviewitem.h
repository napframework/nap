#pragma once

#include <QColor>
#include <QGraphicsEffect>
#include <QGraphicsItem>

#include "plugitem.h"
#include "../itemtypes.h"


class WirePreview : public QGraphicsPathItem
{
public:
	enum { Type = UserType + WirePreviewItemType };
	int type() const { return Type; }
public:
	WirePreview(QGraphicsItem* parent) : QGraphicsPathItem(parent), mSrcPin(NULL), mDstPin(NULL)
	{
		setAcceptTouchEvents(false);
		setAcceptHoverEvents(false);
	}

	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
	QRectF boundingRect() const;

	void setSrcPos(const QPointF& pos) { mSrcPos = pos; }
	void setDstPos(const QPointF& pos) { mDstPos = pos; }

	void setSrcPin(PinItem* pin) { mSrcPin = pin; }
	void setDstPin(PinItem* pin) { mDstPin = pin; }

	PinItem* srcPin() const { return mSrcPin; }
	PinItem* dstPin() const { return mDstPin; }

	void updatePath();

private:
	QPointF mSrcPos;
	QPointF mDstPos;
	PinItem* mSrcPin;
	PinItem* mDstPin;
};

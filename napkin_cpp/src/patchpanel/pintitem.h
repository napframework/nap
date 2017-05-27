#pragma once
#include <QGraphicsPathItem>
#include <QStyleOption>
#include <QApplication>

#include <nap/patch.h>

#include "../itemtypes.h"


class PlugItem;

class PinItem : public QGraphicsItem
{
public:
	enum { Type = UserType + PinItemType };
	int type() const { return Type; } // Unique ID for qgraphicsitemcast<T>()
public:
                 PinItem(PlugItem* plug);
	QPointF      attachPos();
	PlugItem* plugItem() { return mPlug; }
	QColor color() const { return mColor; }
    virtual void paint(QPainter* painter,
                       const QStyleOptionGraphicsItem* option,
                       QWidget* widget);

    virtual QRectF boundingRect() const override {
        return QRectF(0, 0, 10, 10);
    }

private:
	QPainterPath shapeFromPlug(const nap::Plug& plug) const;
	QRectF mRect;
	PlugItem* mPlug;
	QColor mColor;
};

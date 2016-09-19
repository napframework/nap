#pragma once

#include <QGraphicsEffect>
#include <QGraphicsItem>
#include <QGraphicsView>
#include "../commands.h"
#include "operatoritem.h"

class Layer : public QGraphicsItem
{
public:
    enum { Type = UserType + LayerType };
    int type() const { return Type; }
public:
	Layer() : QGraphicsItem() {}
	QRectF boundingRect() const { return childrenBoundingRect(); }
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
	{
		Q_UNUSED(painter)
		Q_UNUSED(option)
		Q_UNUSED(widget)
	}
};


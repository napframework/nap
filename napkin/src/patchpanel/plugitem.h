#pragma once

#include <QtWidgets/QStylePainter>
#include <QtWidgets/QApplication>
#include <QtWidgets/QStyleOption>
#include "nap/patch.h"
#include "operatoritem.h"
#include "pintitem.h"
#include "wireitem.h"

class OperatorItem;
class WireItem;

class PlugItem : public QGraphicsItem
{
public:
	enum { Type = UserType + PlugItemType };
	int type() const { return Type; }
public:
	PlugItem(OperatorItem* operatorItem, nap::Plug& plug);

	QRectF boundingRect() const;
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    }
	void setName(const QString name);
	PinItem& pin() const { return *mPin; }
	OperatorItem* operatorItem() const { return mOperator; }
	nap::Plug& plug() const { return mPlug; }

	bool isInput() const { return mPlug.getTypeInfo().isKindOf<nap::InputPlugBase>(); }

	void setUsable(bool usable) { setVisible(usable); }
	bool isUsable() const { return isVisible(); }
private:
	OperatorItem* mOperator;
	QGraphicsTextItem* mValue;
	QGraphicsTextItem* mLabel;
	PinItem* mPin;
	nap::Plug& mPlug;
	WireItem* mConnectedWire;
	void layout();
};

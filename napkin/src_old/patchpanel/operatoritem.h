#pragma once

#include <QColor>
#include <QGraphicsEffect>
#include <QGraphicsItem>
#include <nap/logger.h>
#include <nap/patch.h>

#include "../itemtypes.h"
#include "plugitem.h"


namespace NetworkStyle
{
	const QColor Col_Node(0xE0, 0xE0, 0xE0);
	const QColor Col_Node_Title(0xD0, 0xD0, 0xD0);
	const QColor Col_NodeBorder(0xDD, 0xDD, 0xDD);
	const QColor Col_NodeBorder_Selected(0x55, 0x55, 0x55);
	const QColor Col_DefaultPin(0x88, 0x88, 0x88);
	const QColor Col_BG_GridLines(0xDE, 0xDE, 0xDE);
	const QColor Col_NodeConnection(0x70, 0x70, 0x70);
	const QColor Col_NodeConnection_Selected(0xFF, 0x88, 0x00);
	const qreal PinSize = 10;
}


class OperatorItem;

class SmallTextItem;
class PlugItem;
class PinItem;
class WireItem;

class ValueItem : public QGraphicsTextItem
{
public:
	ValueItem(PlugItem* plug);

private:
	PlugItem* mPlug;
};

class OperatorItem : public QGraphicsObject
{
	Q_OBJECT
public:
	enum { Type = UserType + OperatorItemType };
	int type() const { return Type; }
public:
	OperatorItem(QGraphicsItem* parent, nap::Operator& op);
	~OperatorItem();

	QRectF boundingRect() const override;
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
	void setTitle(const QString name);
	PlugItem* findPlugItem(const nap::Plug& plug) const;
	const std::string& name() const;

	nap::Operator& getOperator() const { return mOperator; }

	QList<WireItem*> connectedWires() const;

	void hideIncompatiblePlugs(PlugItem* src);
	void showAllPlugs();
	void layout();

	QList<PlugItem*> plugs() const { return mInputPlugs + mOutputPlugs; }

	QVariant itemChange(GraphicsItemChange change, const QVariant& value);
signals:
	void plugConnected(const nap::OutputPlugBase& srcPlug, const nap::InputPlugBase& dstPlug);
	void plugDisconnected(const nap::OutputPlugBase& srcPlug, const nap::InputPlugBase& dstPlug);

private:
	void onInputPlugAdded(nap::InputPlugBase& plug);
	nap::Slot<nap::InputPlugBase&> onInputPlugAddedSlot = {this, &OperatorItem::onInputPlugAdded};
	void onOutputPlugAdded(nap::OutputPlugBase& plug);
	nap::Slot<nap::OutputPlugBase&> onOutputPlugAddedSlot = {this, &OperatorItem::onOutputPlugAdded};

	void onPlugRemoved(nap::Plug& plug);
	nap::Slot<nap::Plug&> onPlugRemovedSlot = {this, &OperatorItem::onPlugRemoved};

	void onAttributeChanged(nap::AttributeBase& attrib);
	nap::Slot<nap::AttributeBase&> onAttributeChangedSlot = {this, &OperatorItem::onAttributeChanged};

	SmallTextItem* mTitleLabel;
	QList<PlugItem*> mInputPlugs;
	QList<PlugItem*> mOutputPlugs;
	QRectF mTitleRect;
	nap::Operator& mOperator;
	qreal operatorBorder;
	qreal operatorBorderSelected;
};

class SmallTextItem : public QGraphicsTextItem
{
public:
	SmallTextItem(QGraphicsItem* parent = 0) : QGraphicsTextItem(parent) {}
	QRectF boundingRect() const;
};

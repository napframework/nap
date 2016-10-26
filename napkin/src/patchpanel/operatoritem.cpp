#include "operatoritem.h"
#include <nap/object.h>
#include <QGraphicsScene>
#include <QPainter>
#include "../napkin_utilities.h"
#include <QStyleOption>

inline void moveToFront(QGraphicsItem* item)
{
	QList<QGraphicsItem*> items = item->scene()->items();
	qreal highest = -10000000;
	foreach (QGraphicsItem* item, items)
		highest = qMax(highest, item->zValue());
	item->setZValue(highest + 1);
}

OperatorItem::OperatorItem(QGraphicsItem* parent, nap::Operator& op) : QGraphicsObject(parent), mOperator(op)
{
	setPos(getObjectEditorPosition(op));

	mTitleLabel = new SmallTextItem(this);
	setFlag(QGraphicsItem::ItemIsFocusable, true);
	setFlag(QGraphicsItem::ItemIsSelectable, true);
	// setFlag(QGraphicsItem::ItemIsMovable, true);

	operatorBorder = 1;
	operatorBorderSelected = 2;
//
//	QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(this);
//	shadow->setBlurRadius(8.0);
//	shadow->setColor(QColor(0x00, 0x10, 0x20, 0x63));
//	shadow->setOffset(1, 2);
//	shadow->setEnabled(true);
//	setGraphicsEffect(shadow);

	setTitle(QString::fromStdString(op.getName()));

    // TODO: THESE SIGNALS HAVE BEEN REMOVED? (COEN)
    /*
	mOperator.childAdded.connect(onInputPlugAddedSlot);
	mOperator.childAdded.connect(onOutputPlugAddedSlot);
	mOperator.childRemoved.connect(onPlugRemovedSlot);
	mOperator.childRemoved.connect(onPlugRemovedSlot);
    */

    // Listen for stuff like position changes
    // TODO: This must be made simpler
    {
        mOperator.childAdded.connect([&](nap::Object& ob) {
            if (!ob.getTypeInfo().isKindOf<nap::AttributeBase>()) {
                nap::AttributeBase& attrib = static_cast<nap::AttributeBase&>(ob);
                attrib.valueChanged.connect([=](nap::AttributeBase& at) {
                    setPos(getObjectEditorPosition(mOperator));
                });
            }
        });

        for (nap::AttributeBase* attrib : mOperator.getAttributes()) {
            attrib->valueChanged.connect([&](nap::AttributeBase& at) {
                setPos(getObjectEditorPosition(mOperator));
            });
        }
    }

	for (auto plug : mOperator.getInputPlugs())
		onInputPlugAdded(*plug);
	for (auto plug : mOperator.getOutputPlugs())
		onOutputPlugAdded(*plug);

	layout();
}

OperatorItem::~OperatorItem()
{
	/*
	mOperator->plugAdded.Disconnect(this, &OperatorItem::onPlugRemoved);
	mOperator->plugRemoved.Disconnect(this, &OperatorItem::onPlugRemoved);
	mOperator->changed.Disconnect(this, &OperatorItem::onAttributeChanged);
	*/
}


QRectF OperatorItem::boundingRect() const
{
	qreal b = isSelected() ? operatorBorderSelected : operatorBorder;
	b = operatorBorder;
	return childrenBoundingRect().adjusted(-b, -b, b, b);
}


void OperatorItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	QStyleOptionGraphicsItem op = *option;

//	painter->drawRect(boundingRect());

	//	if (isSelected()) {
//	painter->setBrush(isSelected() ? QColor(0xFF, 0x88, 0x00) : QColor(0x80, 0x80, 0x80));
	//	}

//	painter->setBrush(NetworkStyle::Col_Node);
//	painter->drawRect(childrenBoundingRect());

	painter->setBrush(NetworkStyle::Col_Node_Title);
	painter->drawRect(mTitleRect);

	painter->setBrush(QApplication::palette().window());
	if (isSelected()) {
		painter->setPen(QPen(QApplication::palette().highlight(), 0));
	}else {
		painter->setPen(QPen(QApplication::palette().dark(), 0));
	}
	painter->drawRect(boundingRect());





//	QApplication::style()->drawPrimitive(QStyle::PE_FrameDefaultButton, option, painter, widget);
}

void OperatorItem::setTitle(const QString name)
{
	mTitleLabel->setPlainText(name);
	layout();
}


void OperatorItem::hideIncompatiblePlugs(PlugItem* src)
{
	for (PlugItem* plug : mInputPlugs) {
		if (plug == src) continue; // Do show the originating plug
		if (!canConnect(plug->plug(), src->plug())) plug->setUsable(false);
	}
	for (PlugItem* plug : mOutputPlugs) {
		if (plug == src) continue; // Do show the originating plug
		if (!canConnect(plug->plug(), src->plug())) plug->setUsable(false);
	}
}

void OperatorItem::showAllPlugs()
{
	for (PlugItem* plug : mInputPlugs)
		plug->setUsable(true);
	for (PlugItem* plug : mOutputPlugs)
		plug->setUsable(true);
}

void OperatorItem::layout()
{
	const qreal headerHeight = 20;
	const qreal headerSpacing = 6;
	const qreal itemSpacing = 14;

	qreal currentX = 0;

	// Inputs
	QList<PlugItem*> inPlugs = mInputPlugs;
	for (int i = 0, len = inPlugs.length(); i < len; i++) {
		PlugItem* item = inPlugs[i];
		item->setPos(0, headerHeight + headerSpacing + i * itemSpacing);
		currentX = qMax(currentX, item->boundingRect().width());
	}

	qreal maxX = currentX;
	// Outputs
	QList<PlugItem*> outPlugs = mOutputPlugs;
	for (int i = 0, len = outPlugs.length(); i < len; i++) {
		maxX = qMax(maxX, currentX + outPlugs[i]->boundingRect().width());
	}


	mTitleRect = mTitleLabel->boundingRect();
	mTitleRect.setWidth(childrenBoundingRect().width());
	mTitleLabel->setPos(mTitleRect.right() - mTitleLabel->boundingRect().width(), 0);

	maxX = qMax(maxX, mTitleRect.width());

	for (int i = 0, len = outPlugs.length(); i < len; i++) {
		PlugItem* item = outPlugs[i];
		float w = item->boundingRect().width();
		item->setPos(maxX - w, headerHeight + headerSpacing + i * itemSpacing);
	}
}

QVariant OperatorItem::itemChange(GraphicsItemChange change, const QVariant& value)
{
	if (change == QGraphicsItem::ItemSelectedHasChanged) moveToFront(this);

	return QGraphicsItem::itemChange(change, value);
}

void OperatorItem::onInputPlugAdded(nap::InputPlugBase& plug)
{
	// Listen for plug connections
	plug.connectedSignal.connect([&](nap::OutputPlugBase& outPlug) {
		plugConnected(outPlug, plug);
	});
	plug.disconnectedSignal.connect([&](nap::OutputPlugBase& outPlug) {
		plugDisconnected(outPlug, plug);
	});

	auto plugItem = new PlugItem(this, plug);
	mInputPlugs << plugItem;
	layout();
}

void OperatorItem::onOutputPlugAdded(nap::OutputPlugBase& plug)
{
	auto plugItem = new PlugItem(this, plug);
	mOutputPlugs << plugItem;
	layout();
}

void OperatorItem::onPlugRemoved(nap::Plug& plug) { delete findPlugItem(plug); }

void OperatorItem::onAttributeChanged(nap::AttributeBase& attrib) { setPos(getObjectEditorPosition(mOperator)); }

PlugItem* OperatorItem::findPlugItem(const nap::Plug& plug) const
{
	for (auto item : mInputPlugs)
		if (&item->plug() == &plug) return item;
	for (auto item : mOutputPlugs)
		if (&item->plug() == &plug) return item;
	return nullptr;
}

const std::string& OperatorItem::name() const { return mOperator.getName(); }


QRectF SmallTextItem::boundingRect() const { return QGraphicsTextItem::boundingRect().adjusted(0, 0, -2, -2); }


ValueItem::ValueItem(PlugItem* plug) : QGraphicsTextItem(), mPlug(plug)
{
	/*
	InPlug* inPlug = qobject_cast<InPlug*>(plug->plug());
	//Q_ASSERT(inPlug);
	setPlainText(inPlug->value().toString());
	*/


}

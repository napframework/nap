#include "patchscene.h"
#include <nap/operator.h>

template <typename T>
inline QList<T> filter(const QList<QGraphicsItem*>& list)
{
	QList<T> filtered;
	for (QGraphicsItem* item : list) {
		T t = qgraphicsitem_cast<T>(item);
		if (t) filtered.append(t);
	}
	return filtered;
}


PatchScene::PatchScene(nap::Patch& patch) : QGraphicsScene(nullptr), mPatch(patch)
{
	mIsWiring = false;

	mOperatorLayer = new Layer();
	mWireLayer = new Layer();
	mInteractionLayer = new Layer();
	mInteractionLayer->setAcceptedMouseButtons(Qt::NoButton);
	mInteractionLayer->setAcceptHoverEvents(false);
	mInteractionLayer->setAcceptTouchEvents(false);
	mInteractionLayer->setEnabled(false);

	mPreviewWire = new WirePreview(mInteractionLayer);
	mPreviewWire->setEnabled(false);
	mPreviewWire->setParentItem(mInteractionLayer);
	mPreviewWire->setVisible(false);

	addItem(mWireLayer);
	addItem(mOperatorLayer);
	addItem(mInteractionLayer);

	setSceneRect(-1000, -1000, 2000, 2000);

	connect(this, SIGNAL(selectionChanged()), this, SLOT(onSelectionChanged()));

	mPatch.operatorAdded.connect(this->onOperatorAddedSlot);
	mPatch.operatorWillBeRemoved.connect(this->onOperatorRemovedSlot);

	for (auto op : mPatch.getOperators()) {
		nap::Operator* opp = op;
		onOperatorAdded(*opp);
	}
}

void PatchScene::onSelectionChanged()
{
	QList<nap::Operator*> nodes;
	for (QGraphicsItem* item : selectedItems()) {
		OperatorItem* n = qgraphicsitem_cast<OperatorItem*>(item);
		if (!n) continue;
		nodes.append(&n->getOperator());
	}

    QList<nap::Object*> objects;
    for (auto op : nodes) {
        objects << op;
    }
    AppContext::get().setSelection(objects);

	operatorSelectionChanged(nodes);
}

void PatchScene::startDragConnection(PinItem* pin)
{
	hideIncompatiblePlugs(pin->plugItem());

	mWireIsOutput = !pin->plugItem()->isInput();
	if (mWireIsOutput) {
		mPreviewWire->setSrcPin(pin);
		mPreviewWire->setDstPos(pin->attachPos());
	} else {
		mPreviewWire->setDstPin(pin);
		mPreviewWire->setSrcPos(pin->attachPos());
	}

	mPreviewWire->setVisible(true);
	mIsWiring = true;
}

void PatchScene::updateDragConnection(QPointF pt)
{
	if (mWireIsOutput) {
		mPreviewWire->setDstPos(pt);
	} else {
		mPreviewWire->setSrcPos(pt);
	}
	mPreviewWire->updatePath();
}

void PatchScene::stopDragConnection(PlugItem* plug)
{
	if (!mIsWiring) return;

	if (plug) { // Only do this when we got a pin
		PlugItem* srcPlug;
		PlugItem* dstPlug;
		if (mWireIsOutput) {
			srcPlug = mPreviewWire->srcPin()->plugItem();
			dstPlug = plug;
		} else {
			srcPlug = plug;
			dstPlug = mPreviewWire->dstPin()->plugItem();
		}

		if (srcPlug != dstPlug)
			AppContext::get().execute(
				new ConnectPlugsCmd((nap::OutputPlugBase&)srcPlug->plug(), (nap::InputPlugBase&)dstPlug->plug()));
	}
	mPreviewWire->setSrcPin(NULL);
	mPreviewWire->setDstPin(NULL);
	mPreviewWire->setVisible(false);
	mIsWiring = false;

	showAllPlugs();
}

void PatchScene::drawBackground(QPainter* painter, const QRectF& rect)
{
	qreal spacing = 50;
	int xMin = qFloor(rect.left() / spacing);
	int xMax = qCeil(rect.right() / spacing);
	int yMin = qFloor(rect.top() / spacing);
	int yMax = qCeil(rect.bottom() / spacing);

	QPen pen;
	pen.setWidth(0);
	pen.setColor(QApplication::palette().base().color().darker(110));
	painter->setPen(pen);

	for (int x = xMin; x < xMax; x++) {
		painter->drawLine(QPointF(x, yMin) * spacing, QPointF(x, yMax) * spacing);
	}
	for (int y = yMin; y < yMax; y++) {
		painter->drawLine(QPointF(xMin, y) * spacing, QPointF(xMax, y) * spacing);
	}
}

QList<OperatorItem*> PatchScene::operators() const { return filter<OperatorItem*>(mOperatorLayer->childItems()); }

QList<WireItem*> PatchScene::wires() const { return filter<WireItem*>(mWireLayer->childItems()); }

nap::Patch& PatchScene::patch() const { return mPatch; }

void PatchScene::hideIncompatiblePlugs(PlugItem* src)
{
	for (OperatorItem* node : operators())
		node->hideIncompatiblePlugs(src);
}

void PatchScene::showAllPlugs()
{
	for (OperatorItem* node : operators())
		node->showAllPlugs();
}

QList<OperatorItem*> PatchScene::selectedOperatorItems() const { return filter<OperatorItem*>(selectedItems()); }



QList<WireItem*> PatchScene::selectedWires() const { return filter<WireItem*>(selectedItems()); }

void PatchScene::removeOperatorItem(OperatorItem* item) { delete item; }

OperatorItem* PatchScene::findOperatorItem(const std::string& id) const
{
	for (OperatorItem* node : operators())
		if (node->name() == id) return node;
	return nullptr;
}

OperatorItem* PatchScene::findOperatorItem(nap::Operator& op) const { return findOperatorItem(op.getName()); }

nap::Plug& PatchScene::dragConnectionSource() const
{
	if (mPreviewWire->srcPin()) return mPreviewWire->srcPin()->plugItem()->plug();
	return mPreviewWire->dstPin()->plugItem()->plug();
}

void PatchScene::onOperatorAdded(nap::Operator& op)
{
	OperatorItem* item = new OperatorItem(mOperatorLayer, op);

	connect(item, &OperatorItem::plugConnected,
			[=](const nap::OutputPlugBase& srcPlug, const nap::InputPlugBase& dstPlug) { addWire(srcPlug, dstPlug); });

	connect(item, &OperatorItem::plugDisconnected,
			[=](const nap::OutputPlugBase& srcPlug, const nap::InputPlugBase& dstPlug) {
				assert(false); // Not implemented
			});

	item->setParentItem(mOperatorLayer);
}

void PatchScene::onOperatorRemoved(nap::Operator& op)
{
	OperatorItem* item = findOperatorItem(op);
	assert(item);
	removeOperatorItem(item);
}

void PatchScene::onOperatorChanged(nap::Operator& op) { findOperatorItem(op)->setPos(getObjectEditorPosition(op)); }


void PatchScene::addWire(const nap::OutputPlugBase& srcPlug, const nap::InputPlugBase& dstPlug)
{
	OperatorItem* srcNodeItem = findOperatorItem(*srcPlug.getParent());
	PlugItem* srcPlugItem = srcNodeItem->findPlugItem(srcPlug);

	OperatorItem* dstNodeItem = findOperatorItem(*dstPlug.getParent());
	PlugItem* dstPlugItem = dstNodeItem->findPlugItem(dstPlug);

	WireItem* item = new WireItem(srcPlugItem->pin(), dstPlugItem->pin());
	item->setParentItem(mWireLayer);
}

// void PatchScene::onWireRemoved(nap::Wire* wire) { delete findWire(wire); }

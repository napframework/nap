#pragma once

#include <QGraphicsEffect>
#include <QGraphicsItem>
#include <QGraphicsView>
#include <QHash>
#include <QMouseEvent>
#include <QPainter>
#include <QtDebug>
#include <QtMath>

#include "../commands.h"
#include "operatoritem.h"
#include "plugitem.h"
#include "wirepreviewitem.h"
#include "layeritem.h"

class Plug;
class Operator;
class Patch;
class Layer;

class PinItem;
class OperatorItem;
class WireItem;
class PlugItem;

class SmallTextItem;
class PatchScene;
class OperatorFactory;
class WirePreview;


// The patch scene acts as a model for the flow based programming environment and holds the Patch containing all logic
class PatchScene : public QGraphicsScene
{
	Q_OBJECT
public:
	explicit PatchScene(nap::Patch& patch);

	void startDragConnection(PinItem* pin);
	void updateDragConnection(QPointF p);
	void stopDragConnection(PlugItem* plug);

	void drawBackground(QPainter* painter, const QRectF& rect);

	QList<OperatorItem*> operators() const;
	QList<WireItem*> wires() const;

	nap::Patch& patch() const;

	void hideIncompatiblePlugs(PlugItem* src);
	void showAllPlugs();

	QList<OperatorItem*> selectedOperatorItems() const;
	QList<nap::Operator*> selectedOperators() const
	{
		QList<nap::Operator*> ops;
		for (OperatorItem* opItem : selectedOperatorItems())
			ops << &opItem->getOperator();
		return ops;
	}
	QList<WireItem*> selectedWires() const;

	OperatorItem* findOperatorItem(const std::string& id) const;
	OperatorItem* findOperatorItem(nap::Operator& op) const;

	nap::Plug& dragConnectionSource() const;



signals:
	void operatorSelectionChanged(QList<nap::Operator*> operators);

private slots:
	void onSelectionChanged();


//		void onWireAdded(nap::Wire* wire);
//		void onWireRemoved(nap::Wire* wire);

private:
	//	WireItem* findWire(nap::Wire* wire);

	void addWire(const nap::OutputPlugBase& srcPlug, const nap::InputPlugBase& dstPlug);

	void onOperatorAdded(nap::Operator& op);
	nap::Slot<nap::Operator&> onOperatorAddedSlot = {this, &PatchScene::onOperatorAdded};

	void onOperatorRemoved(nap::Operator& op);
	nap::Slot<nap::Operator&> onOperatorRemovedSlot = {this, &PatchScene::onOperatorRemoved};

	void onOperatorChanged(nap::Operator& op);
	nap::Slot<nap::Operator&> onOperatorChangedSlot = {this, &PatchScene::onOperatorChanged};


	void removeOperatorItem(OperatorItem* node);
	void removeOperator(const std::string& id);

	bool mWireIsOutput; // For the preview wire

	nap::Patch& mPatch;

	Layer* mInteractionLayer;
	Layer* mWireLayer;
	Layer* mOperatorLayer;

	WirePreview* mPreviewWire;
	bool mIsWiring;

};



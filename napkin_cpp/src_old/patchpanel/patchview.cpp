#include "patchview.h"


PatchView::PatchView(QWidget* parent) : QGraphicsView(parent)
{
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
	setRenderHint(QPainter::Antialiasing, true);
	setInteractMode(Default);
	setFocusPolicy(Qt::StrongFocus);
	setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, &QGraphicsView::customContextMenuRequested, this, &PatchView::onCustomContextMenuRequested);
}

void PatchView::setInteractMode(InteractMode mode)
{
//	nap::Logger::debug("Interactmode %s", std::to_string((int) mode).c_str());

	mInteractMode = mode;
	switch (mode) {
	case PatchView::Default:
		viewport()->unsetCursor();
		setDragMode(QGraphicsView::RubberBandDrag);
		break;

	case PatchView::PanMode:
		viewport()->setCursor(Qt::ClosedHandCursor);
		setDragMode(QGraphicsView::NoDrag);
		break;

	case PatchView::ConnectMode:
		setDragMode(QGraphicsView::NoDrag);
		break;

	default:
		break;
	}
}

bool PatchView::focusNextPrevChild(bool next) { return false; }

void PatchView::keyPressEvent(QKeyEvent* evt)
{
	if (evt->key() == Qt::Key_Tab) {

		QMenu menu;
		menu.exec(actions(), QCursor::pos());
	}
}

PinItem* PatchView::pinAt(const QPoint& viewPos) const
{
	foreach (QGraphicsItem* item, items(viewPos)) {
		PinItem* castItem = qgraphicsitem_cast<PinItem*>(item);
		if (castItem) return castItem;
	}
	return nullptr;
}

PlugItem* PatchView::plugAt(const QPoint& viewPos) const
{
	foreach (QGraphicsItem* item, items(viewPos)) {
		PlugItem* castItem = qgraphicsitem_cast<PlugItem*>(item);
		if (castItem && &castItem->plug()) return castItem;
	}
	return nullptr;
}

OperatorItem* PatchView::nodeAt(const QPoint& viewPos) const
{
	foreach (QGraphicsItem* item, items(viewPos)) {
		OperatorItem* castItem = qgraphicsitem_cast<OperatorItem*>(item);
		if (castItem) return castItem;
	}
	return nullptr;
}

void PatchView::mousePressEvent(QMouseEvent* evt)
{

	QGraphicsItem* item = itemAt(evt->pos());
	if (item) item->setVisible(true);
	PinItem* pin = pinAt(evt->pos());
	OperatorItem* node = nodeAt(evt->pos());
	PlugItem* plug = plugAt(evt->pos());

	mOldMousePos = evt->pos();
	mMouseClickPos = evt->pos();

	if (mInteractMode == Default) {

		if (evt->buttons().testFlag(Qt::LeftButton)) {
			if (pin) return;

			if (node) {
				QGraphicsView::mousePressEvent(evt);
				setInteractMode(DragMode);
				return;
			}
		} else if (evt->buttons().testFlag(Qt::MiddleButton)) {
			setTransformationAnchor(QGraphicsView::NoAnchor);
			setInteractMode(PanMode);
			return;
		} else if (evt->buttons().testFlag(Qt::RightButton)) {
			return;
		}
		QGraphicsView::mousePressEvent(evt);
	} else if (mInteractMode == ConnectMode) {
        patchScene()->stopDragConnection(plug);
//		setInteractMode(Default);
		return;
	}
}

void PatchView::mouseMoveEvent(QMouseEvent* evt)
{
	PinItem* pin = pinAt(evt->pos());
	PlugItem* plug = plugAt(evt->pos());

	QPoint delta = evt->pos() - mOldMousePos;

	if (mInteractMode == Default) {
		if (pin)
			viewport()->setCursor(Qt::PointingHandCursor);
		else
			viewport()->unsetCursor();
	} else if (mInteractMode == ConnectMode) {
		nap::Plug& srcPlug = patchScene()->dragConnectionSource();
		QPointF pt = mapToScene(evt->pos());

		if (pin) {
            plug = pin->plugItem();

            if (plug && &plug->plug() && canConnect(srcPlug, plug->plug())) {
                pt = plug->pin().attachPos();
                viewport()->setCursor(Qt::PointingHandCursor);
            } else {
                viewport()->unsetCursor();
            }
        }
        patchScene()->updateDragConnection(pt.toPoint());
	} else if (mInteractMode == PanMode) {
		translate(delta.x(), delta.y());
	} else if (mInteractMode == DragMode) {
		foreach (QGraphicsItem* item, patchScene()->selectedItems()) {
			OperatorItem* node = qgraphicsitem_cast<OperatorItem*>(item);
			if (!node) continue;
			node->moveBy(delta.x(), delta.y());
		}
	}

	mOldMousePos = evt->pos();
	QGraphicsView::mouseMoveEvent(evt);
}

void PatchView::mouseReleaseEvent(QMouseEvent* evt)
{
	PinItem* pin = pinAt(evt->pos());

	if (mInteractMode == Default) {
		if (pin) {
            patchScene()->startDragConnection(pin);
			setInteractMode(ConnectMode);
			return;
		}
	} else if (mInteractMode == ConnectMode) {
		setInteractMode(Default);
	} else if (mInteractMode == PanMode) {
		setInteractMode(Default);
		return;
	} else if (mInteractMode == DragMode) {
		QPointF delta = evt->pos() - mMouseClickPos;
		if (!delta.isNull()) {
			AppContext::get().execute(new MoveOperatorsCmd(patchScene()->selectedOperators(), delta));
		}

		setInteractMode(Default);
	}
	QGraphicsView::mouseReleaseEvent(evt);
}

void PatchView::wheelEvent(QWheelEvent* evt)
{
	setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

	double scaleFactor = 1.15;
	if (evt->delta() > 0) {
		scale(scaleFactor, scaleFactor);
	} else {
		scale(1.0 / scaleFactor, 1.0 / scaleFactor);
	}
}

PatchScene* PatchView::patchScene() { return (PatchScene*)scene(); }

void PatchView::resetZoom() {
	qreal dx = transform().dx();
	qreal dy = transform().dy();

	QTransform xf = QTransform::fromTranslate(dx, dy);
	this->setTransform(xf);
}

//void PatchView::onNodeTypeRegistered(const OperatorTypeBase* nodeType)
//{
//	addAction(new CreateOperatorAction(this, nodeType->id()));
//}

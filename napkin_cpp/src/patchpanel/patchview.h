#pragma once

#include "nap/patch.h"
#include "patchscene.h"
#include <QMenu>

// The patch view also acts as a controller
class PatchView : public QGraphicsView
{
	Q_OBJECT
public:
	enum InteractMode { Default, PanMode, ConnectMode, DragMode };

	explicit PatchView(QWidget* parent = 0);
	void mousePressEvent(QMouseEvent* evt) override;
	void mouseMoveEvent(QMouseEvent* evt) override;
	void mouseReleaseEvent(QMouseEvent* evt) override;
	void wheelEvent(QWheelEvent* evt) override;
	void setInteractMode(InteractMode mode);

	void resetZoom();

	void onCustomContextMenuRequested(const QPoint& pos)
	{
		QMenu menu;

		QList<QAction*> actions;
		for (QAction* action : AppContext::get().actionStore().actionsFor(patchScene()->patch())) {
			if (!actions.contains(action)) actions << action;
		}
		menu.addActions(actions);

		if (menu.actions().size() > 0) menu.exec(viewport()->mapToGlobal(pos));
	}

	virtual bool focusNextPrevChild(bool next) override;
	virtual void keyPressEvent(QKeyEvent* evt) override;
	PinItem* pinAt(const QPoint& scenePos) const;
	PlugItem* plugAt(const QPoint& scenePos) const;
	OperatorItem* nodeAt(const QPoint& scenePos) const;
	PatchScene* patchScene();

	QPointF mouseClickPos() const { return mMouseClickPos; }

private:
	QPoint mOldMousePos;
	QPoint mMouseClickPos;
	InteractMode mInteractMode;

	bool isValidConnection(PinItem* src, PinItem* dst);
};


/*!
 * The QStandardItem derived classes expose/mirror the behavior and properties of a subset of nap::Node for use int the
 * outline panel.
 * NodeItem acts as a convenience base for derived QStandardItem classes.
 */

#pragma once

#include "../appcontext.h"
#include "ui_scenepanel.h"


namespace Ui
{
	class ScenePanel;
}


class ScenePanel : public QDockWidget
{
	Q_OBJECT

public:
	explicit ScenePanel(QWidget* parent = 0);
	~ScenePanel() {}


private slots:
    void onCoreChanged(nap::Core* core) {  }
	void onCustomContextMenuRequested(const QPoint& pos) {}

private:
	nap::Entity* selectedEntity() const;
	template <typename T>
	QList<T*> selection() const;


	Ui::ScenePanel ui;
};


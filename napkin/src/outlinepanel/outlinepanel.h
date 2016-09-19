/*!
 * The QStandardItem derived classes expose/mirror the behavior and properties of a subset of nap::Node for use int the
 * outline panel.
 * NodeItem acts as a convenience base for derived QStandardItem classes.
 */

#pragma once

#include <QDockWidget>
#include <QList>
#include <QMenu>
#include <QStandardItemModel>

#include "ui_outlinepanel.h"

#include "objectitem.h"
#include "outlinemodel.h"
#include "../appcontext.h"
#include "outlinefilter.h"


namespace Ui
{
	class OutlinePanel;
}


class OutlinePanel : public QDockWidget
{
	Q_OBJECT

public:
	explicit OutlinePanel(QWidget* parent = 0);
	~OutlinePanel();

protected:
    virtual void showEvent(QShowEvent* event) override {
        mModel.setCore(&AppContext::get().core());

    }


private slots:
	void onCustomContextMenuRequested(const QPoint& pos);
	void onRowsInserted(const QModelIndex& parent, int start, int end);
	void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

private:
	template <typename T>
	QList<T*> selection() const;


	Ui::OutlinePanel ui;
	OutlineModel mModel;
	OutlineFilterProxy mFilterProxy;
};



template <typename T>
QList<T*> OutlinePanel::selection() const
{
	QList<T*> selected;
	for (auto index : ui.treeView->selectionModel()->selectedIndexes()) {
		ObjectItem* item = (ObjectItem*)mModel.itemFromIndex(index);
		if (!item) continue;

        auto& object = item->object();
        if (!object.getTypeInfo().isKindOf<T>())
            continue;
        selected << static_cast<T*>(&object);
	}
	return selected;
}

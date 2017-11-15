#include "outlinepanel.h"


OutlinePanel::OutlinePanel(QWidget* parent) : QDockWidget(parent)
{
	ui.setupUi(this);
	ui.treeView->setModel(&mModel);

	connect(ui.treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this,
			&OutlinePanel::onSelectionChanged);

	connect(ui.treeView, &QTreeView::customContextMenuRequested, this, &OutlinePanel::onCustomContextMenuRequested);
	connect(&mModel, &QStandardItemModel::rowsInserted, this, &OutlinePanel::onRowsInserted);

    connect(ui.btShowComponents, &QAbstractButton::clicked, [&](bool checked) {
		mFilterProxy.setShowComponents(checked);
	});

	connect(ui.btShowAttributes, &QAbstractButton::clicked, [&](bool checked) {
		mFilterProxy.setShowAttributes(checked);
	});

}

OutlinePanel::~OutlinePanel() {}

void OutlinePanel::onCustomContextMenuRequested(const QPoint& pos)
{
	QMenu menu;

    QList<QAction*> actions;
    for (auto obj : AppContext::get().selection()) {
        for (QAction* action : AppContext::get().actionStore().actionsFor(*obj)) {
            if (!actions.contains(action))
                actions << action;
        }
    }
    menu.addActions(actions);

	if (menu.actions().size() > 0) menu.exec(ui.treeView->viewport()->mapToGlobal(pos));
}

// React to items being added
void OutlinePanel::onRowsInserted(const QModelIndex& parent, int start, int end)
{
	// Update selection to new items
	QItemSelectionModel* selectionModel = ui.treeView->selectionModel();
	selectionModel->clear();

	for (int row = start; row <= end; row++) {
		auto idx = parent.child(row, 0);
		selectionModel->select(idx, QItemSelectionModel::Select);
	}

	// Expand parent to show them
	QModelIndex idx = parent;
	while (idx.isValid()) {
		ui.treeView->setExpanded(idx, true);
		idx = idx.parent();
	}
}

void OutlinePanel::onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
	QList<nap::Object*> objectSelection;

	for (QModelIndex idx : ui.treeView->selectionModel()->selectedIndexes()) {
		ObjectItem* objectItem = (ObjectItem*)mModel.itemFromIndex(idx);
		if (!objectItem) continue;

		objectSelection.append(&objectItem->object());
	}
	AppContext::get().setSelection(objectSelection);
}

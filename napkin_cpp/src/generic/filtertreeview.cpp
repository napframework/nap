#include "filtertreeview.h"

FilterTreeView::FilterTreeView() {
    layout.setContentsMargins(0, 0, 0, 0);
    layout.setSpacing(0);
    setLayout(&layout);

    leFilter.setPlaceholderText("filter");
    leFilter.setClearButtonEnabled(true);
    connect(&leFilter, &QLineEdit::textChanged, this, &FilterTreeView::onFilterChanged);
    layout.addWidget(&leFilter);

    treeView.setModel(&sortFilter);

    layout.addWidget(&treeView);

    setContextMenuPolicy(Qt::ActionsContextMenu);

    actionExpandAll.setText("Expand All");
    addAction(&actionExpandAll);
    actionCollapseAll.setText("Collapse");
    addAction(&actionCollapseAll);
}

void FilterTreeView::setModel(QStandardItemModel* model) {
    sortFilter.setSourceModel(model);
}

QStandardItemModel* FilterTreeView::model() {
    return dynamic_cast<QStandardItemModel *>(sortFilter.sourceModel());
}

QStandardItem* FilterTreeView::selectedItem() {
    for (auto idx : selectedIndexes())
        return model()->itemFromIndex(idx);
    return nullptr;
}


QList<QStandardItem*> FilterTreeView::selectedItems() {
    QList<QStandardItem *> ret;
    for (auto idx : selectedIndexes())
        ret.append(model()->itemFromIndex(idx));
    return ret;
}

QList<QModelIndex> FilterTreeView::selectedIndexes() {
    QList<QModelIndex> ret;
    for (auto idx : selectionModel()->selectedRows())
        ret.append(sortFilter.mapToSource(idx));
    return ret;
}

void FilterTreeView::onFilterChanged(const QString& text) {
    sortFilter.setFilterRegExp(text);
}


void FilterTreeView::onExpandSelected() {
    for (auto& idx : selectedIndexes())
        expandChildren(&treeView, idx, true);
}

void FilterTreeView::onCollapseSelected() {
    for (auto& idx : selectedIndexes())
        expandChildren(&treeView, idx, false);
}

void FilterTreeView::expandChildren(QTreeView* view, const QModelIndex& idx, bool expanded) {
    if (!idx.isValid())
        return;

    for (int i = 0, len = idx.model()->rowCount(idx); i < len; i++)
        expandChildren(view, idx.child(i, 0), expanded);

    if (expanded && !view->isExpanded(idx))
        view->expand(idx);
    else if (view->isExpanded(idx))
        view->collapse(idx);
}


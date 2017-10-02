#include "filtertreeview.h"
#include <QTimer>
#include <QMenu>
#include <assert.h>

FilterTreeView::FilterTreeView()
{
    layout.setContentsMargins(0, 0, 0, 0);
    layout.setSpacing(0);
    setLayout(&layout);

    sortFilter.setFilterCaseSensitivity(Qt::CaseInsensitive);
    sortFilter.setFilterKeyColumn(-1); // Filter all columns

    leFilter.setPlaceholderText("filter");
    leFilter.setClearButtonEnabled(true);
    connect(&leFilter, &QLineEdit::textChanged, this, &FilterTreeView::onFilterChanged);
    layout.addWidget(&leFilter);

    treeView.setModel(&sortFilter);

    layout.addWidget(&treeView);

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QWidget::customContextMenuRequested, this, &FilterTreeView::onCustomContextMenuRequested);
}

void FilterTreeView::setModel(QStandardItemModel* model)
{
    sortFilter.setSourceModel(model);
}

QStandardItemModel* FilterTreeView::model() const
{
    return dynamic_cast<QStandardItemModel*>(sortFilter.sourceModel());
}

void FilterTreeView::selectAndReveal(QStandardItem* item)
{
    if (item == nullptr)
        return;
    auto idx = filterModel().mapFromSource(item->index());
    // We are going to select an entire row
    auto topleft = idx;
    auto botRight = filterModel().index(idx.row(), filterModel().columnCount(idx.parent())-1, idx.parent());
    tree().selectionModel()->select(QItemSelection(idx, botRight), QItemSelectionModel::ClearAndSelect);
    tree().scrollTo(idx);
}


QStandardItem* FilterTreeView::selectedItem()
{
    for (auto idx : selectedIndexes())
        return model()->itemFromIndex(idx);
    return nullptr;
}


QList<QStandardItem*> FilterTreeView::selectedItems() const
{
    QList<QStandardItem*> ret;
    for (auto idx : selectedIndexes())
        ret.append(model()->itemFromIndex(idx));
    return ret;
}

QList<QModelIndex> FilterTreeView::selectedIndexes() const
{
    QList<QModelIndex> ret;
    for (auto idx : selectionModel()->selectedRows())
        ret.append(sortFilter.mapToSource(idx));
    return ret;
}

void FilterTreeView::onFilterChanged(const QString& text)
{
    sortFilter.setFilterRegExp(text);
    treeView.expandAll();
}


void FilterTreeView::onExpandSelected()
{
    for (auto& idx : selectedIndexes())
        expandChildren(&treeView, idx, true);
}

void FilterTreeView::onCollapseSelected()
{
    for (auto& idx : selectedIndexes())
        expandChildren(&treeView, idx, false);
}

void FilterTreeView::expandChildren(QTreeView* view, const QModelIndex& idx, bool expanded)
{
    if (!idx.isValid())
        return;

    for (int i = 0, len = idx.model()->rowCount(idx); i < len; i++)
        expandChildren(view, idx.child(i, 0), expanded);

    if (expanded && !view->isExpanded(idx))
        view->expand(idx);
    else if (view->isExpanded(idx))
        view->collapse(idx);
}

void FilterTreeView::onCustomContextMenuRequested(const QPoint& pos)
{
    QMenu menu;

    if (mMenuHookFn != nullptr)
        mMenuHookFn(menu);

    actionExpandAll.setText("Expand All");
    menu.addAction(&actionExpandAll);
    actionCollapseAll.setText("Collapse");
    menu.addAction(&actionCollapseAll);

    menu.exec(mapToGlobal(pos));
}



#pragma once

#include <QTreeView>
#include <QWidget>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QAction>
#include <QStandardItem>
#include <QItemSelectionModel>
#include <QtWidgets/QAbstractItemView>

#include "leaffilterproxymodel.h"


class FilterTreeView : public QWidget {

public:
    FilterTreeView() {
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

    void setModel(QStandardItemModel *model) {
        sortFilter.setSourceModel(model);
    }

    QStandardItemModel *model() {
        return static_cast<QStandardItemModel *>(sortFilter.sourceModel());
    }

    QTreeView& tree() { return treeView; }


    QStandardItem *selectedItem() {
        for (auto idx : selectedIndexes())
            return model()->itemFromIndex(idx);
        return nullptr;
    }

    QList<QStandardItem *> selectedItems() {
        QList<QStandardItem *> ret;
        for (auto idx : selectedIndexes())
            ret.append(model()->itemFromIndex(idx));
        return ret;
    }

    QItemSelectionModel *selectionModel() { return treeView.selectionModel(); }

    QList<QModelIndex> selectedIndexes() {
        QList<QModelIndex> ret;
        for (auto idx : selectionModel()->selectedRows())
            ret.append(sortFilter.mapToSource(idx));
        return ret;
    }

protected:

    void onFilterChanged(const QString& text) {
        sortFilter.setFilterRegExp(text);
    }

    void onExpandSelected() {
        for (auto& idx : selectedIndexes())
            expandChildren(&treeView, idx, true);
    }

    void onCollapseSelected() {
        for (auto& idx : selectedIndexes())
            expandChildren(&treeView, idx, false);
    }

    static void expandChildren(QTreeView *view, const QModelIndex& idx, bool expanded) {
        if (!idx.isValid())
            return;

        for (int i = 0, len = idx.model()->rowCount(idx); i < len; i++)
            expandChildren(view, idx.child(i, 0), expanded);

        if (expanded && !view->isExpanded(idx))
            view->expand(idx);
        else if (view->isExpanded(idx))
            view->collapse(idx);
    }

private:

private:
    QVBoxLayout layout;
    QLineEdit leFilter;
    QTreeView treeView;
    LeafFilterProxyModel sortFilter;

    QAction actionExpandAll;
    QAction actionCollapseAll;

};
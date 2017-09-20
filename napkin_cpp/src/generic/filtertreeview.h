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
    FilterTreeView();

    void setModel(QStandardItemModel* model);

    QStandardItemModel* model();

    QTreeView& tree()
    { return treeView; }

    QStandardItem* selectedItem();

    QList<QStandardItem*> selectedItems();

    QItemSelectionModel* selectionModel()
    { return treeView.selectionModel(); }

    QList<QModelIndex> selectedIndexes();

protected:
    void onFilterChanged(const QString& text);

    void onExpandSelected();

    void onCollapseSelected();

    /**
     * @see QWidget::customContextMenuRequested
     */
    void onCustomContextMenuRequested(const QPoint& pos);

    static void expandChildren(QTreeView* view, const QModelIndex& idx, bool expanded);

private:
    QVBoxLayout layout;
    QLineEdit leFilter;
    QTreeView treeView;
    LeafFilterProxyModel sortFilter;

    QAction actionExpandAll;
    QAction actionCollapseAll;

};
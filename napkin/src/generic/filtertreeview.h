#pragma once

#include <QTreeView>
#include <QWidget>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QAction>
#include <QStandardItem>
#include <QItemSelectionModel>
#include <QAbstractItemView>
#include <QMenu>
#include <functional>

#include "leaffilterproxymodel.h"



class FilterTreeView : public QWidget {

public:
    FilterTreeView();

    void setModel(QStandardItemModel* model);

    QStandardItemModel* model() const;
    const QSortFilterProxyModel& filterModel() const { return sortFilter; }

    QTreeView& tree()
    { return treeView; }

    void selectAndReveal(QStandardItem* item);

    QStandardItem* selectedItem();

    QList<QStandardItem*> selectedItems() const;

    QItemSelectionModel* selectionModel() const { return treeView.selectionModel(); }

    QList<QModelIndex> selectedIndexes() const;

    using MenuHookFn = std::function<void(QMenu&)>;
    void setMenuHook(MenuHookFn fn) { mMenuHookFn = fn; }

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
    MenuHookFn mMenuHookFn = nullptr;

    QAction actionExpandAll;
    QAction actionCollapseAll;

};
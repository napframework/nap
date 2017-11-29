#pragma once

#include <QAbstractItemView>
#include <QAction>
#include <QItemSelectionModel>
#include <QLineEdit>
#include <QMenu>
#include <QStandardItem>
#include <QTreeView>
#include <QVBoxLayout>
#include <QWidget>
#include <functional>

#include "leaffilterproxymodel.h"

namespace napkin
{
	/**
	 * A tree view composing a QTreeView and a filter text field that allows filtering of the tree.
	 * This widget keeps an internal filter model, so beware when dealing with QModelIndex instances:
	 *  userModel -> filterModel -> view
	 */
	class FilterTreeView : public QWidget
	{

	public:
		FilterTreeView();

		/**
		 * @param model
		 */
		void setModel(QStandardItemModel* model);

		/**
		 * @return The model set using setModel()
		 */
		QStandardItemModel* model() const;

		/**
		 * @return The sort/filter model that sits between the user model and the view.
		 */
		const QSortFilterProxyModel& filterModel() const
		{
			return sortFilter;
		}

		/**
		 * @return The actual QTreeView used by this widget.
		 */
		QTreeView& tree()
		{
			return treeView;
		}

		/**
		 * Select and item and make sure it's visible on screen by scrolling if needed.
		 * @param item
		 */
		void selectAndReveal(QStandardItem* item);

		/**
		 * @return The first currently selected item.
		 */
		QStandardItem* selectedItem();

		/**
		 * @return The currently selected items in the view.
		 */
		QList<QStandardItem*> selectedItems() const;

		/**
		 * @return The selection model used by the tree view.
		 */
		QItemSelectionModel* selectionModel() const
		{
			return treeView.selectionModel();
		}

		/**
		 * @return The currently selected indexes from the model set by setModel().
		 */
		QList<QModelIndex> selectedIndexes() const;

		/**
		 * When the menu is about to be shown, invoke the provided method to allow a client to insert items into it.
		 * @param fn
		 */
		void setMenuHook(std::function<void(QMenu&)> fn)
		{
			mMenuHookFn = fn;
		}

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
		std::function<void(QMenu&)> mMenuHookFn = nullptr;

		QAction actionExpandAll;
		QAction actionCollapseAll;
	};
};
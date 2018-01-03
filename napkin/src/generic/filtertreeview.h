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
		QStandardItemModel* getModel() const;

		/**
		 * @return The sort/filter model that sits between the user model and the view.
		 */
		const QSortFilterProxyModel& getFilterModel() const
		{
			return mSortFilter;
		}

		/**
		 * @return The actual QTreeView used by this widget.
		 */
		QTreeView& getTreeView()
		{
			return mTreeView;
		}

		/**
		 * Select and item and make sure it's visible on screen by scrolling if needed.
		 * @param item
		 */
		void selectAndReveal(QStandardItem* item);

		/**
		 * @return The first currently selected item.
		 */
		QStandardItem* getSelectedItem();

		/**
		 * @return The currently selected items in the view.
		 */
		QList<QStandardItem*> getSelectedItems() const;

		/**
		 * @return The selection model used by the tree view.
		 */
		QItemSelectionModel* getSelectionModel() const
		{
			return mTreeView.selectionModel();
		}

		/**
		 * @return The currently selected indexes from the model set by setModel().
		 */
		QList<QModelIndex> getSelectedIndexes() const;

		/**
		 * When the menu is about to be shown, invoke the provided method to allow a client to insert items into it.
		 * @param fn
		 */
		void setMenuHook(std::function<void(QMenu&)> fn)
		{
			mMenuHookFn = fn;
		}

	protected:
        /**
         * Invoked when the filter has changed
         * @param text The filter text
         */
		void onFilterChanged(const QString& text);

        /**
         * invoked when the user wants to expand the selection
         */
		void onExpandSelected();

        /**
         * invoked when the user wants to collapse the selection
         */
		void onCollapseSelected();

		/**
		 * @see QWidget::customContextMenuRequested
		 */
		void onCustomContextMenuRequested(const QPoint& pos);

        /**
         * Recursively expand the children of the specified treeview, starting with index
         * @param view The view in which to expand the children
         * @param index The index at which to start expansion
         * @param expanded True for expansion, false for collapse
         */
		static void expandChildren(QTreeView* view, const QModelIndex& index, bool expanded);

	private:
		QVBoxLayout mLayout;
		QLineEdit mLineEditFilter;
		QTreeView mTreeView;
		LeafFilterProxyModel mSortFilter;
		std::function<void(QMenu&)> mMenuHookFn = nullptr;

		QAction mActionExpandAll;
		QAction mActionCollapseAll;
	};
};
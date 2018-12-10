#pragma once

#include <functional>

#include <QAbstractItemView>
#include <QAction>
#include <QItemSelectionModel>
#include <QLineEdit>
#include <QMenu>
#include <QStandardItem>
#include <QTreeView>
#include <QVBoxLayout>
#include <QWidget>

#include "leaffilterproxymodel.h"

namespace napqt
{
	/**
	 * A tree view composing a QTreeView and a filter text field that allows filtering of the tree.
	 * This widget keeps an internal filter model, so beware when dealing with QModelIndex instances:
	 *  userModel -> filterModel -> view
	 */
	class FilterTreeView : public QWidget
	{
	Q_OBJECT
	public:
		FilterTreeView(QTreeView* treeView = nullptr);

		/**
		 * @param model
		 */
		void setModel(QAbstractItemModel* model);

		/**
		 * @return The model set using setModel()
		 */
		QStandardItemModel* getModel() const;

		/**
		 * @return The sort/filter model that sits between the user model and the view.
		 */
		const LeafFilterProxyModel& getFilterModel() const { return mSortFilter; }

		/**
		 * @return The sort/filter model that sits between the user model and the view.
		 */
		LeafFilterProxyModel& getFilterModel() { return mSortFilter; }

		/**
		 * @return The actual QTreeView used by this widget.
		 */
		QTreeView& getTreeView() { return *mTreeView; }

		/**
		 * @return The actual QTreeView used by this widget.
		 */
		const QTreeView& getTreeView() const { return *mTreeView; }

		/**
		 * @return The filter line edit at the top
		 */
		QLineEdit& getLineEdit() { return mLineEditFilter; }

		/**
		 * Select and item and make sure it's visible on screen by scrolling if needed.
		 * @param item
		 */
		void selectAndReveal(QStandardItem* item);

		/**
		 * Setting this will change the following:
		 * As long as there are items visible, the top item will be kept selected.
		 * @param b True if this tree view should behave like a single-selection widget (like a selection dialog).
		 */
		void setIsItemSelector(bool b);

		/**
		 * Force the selection to the top item
		 */
		void setTopItemSelected();

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
		QItemSelectionModel* getSelectionModel() const { return mTreeView->selectionModel(); }

		/**
		 * @return The currently selected indexes from the model set by setModel().
		 */
		QList<QModelIndex> getSelectedIndexes() const;

		/**
		 * When the menu is about to be shown, invoke the provided method to allow a client to insert items into it.
		 * @param fn
		 */
		void setMenuHook(std::function<void(QMenu&)> fn) { mMenuHookFn = fn; }

		/**
		 * @return The widget at the top-right corner, next to the filter line edit.
		 */
		QWidget& getCornerWidget();

	Q_SIGNALS:
		void doubleClicked(const QModelIndex& idx);

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


	private:
		QVBoxLayout mLayout; ///< The main layout
		QHBoxLayout mTopLayout; ///< The layout containing the filter input and the corner widget
		QLineEdit mLineEditFilter; ///< The filter box
		QWidget mCornerWidget;    ///< The [empty] widget at the tip right corner
		QTreeView* mTreeView;
		LeafFilterProxyModel mSortFilter; ///< Sits in between the user model and the tree view
		std::function<void(QMenu&)> mMenuHookFn = nullptr; ///< Gives subclasses the chance to add to the menu
	};

};
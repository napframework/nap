#pragma once

#include <functional>

#include <QAbstractItemModel>
#include <QApplication>
#include <QColor>
#include <QMetaEnum>
#include <QPalette>
#include <QStandardItemModel>

#include <QTreeView>
#include "propertypath.h"

namespace napkin
{
	/**
	 * Mix two colors using a weight.
	 * @param a The color to mix from
	 * @param b The color to mix to
	 * @param p The weight value [0-1]
	 * @return The mixed color.
	 */
	QColor lerpCol(const QColor& a, const QColor& b, qreal p);

	/**
	 * @return A dimmed foreground color based on the current QApplication style
	 */
	const QColor& getSoftForeground();

	/**
	 * @return A dimmed background color based on the current QApplication style
	 */
	const QColor& getSoftBackground();

	/**
	 * Recursively traverse the provided model and invoke the provided visitor for every item.
	 * @param model The mod
	 * @param parent The root index to start at
	 * @param visitor The function to invoke on every item, return true if the traversal should continue
	 * @return false if the the visitor has decided to stop traversal
	 */
	using ModelIndexFilter = std::function<bool(const QModelIndex&)>;
	bool traverse(const QAbstractItemModel& model, ModelIndexFilter visitor,
				  QModelIndex parent = QModelIndex(), int column = 0);

	/**
	 * Recursively traverse the provided model and invoke the provided visitor for every item.
	 * @param model The mod
	 * @param parent The root index to start at
	 * @param visitor The function to invoke on every item, return true if the traversal should continue
	 * @return false if the the visitor has decided to stop traversal
	 */
	using ModelItemFilter = std::function<bool(QStandardItem*)>;
	bool traverse(const QStandardItemModel& model, ModelItemFilter filter,
					   QModelIndex parent = QModelIndex(), int column = 0);

	/**
	 * Recursively expand the children of the specified treeview, starting with index
	 * @param view The view in which to expand the children
	 * @param index The index at which to start expansion
	 * @param expanded True for expansion, false for collapse
	 */
	void expandChildren(QTreeView* view, const QModelIndex& index, bool expanded);

	/**
	 * Traverse a model and using the given filter function, return the first index.
	 * @param model The model to search
	 * @param condition The filter function that when it returns true, the traversal will stop and return the current
	 * index.
	 * @return The model index representing the item to be found.
	 */
	QModelIndex findIndexInModel(const QAbstractItemModel& model, ModelIndexFilter condition, int column = 0);

	/**
	 * Traverse a model and using the given filter function, return the first index.
	 * @param model The model to search
	 * @param condition The filter function that when it returns true, the traversal will stop and return the current
	 * index.
	 * @return The model index representing the item to be found.
	 */
	QStandardItem* findItemInModel(const QStandardItemModel& model, ModelItemFilter condition, int column = 0);

	/**
	 * Check wherether the provided filename exists somewhere in the provided directory.
	 * @param dir The parent directory to check for
	 * @param filename The filename to 'look' for.
	 * @return true if the file is a child of dir
	 */
	bool directoryContains(const QString& dir, const QString& filename);


	/**
	 * Show a dialog box containing the given properties and a custom message.
	 * @param parent The parent widget to attach the dialog to
	 * @param props The properties to display in the dialog
	 * @param message The custom to display alongside the list of properties
	 */
	void showPropertyListDialog(QWidget* parent, QList<PropertyPath> props, const QString& title, QString message);

	/**
	 * @tparam QEnum The enum type to use
	 * @param value The enum value to get the string from.
	 * @return A string representing the enum value
	 */
	template <typename QEnum>
	static QString QEnumToString(const QEnum value)
	{
		return QMetaEnum::fromType<QEnum>().valueToKey(value);
	}

}


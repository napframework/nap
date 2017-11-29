#pragma once

#include <QAbstractItemModel>
#include <QApplication>
#include <QColor>
#include <QMetaEnum>
#include <QPalette>
#include <QtGui/QStandardItemModel>
#include <functional>
#include <panels/resourcepanel.h>
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
	const QColor& softForeground();

	/**
	 * @return A dimmed background color based on the current QApplication style
	 */
	const QColor& softBackground();

	/**
	 * Recursively traverse the provided model and invoke the provided visitor for every item.
	 * @param model The mod
	 * @param parent The root index to start at
	 * @param visitor The function to invoke on every item, return true if the traversal should continue
	 * @return false if the the visitor has decided to stop traversal
	 */
	bool traverse(const QAbstractItemModel& model, std::function<bool(const QModelIndex&)> visitor,
				  QModelIndex parent = QModelIndex());

	/**
	 * Traverse a model and using the given filter function, return the first index.
	 * @param model The model to search
	 * @param condition The filter function that when it returns true, the traversal will stop and return the current
	 * index.
	 * @return The model index representing the item to be found.
	 */
	QModelIndex findItemInModel(const QAbstractItemModel& model, std::function<bool(const QModelIndex& idx)> condition);


	/**
	 * @return All nap component types in the rtti system
	 */
	std::vector<rttr::type> getComponentTypes();

	/**
	 * @return All nap resource types in the rtti system
	 */
	std::vector<rttr::type> getResourceTypes();

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
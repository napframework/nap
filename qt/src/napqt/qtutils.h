/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <functional>
#include <QAbstractItemModel>
#include <QApplication>
#include <QColor>
#include <QMetaEnum>
#include <QPalette>
#include <QStandardItemModel>
#include <QTreeView>
#include <QGraphicsItem>

namespace nap
{
	namespace qt
	{
		template<typename T, typename U>
		T lerp(const T& a, const T& b, const U& t)
		{
			return a + ((b - a) * t);
		}

		/**
		 * Mix two colors using a weight.
		 * @param a The color to mix from
		 * @param b The color to mix to
		 * @param p The weight value [0-1]
		 * @return The mixed color.
		 */
		QColor lerpCol(const QColor& a, const QColor& b, qreal p);

		QPointF lerpPoint(const QPointF& a, const QPointF& b, qreal p);

		qreal roundToInterval(qreal v, qreal step);
		qreal floorToInterval(qreal v, qreal step);
		qreal ceilToInterval(qreal v, qreal step);

		template<typename T>
		T clamp(const T& value, const T& min, const T& max)
		{
			bool less = min < max;
			T mn = less ? min : max;
			T mx = less ? max : min;
			if (value < mn)
				return mn;
			if (value > mx)
				return mx;
			return value;
		}

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
		 * @tparam QEnum The enum type to use
		 * @param value The enum value to get the string from.
		 * @return A string representing the enum value
		 */
		template<typename QEnum>
		static QString QEnumToString(const QEnum value)
		{
			return QMetaEnum::fromType<QEnum>().valueToKey(value);
		}

		/**
		 * Reveal the given file in Explorer, Finder, Files or whatever the OS' file browser is.
		 * @param filename The file to show (not open)
		 */
		bool revealInFileBrowser(const QString& filename);

		/**
		 * Reveal the given file in an external editor associated with that file extension by the OS.
		 * @param filename The file to open
		 */
		bool openInExternalEditor(const QString& filename);

		/**
		 * @return The name of the OS' default file browser.
		 */
		QString fileBrowserName();

		/**
		 * Get the translation component from a QTransform
		 */
		QPointF getTranslation(const QTransform& xf);

		/**
		 * Set the translation component from a QTransform
		 */
		void setTranslation(QTransform& xf, qreal x, qreal y);

		/**
		 * Set the translation component from a QTransform
		 */
		void setTranslation(QTransform& xf, const QPointF& p);

		/**
		 * Get the scale component from a QTransform
		 */
		QSizeF getScale(const QTransform& xf);

		/**
		 * Set the scale component from a QTransform
		 */
		void setScale(QTransform& xf, qreal x, qreal y);

		/**
		 * Move a QGraphicsItem to the front
		 */
		void moveItemToFront(QGraphicsItem& item);

		/**
		 * Reverse sort the list
		 */
		QList<int> reverseSort(const QList<int>& ints);

		template<typename K, typename V>
		void removeFromMapByValue(QMap<K, V> map, V element)
		{
			for (auto it = map.begin(); it != map.end();)
				if (it.value() == element)
					it = map.erase(it);
				else
					++it;
		}

		template<typename K, typename V>
		const K& keyFromValue(const QMap<K, V>& map, const V& value, const K& defaultKey)
		{
			for (auto it = map.cbegin(); it != map.cend(); it++)
				if (it.value() == value)
					return it.key();
			return defaultKey;
		}

	} // namespace qt

} // namespace nap
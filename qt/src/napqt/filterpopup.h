/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <QDialog>
#include <QStringListModel>
#include <memory>

#include "filtertreeview.h"

namespace nap
{
	namespace qt
	{
		/**
		 * Simple text model with tooltip.
		 * Replacement for QStringListModel, which doesn't support tooltips.
		 */
		class StringModel final : public QStandardItemModel
		{
			Q_OBJECT
		public:
			/**
			 * Single text model item entry with optional tooltip
			 */
			struct Entry final
			{
				// Text without tooltip
				Entry(const QString& text) : mText(text) {}
				Entry(QString&& text) : mText(std::move(text)) {}

				// Text with tooltip
				Entry(const QString& text, const QString& tooltip) : mText(text), mTooltip(tooltip) {}
				Entry(QString&& text, QString&& toolTip) : mText(std::move(text)), mTooltip(std::move(toolTip)) {}

				QString mText = "";		///< The text to display
				QString mTooltip;		///< The tooltip to display
			};
			using Entries = QList<Entry>;

			/**
			 * Single text model item, wraps an entry
			 */
			class Item final : public QStandardItem
			{
			public:
				Item(Entry&& entry) : QStandardItem(entry.mText), mEntry(std::move(entry)) { }
				Entry mEntry;
			};

			/**
			 * Returns text or tooltip data
			 */
			virtual QVariant data(const QModelIndex& index, int role) const override;

			/**
			 * Move Constructor
			 * @param items movable item data
			 */
			StringModel(Entries&& items);
		};


		/**
		 * A popup of selectable string items that can be filtered
		 */
		class FilterPopup : public QMenu
		{
		private:
			// Construct popup with items (from entries) to select from
			FilterPopup(StringModel::Entries&& entries, QWidget* parent = nullptr);

		public:
			/**
			 * Show a popup and select a text item from the given entries
			 * @param parent parent widget
			 * @param entries text items to choose from
			 */
			static QString show(QWidget* parent, StringModel::Entries&& entries);

			/**
			 * Show a popup at the given position and select a text item from the given entries
			 * @param parent parent widget
			 * @param entries text items to choose from
			 * @param pos popup screen position
			 */
			static QString show(QWidget* parent, StringModel::Entries&& entries, QPoint pos);

		protected:
			void keyPressEvent(QKeyEvent* event) override;
			void showEvent(QShowEvent* event) override;

		private:
			void moveSelection(int d);
			void accept();
			void computeSize();

			int mBottomMargin = 10;
			QVBoxLayout mLayout;
			std::unique_ptr<StringModel> mModel = nullptr;
			FilterTreeView mFilterTree;
			QString mChoice;
		};

	} // namespace qt

} // namespace nap

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
		 * Replacement for QStringListModel, which doesn't support tooltips
		 */
		class StringModel final : public QStandardItemModel
		{
			Q_OBJECT
		public:
			/**
			 * Single text model item entry
			 */
			struct StringEntry final
			{
				StringEntry() = default;
				StringEntry(const QString& text) : mText(text) {}
				StringEntry(const QString& text, const QString& tooltip) : mText(text), mTooltip(tooltip) {}

				QString mText = "";		///< The text to display
				QString mTooltip;		///< The tooltip to display
			};
			using Entries = QList<StringEntry>;

			/**
			 * Single text model item, wraps an entry
			 */
			class StringItem final : public QStandardItem
			{
			public:
				StringItem(StringEntry&& entry) : QStandardItem(entry.mText), mEntry(std::move(entry)) { }
				StringItem(const StringEntry& entry) : QStandardItem(entry.mText), mEntry(entry) { }
				StringEntry mEntry;
			};

			/**
			 * Returns text or tooltip data
			 */
			virtual QVariant data(const QModelIndex& index, int role) const override;

			/**
			 * Constructor
			 * @param items item data
			 */
			StringModel(const Entries& items);

			/**
			 * Move Constructor
			 * @param items movable item data
			 */
			StringModel(Entries&& items);
		};


		/**
		 * A popup list of String items that can be filtered
		 */
		class FilterPopup : public QMenu
		{
		private:
			FilterPopup(QWidget* parent = nullptr);

		public:
			static QString show(QWidget* parent, const QStringList& items);
			static QString show(QWidget* parent, const QStringList& items, QPoint pos);

		protected:
			void keyPressEvent(QKeyEvent* event) override;
			void showEvent(QShowEvent* event) override;

		private:
			void setItems(const QStringList& items);
			void moveSelection(int d);
			void accept();
			void computeSize();

			int mBottomMargin = 10;
			QVBoxLayout mLayout;
			std::unique_ptr<QStringListModel> mModel = nullptr;
			FilterTreeView mFilterTree;
			QString mChoice;
		};

	} // namespace qt

} // namespace nap

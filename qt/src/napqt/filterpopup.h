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
			struct StringItem final : public QStandardItem
			{
				StringItem() = default;
				StringItem(const QString& text) : mText(text) {}
				StringItem(const QString& text, const QString& tooltip) : mText(text), mTooltip(tooltip) {}

				QString mText = "";		///< The text to display
				QString mTooltip;		///< The tooltip to display
			};
			using Items = QList<StringItem>;

			/**
			 * Returns text or tooltip data
			 */
			virtual QVariant data(const QModelIndex& index, int role) const override;

			/**
			 * Constructor
			 * @param items item data
			 */
			StringModel(const Items& items) : mItems(items) { }

		private:
			StringModel::Items mItems;
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

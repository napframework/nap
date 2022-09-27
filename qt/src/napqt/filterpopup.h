/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <QDialog>
#include <QStringListModel>

#include "filtertreeview.h"

namespace nap
{
	namespace qt
	{
		/**
		 * A popup list of strings that can be filtered
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

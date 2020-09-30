/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <QWidget>
#include <QSplitter>
#include <QItemDelegate>
#include <napqt/filtertreeview.h>

#include <napqt/qtutils.h>
#include "curveview.h"
#include "curvetreemodel.h"

namespace nap
{

	namespace qt
	{
		/**
		 * This delegate paints a single icon/button in a curve outline row
		 */
		class CurveTreeIconDelegate
		{
		public:
			void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
			{
				painter->fillRect(option.rect, Qt::red);
			}

		};

		/**
		 * This delegate paints multiple icons/buttons in a curve outline row
		 */
		class CurveTreeDelegate : public QItemDelegate
		{
		public:
			void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
			QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;

			void addDelegate(std::shared_ptr<CurveTreeIconDelegate> delegate);
		private:
			QList<std::shared_ptr<CurveTreeIconDelegate>> mIconDelegates;
			int mIconSpacing = 4;
			QMargins mIconMargins = {8, 2, 16, 2};
		};

		/**
		 * A widget that allows editing multiple curves
		 */
		class CurveWidget : public QWidget
		{
		Q_OBJECT
		public:
			explicit CurveWidget(QWidget* parent = nullptr);

			/**
			 * Replace the curves in this widget with the ones provided by model.
			 */
			void setModel(AbstractCurveModel* model);


		private:
			void onTreeDoubleClicked(const QModelIndex& idx);

			QVBoxLayout mLayout;
			QSplitter mSplitter;
			CurveView mCurveView;
			FilterTreeView mTreeView;
			CurveTreeModel mTreeModel;
			CurveTreeDelegate mDelegate;
		};

	} // namespace qt

} // namespace nap

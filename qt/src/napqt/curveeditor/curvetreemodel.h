/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <QWidget>
#include <QSplitter>
#include <napqt/filtertreeview.h>
#include <napqt/qtutils.h>
#include "curveview.h"

namespace nap
{
	namespace qt
	{
		enum CurveTreeRole
		{
			ColorRole = Qt::UserRole,

		};

		class CurveTreeItem : public QObject, public QStandardItem
		{
		Q_OBJECT
		public:
			CurveTreeItem(AbstractCurve& curve);
		private:
			void onCurveChanged(AbstractCurve* curve);
			AbstractCurve& mCurve;
		};

		class CurveTreeModel : public QStandardItemModel
		{
		Q_OBJECT
		public:
			CurveTreeModel(QObject* parent = nullptr) : QStandardItemModel(parent) {}
			void setCurveModel(AbstractCurveModel* model);
			AbstractCurve* curveFromIndex(const QModelIndex& idx);

		private:
			void onCurvesInserted(const QList<int>& indexes);
			void onCurvesRemoved(const QList<int>& indexes);

			AbstractCurveModel* mModel = nullptr;
		};
	} // namespace qt
} // namespace nap

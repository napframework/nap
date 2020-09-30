/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "curvetreemodel.h"

using namespace nap::qt;

CurveTreeItem::CurveTreeItem(AbstractCurve& curve) : QObject(), QStandardItem(), mCurve(curve)
{
	curve.connect(&curve, &AbstractCurve::changed, this, &CurveTreeItem::onCurveChanged);
	onCurveChanged(&mCurve);
}

void CurveTreeItem::onCurveChanged(AbstractCurve* curve)
{
	setText(mCurve.name());
}

void CurveTreeModel::setCurveModel(AbstractCurveModel* model)
{
	if (mModel)
	{
		disconnect(mModel, &AbstractCurveModel::curvesInserted, this, &CurveTreeModel::onCurvesInserted);
		disconnect(mModel, &AbstractCurveModel::curvesRemoved, this, &CurveTreeModel::onCurvesRemoved);
	}
	mModel = model;

	if (mModel)
	{
		connect(mModel, &AbstractCurveModel::curvesInserted, this, &CurveTreeModel::onCurvesInserted);
		connect(mModel, &AbstractCurveModel::curvesRemoved, this, &CurveTreeModel::onCurvesRemoved);
	}

	QList < int > indexes;
	for (int i = 0, len = model->curveCount(); i < len; i++)
		indexes << i;
	onCurvesInserted(indexes);
}

void CurveTreeModel::onCurvesInserted(const QList<int> indexes)
{
	auto sortedIndexes = indexes;
	qSort(sortedIndexes);

	for (int index : sortedIndexes)
	{
		auto curve = mModel->curve(index);
		assert(curve != nullptr);
		insertRow(index, new CurveTreeItem(*curve));
	}
}

void CurveTreeModel::onCurvesRemoved(const QList<int> indexes)
{
	for (int i : reverseSort(indexes))
		removeRow(i);
}

AbstractCurve* CurveTreeModel::curveFromIndex(const QModelIndex& idx)
{
	assert(idx.model() == this);
	return mModel->curve(idx.row());
}


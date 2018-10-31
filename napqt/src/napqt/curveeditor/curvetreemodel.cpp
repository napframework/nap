#include "curvetreemodel.h"



using namespace napqt;

void CurveTreeModel::setCurveModel(AbstractCurveModel* model)
{
	if (mModel)
	{
		disconnect(mModel, &AbstractCurveModel::curvesAdded, this, &CurveTreeModel::onCurvesAdded);
		disconnect(mModel, &AbstractCurveModel::curvesChanged, this, &CurveTreeModel::onCurvesChanged);
		disconnect(mModel, &AbstractCurveModel::curvesRemoved, this, &CurveTreeModel::onCurvesRemoved);
	}
	mModel = model;

	if (mModel)
	{
		connect(mModel, &AbstractCurveModel::curvesAdded, this, &CurveTreeModel::onCurvesAdded);
		connect(mModel, &AbstractCurveModel::curvesChanged, this, &CurveTreeModel::onCurvesChanged);
		connect(mModel, &AbstractCurveModel::curvesRemoved, this, &CurveTreeModel::onCurvesRemoved);
	}

	auto topLeft = index(0, 0);
	auto botRight = index(rowCount() - 1, 0);
	dataChanged(topLeft, botRight);
}

QModelIndex CurveTreeModel::index(int row, int column, const QModelIndex& parent) const
{
	if (!mModel)
		return {};

	return createIndex(row, column);
}

QModelIndex CurveTreeModel::parent(const QModelIndex& child) const
{
	if (!mModel)
		return {};
	return {};
}

int CurveTreeModel::rowCount(const QModelIndex& parent) const
{
	if (!mModel || parent.isValid())
		return 0;

	return mModel->curveCount();
}

int CurveTreeModel::columnCount(const QModelIndex& parent) const
{
	if (!mModel)
		return 0;

	return 1;
}

QVariant CurveTreeModel::data(const QModelIndex& index, int role) const
{
	if (!mModel)
		return QVariant();

	switch (role)
	{
		case Qt::DisplayRole:
			return QString("Curve %1").arg(index.row());
		default:
			return QVariant();
	}
}

void CurveTreeModel::onCurvesAdded(const QList<int> indexes)
{
	auto idc = indexes;
	qSort(idc);

	auto parent = QModelIndex();
	for (int i : idc)
	{
		beginInsertRows(parent, i, i);
		insertRow(i);
		endInsertRows();
	}
}

void CurveTreeModel::onCurvesChanged(const QList<int> indexes)
{
	for (int i : indexes)
	{
		auto topLeft = index(i, 0);
		auto botRight = index(i, 0);
		dataChanged(topLeft, botRight);
	}
}

void CurveTreeModel::onCurvesRemoved(const QList<int> indexes)
{
	auto parent = QModelIndex();
	for (int i : reverseSort(indexes))
	{
		beginRemoveRows(parent, i, i);
		removeRows(i, 1);
		endRemoveRows();
	}
}
AbstractCurve* CurveTreeModel::curveFromIndex(const QModelIndex& idx)
{
	assert(idx.model() == this);
	return mModel->curve(idx.row());
}

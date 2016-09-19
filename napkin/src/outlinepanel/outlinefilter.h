
#pragma once

#include "outlinemodel.h"
#include <QSortFilterProxyModel>


class OutlineFilterProxy : public QSortFilterProxyModel
{
	Q_OBJECT
public:
	OutlineFilterProxy(QObject* parent = nullptr) : QSortFilterProxyModel(parent) {}

	void setShowComponents(bool b) {
		mShowComponents = b;
		invalidate();
	}

	void setShowAttributes(bool b) {
		mShowAttributes = b;
		invalidate();
	}

protected:
	ObjectItem* toObjectItem(int sourceRow, const QModelIndex& sourceParent) const
	{
		OutlineModel* outlineModel = (OutlineModel*)sourceModel();
		if (!outlineModel) return nullptr;

		QModelIndex idx = index(sourceRow, 0, sourceParent);
		idx = mapToSource(idx);
		QStandardItem* item = outlineModel->itemFromIndex(idx);
		return (ObjectItem*)item;
	}


	bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override
	{
		ObjectItem* item = toObjectItem(source_row, source_parent);
		if (!item) {
			return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
		}

        RTTI::TypeInfo objectType = item->object().getTypeInfo();

		if (!mShowAttributes && objectType.isKindOf<nap::AttributeBase>()) return false;

		if (!mShowComponents && objectType.isKindOf<nap::Component>()) return false;

		return true;
	}

private:
	bool mShowComponents = false;
	bool mShowAttributes = false;
};
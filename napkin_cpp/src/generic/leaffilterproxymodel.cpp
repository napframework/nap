#include "leaffilterproxymodel.h"
#include <nap/logger.h>
#include <QStandardItemModel>

bool LeafFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const {
//    if (!source_parent.isValid())
//        return true;

	if (QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent)) {
		auto idx = source_parent.child(source_row, 0);
		auto item = ((QStandardItemModel*)sourceModel())->itemFromIndex(idx);
		if (item) {
			nap::Logger::info("Accepted: %s", item->text().toStdString().c_str());
		}
		return true;
	}
	return false;
    return hasAcceptedChildren(source_row, source_parent);
}

bool LeafFilterProxyModel::filterAcceptsAnyParent(QModelIndex parent) const {
    while (parent.isValid()) {
        if (QSortFilterProxyModel::filterAcceptsRow(parent.row(), parent.parent()))
            return true;
        parent = parent.parent();
    }
    return false;
}

bool LeafFilterProxyModel::hasAcceptedChildren(int row, QModelIndex parent) const {
    auto model = sourceModel();
    auto sourceIndex = model->index(row, 0, parent);

    for (int r = 0, len = model->rowCount(sourceIndex); r < len; r++) {
        if (filterAcceptsRow(row, sourceIndex))
            return true;
    }
    return false;
}


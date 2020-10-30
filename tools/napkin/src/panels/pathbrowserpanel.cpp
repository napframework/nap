/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "pathbrowserpanel.h"
#include <QList>
#include <QtDebug>

using namespace napkin;

QModelIndex PathBrowserModel::index(int row, int column, const QModelIndex& parent) const
{
	if (!parent.isValid())
		return createIndex(row, column, mRootPaths[row].get());

	auto parentPath = static_cast<PropertyPath*>(parent.internalPointer());
	return createIndex(row, column, mCache[parentPath][row].get());
}

QModelIndex PathBrowserModel::parent(const QModelIndex& child) const
{
	return QModelIndex();
}

int PathBrowserModel::rowCount(const QModelIndex& parent) const
{
	if (!parent.isValid())
		return mRootPaths.size();

	auto parentPath = static_cast<PropertyPath*>(parent.internalPointer());
	if (!mCache.contains(parentPath))
	{
		QList<std::shared_ptr<PropertyPath>> children;
		parentPath->iterateChildren([&children](const PropertyPath& path)
		{
			children.append(std::make_shared<PropertyPath>(path));
			return true;
		}, IterFlag::FollowPointers | IterFlag::FollowEmbeddedPointers);
		mCache.insert(parentPath, children);
	}
	return mCache[parentPath].size();
}

int PathBrowserModel::columnCount(const QModelIndex& parent) const
{
	return 2;
}

QVariant PathBrowserModel::data(const QModelIndex& index, int role) const
{
	if (!index.internalPointer())
		return {};
	auto propPath = static_cast<PropertyPath*>(index.internalPointer());

	if (index.column() == 0)
	{
		if (role == Qt::DisplayRole)
			return QString::fromStdString(propPath->getName());
	}
	else if (index.column() == 1)
	{
		if (role == Qt::DisplayRole)
			return QString::fromStdString(std::string(propPath->getType().get_name().data()));
	}
	return QVariant();
}

void PathBrowserModel::clear()
{
	mCache.clear();
	mRootPaths = {};
}

void PathBrowserModel::addPath(const PropertyPath& path)
{
	beginInsertRows({}, 0, 0);
	mRootPaths.append(std::make_shared<PropertyPath>(path));
	endInsertRows();
}


PathBrowserPanel::PathBrowserPanel()
{
	setLayout(&mLayout);
	mLayout.setContentsMargins(0, 0, 0, 0);
	mLayout.addWidget(&mTreeView);
	mTreeView.setModel(&mModel);

	mTreeView.getTreeView().setSortingEnabled(true);

	connect(&AppContext::get(), &AppContext::documentChanged, [this](Document* doc)
	{
		mModel.clear();
		if (!doc)
			return;

		for (auto& obj : doc->getObjects())
		{
			if (obj->get_type().is_derived_from<nap::Component>())
				continue;
			if (obj->get_type().is_derived_from<nap::InstancePropertyValue>())
				continue;
//			mModel.addPath({*obj});
		}
	});

	connect(mTreeView.getTreeView().selectionModel(), &QItemSelectionModel::selectionChanged,
			[this](const QItemSelection& selected,
				   const QItemSelection& deselected)
			{
				auto indexes = mTreeView.getTreeView().selectionModel()->selectedIndexes();
				for (auto idx : indexes)
				{
					if (idx.column() > 0)
						continue;
					auto srcIndex = mTreeView.getFilterModel().mapToSource(idx);
					auto item = mTreeView.getModel()->itemFromIndex(srcIndex);
//					auto pathItem = dynamic_cast<PathItem*>(item);
//					qInfo() << pathItem->text();
				}
			});
}
nap::qt::FilterTreeView& PathBrowserPanel::treeView() { return mTreeView; }

#include "pathbrowserpanel.h"

#include <QtDebug>

using namespace napkin;

PathItem::PathItem(const PropertyPath& path) : mPath(path)
{
	if (!path.hasProperty())
		setIcon(AppContext::get().getResourceFactory().getIcon(path.getObject()));

	for (auto p : path.getChildren(IterFlag::FollowPointers | IterFlag::FollowEmbeddedPointers))
	{
		appendRow({
						  new PathItem(p),
						  new PathTypeItem(p)
				  });
	}
}
QVariant PathItem::data(int role) const
{
	if (role == Qt::DisplayRole)
	{
		auto pathstr = QString::fromStdString(mPath.toString());
		auto pathsplit = pathstr.split('/');
		return pathsplit.last();
	}
//	else if (role == Qt::ForegroundRole)
//	{
//		if (mPath.hasProperty())
//		{
//			return QVariant::fromValue<QColor>(Qt::blue);
//		}
//	}
	return QStandardItem::data(role);
}

PathTypeItem::PathTypeItem(const PropertyPath& path)
		: PathItem(path)
{

}

QVariant PathTypeItem::data(int role) const
{
	if (role == Qt::DisplayRole)
		return QString::fromStdString(std::string(path().getType().get_name()));
	if (role == Qt::DecorationRole)
		return {};
	return PathItem::data(role);
}

const PropertyPath& PathItem::parentPath()
{
	auto parentPathItem = dynamic_cast<PathItem*>(parent());
	if (parentPathItem)
		return parentPathItem->path();

	return PropertyPath::invalid();
}

const PropertyPath& PathItem::path() const
{
	return mPath;
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
			PropertyPath p(*obj);
			mModel.appendRow({
									 new PathItem(p),
									 new PathTypeItem(p)
							 });
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
					auto pathItem = dynamic_cast<PathItem*>(item);
					qInfo() << pathItem->text();
				}
			});
}
nap::qt::FilterTreeView& PathBrowserPanel::treeView() { return mTreeView; }

#include "pathbrowserpanel.h"

using namespace napkin;

PathItem::PathItem(const PropertyPath& path) : mPath(path)
{
	if (!path.hasProperty())
		setIcon(AppContext::get().getResourceFactory().getIcon(path.getObject()));

	for (auto p : path.getChildren())
	{
//		if (mPath.getType().is_derived_from<nap::Entity>())
			appendRow(new PathItem(p));
	}
}
QVariant PathItem::data(int role) const
{
	if (role == Qt::DisplayRole)
		return QString::fromStdString(mPath.toString());
	return QStandardItem::data(role);
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
			mModel.appendRow(new PathItem(PropertyPath(*obj)));
		}
	});
}
nap::qt::FilterTreeView& PathBrowserPanel::treeView() { return mTreeView; }

#include "hierarchypanel.h"
#include "naputils.h"
#include "napkinfiltertree.h"

napkin::TypeModel::TypeModel()
{
	refresh();
}

void napkin::TypeModel::refresh()
{
	// Clear existing items first
	while (rowCount())
		removeRow(0);

	nap::rtti::TypeInfo rootType = RTTI_OF(nap::rtti::Object);
	auto rootItem = new RTTITypeItem(rootType);
	appendRow(rootItem);
	for (const auto& derived : getDerivedTypes(rootType))
	{
		rootItem->appendRow(new RTTITypeItem(derived));
	}
}

napkin::HierarchyPanel::HierarchyPanel()
{
	setLayout(&mLayout);
	layout()->setContentsMargins(0, 0, 0, 0);
	mLayout.addWidget(&mTreeView);
	mTreeView.setModel(&mModel);
}

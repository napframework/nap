#include "hierarchypanel.h"
#include "generic/naputils.h"

napkin::TypeModel::TypeModel()
{
	refresh();
}

void napkin::TypeModel::refresh()
{
	// Clear existing items first
	while (rowCount() > 0)
		removeRow(0);

	nap::rtti::TypeInfo rootType = RTTI_OF(nap::rtti::RTTIObject);
	for (const nap::rtti::TypeInfo& derived : rootType.get_derived_classes())
	{
		appendRow(new RTTITypeItem(derived));
	}
}

napkin::HierarchyPanel::HierarchyPanel()
{
	setLayout(&mLayout);
	layout()->setContentsMargins(0, 0, 0, 0);
	mLayout.addWidget(&mTreeView);
	mTreeView.setModel(&mModel);
}

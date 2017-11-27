#include "hierarchypanel.h"
#include "generic/napgeneric.h"


TypeModel::TypeModel() {
    refresh();
}

void TypeModel::refresh() {
    // Clear existing items first
    while (rowCount() > 0)
        removeRow(0);

    nap::rtti::TypeInfo rootType = RTTI_OF(nap::rtti::RTTIObject);
    for (const nap::rtti::TypeInfo& derived : rootType.get_derived_classes()) {
        appendRow(new RTTITypeItem(derived));
    }
}

HierarchyPanel::HierarchyPanel() {
    setLayout(&mLayout);
    layout()->setContentsMargins(0,0,0,0);
    mLayout.addWidget(&mTreeView);
    mTreeView.setModel(&mModel);
}

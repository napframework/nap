#include "hierarchypanel.h"
#include "napgeneric.h"


TypeModel::TypeModel() {
    refresh();
}

void TypeModel::refresh() {
    // Clear existing items first
    while (rowCount() > 0)
        removeRow(0);

    nap::rtti::TypeInfo rootType = RTTI_OF(nap::rtti::RTTIObject);
    for (const nap::rtti::TypeInfo& derived : rootType.get_derived_classes()) {
        appendRow(new TypeItem(derived));
    }
}

HierarchyPanel::HierarchyPanel() {
    setLayout(&layout);
    layout.addWidget(&treeView);
    treeView.setModel(&model);
}

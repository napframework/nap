#include "hierarchypanel.h"



TypeItem::TypeItem(const nap::rtti::TypeInfo& type) : type(type) {
    setText(type.get_name().data());
    refresh();
}

void TypeItem::refresh() {
    for (const nap::rtti::TypeInfo& derived : type.get_derived_classes()) {
        appendRow(new TypeItem(derived));
    }
}

TypeModel::TypeModel() {

    refresh();
}

void TypeModel::refresh() {
    // Clear existing items first
    while(rowCount() > 0)
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

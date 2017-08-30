#include "napgeneric.h"


TypeItem::TypeItem(const nap::rtti::TypeInfo& type) : type(type) {
    setText(type.get_name().data());
    refresh();
}

void TypeItem::refresh() {
    for (const nap::rtti::TypeInfo& derived : type.get_derived_classes()) {
        appendRow(new TypeItem(derived));
    }
}

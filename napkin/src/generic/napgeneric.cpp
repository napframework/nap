#include <generic/utility.h>
#include "napgeneric.h"


RTTITypeItem::RTTITypeItem(const nap::rtti::TypeInfo& type) : type(type) {
    setText(type.get_name().data());
    setEditable(false);
//    setForeground(softForeground());
//    setBackground(softBackground());
    refresh();
}

void RTTITypeItem::refresh() {
    for (const nap::rtti::TypeInfo& derived : type.get_derived_classes()) {
        appendRow(new RTTITypeItem(derived));
    }
}
